#include <chrono>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <endian.h>
#include <iostream>
#include <math.h>
#include <utility>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <random>
#include <unordered_map>
#include "colors.h"
#include "coord.h"
#include "grid.h"
#include "node.h"
#include <array>
#include "decisions.h"
#include "mcts.h"
#include "middleware.h"

// ---------- Thread-safety globals ----------
static std::mutex expand_mutex;      // protects node expansion (SetChildren)
static std::mutex backprop_mutex;    // protects BackProp / updating visits/rewards if Node isn't internally locked
static std::mutex amaf_mutex;        // protects UpdateAMAF (if Node lacks internal locking)
static std::mutex selection_mutex;   // protects selection when reading children structure
static std::atomic<bool> keepGoing{true};
static std::atomic<COLOUR> globalAgentAtomic; // thread-safe storage of agent
// thread_local RNG for each thread
thread_local std::mt19937 thread_rng((std::random_device())());

// helper to pick random element index from vector size
inline int randIndex(int n) {
    std::uniform_int_distribution<int> dist(0, n - 1);
    return dist(thread_rng);
}

// ---------- Core functions ----------

Node* Traverse(Node* root, HexGrid* grid) {
    // Selection: while node has children, pick the best according to RAVE/UCB (your code used RAVE)
    Node* cur = root;
    while (true) {
        std::vector<Node*> children;
        {
            // copy children pointer list under protection to avoid races on vector modification
            std::lock_guard<std::mutex> lock(selection_mutex);
            children = cur->GetChildren();
        }

        if (children.size() == 0) {
            // unexpanded node -> return for expansion
            return cur;
        }

        // choose best child using RAVE (your original logic)
        Node* best_node = children[randIndex((int)children.size())];
        float best_rave = best_node->GetRaveScore();
        for (Node* n : children) {
            float n_rave = n->GetRaveScore();
            // prefer higher RAVE if agent is BLUE (your original comparison logic)
            if ((n_rave > best_rave) == (cur->GetAgent() == BLUE)) {
                best_node = n;
                best_rave = n_rave;
            }
        }

        // advance grid by applying best_node move
        grid->Update(best_node->GetMove(), cur->GetAgent());
        cur = best_node;
    }
}

// BackProp now locks when updating nodes (assumes Node methods are not internally thread-safe)
void BackProp(Node* nodePtr, float reward) {
    // iterative to avoid deep recursion across threads (safe)
    Node* cur = nodePtr;
    float r = reward;
    while (cur) {
        {
            std::lock_guard<std::mutex> lock(backprop_mutex);
            cur->IncrementVisits();
            cur->UpdateReward(r);
        }
        if (cur->IsRoot()) break;
        cur = cur->GetParent();
        r *= mctsdamping;
    }
}

// Stop helper: choose final child when time's up / strong stop condition
Node* MCTSStop(Node* tree, COLOUR agent, std::chrono::steady_clock::time_point end) {
    Node* best_node = nullptr;
    while (std::chrono::steady_clock::now() < end && keepGoing.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // small sleep to reduce busy spin
        std::vector<Node*> children;
        {
            std::lock_guard<std::mutex> lock(selection_mutex);
            children = tree->GetChildren();
        }
        if (children.size() == 0) continue;

        // Random initial pick
        best_node = children[randIndex((int)children.size())];
        float best_ucb = best_node->UCB(); // call as in your code

        int maxvisits = 0;
        for (Node* n : children) {
            float nucb = n->UCB();
            if (((nucb > best_ucb) == (agent == BLUE))) {
                best_ucb = nucb;
                best_node = n;
            }
            // track max visits for stop condition
            int nv = n->GetVisits();
            if (nv > maxvisits) maxvisits = nv;
        }
        // if the chosen node already has visits equal to max visits (i.e., top contender is stable), stop early
        if (best_node && best_node->GetVisits() >= maxvisits) {
            keepGoing.store(false);
            return best_node;
        }
    }
    // time's up — signal termination and return last best found (may be nullptr)
    keepGoing.store(false);
    return best_node;
}

// Inner loop executed by worker threads (does selection, expansion, simulation, backprop)
void MCTSInnerLoop(Node* root, std::chrono::steady_clock::time_point end, HexGrid grid) {
    while (keepGoing.load() && std::chrono::steady_clock::now() < end) {
        HexGrid g(grid); // copy grid
        // Selection
        Node* nodePtr = Traverse(root, &g);

        // Possibly expand this node: ensure only one thread expands it
        bool do_expand = false;
        {
            // double-check under expand lock
            std::lock_guard<std::mutex> lock(expand_mutex);
            auto children = nodePtr->GetChildren();
            if (children.size() == 0) {
                // expand
                do_expand = true;
            }
        }

        std::tuple<float, std::vector<Coord>> sim_result;
        if (do_expand) {
            // Expand (this calls Simulate) - we protect SetChildren inside Expand by expand_mutex already
            // Expand returns (reward, moves)
            sim_result = Expand(nodePtr, &g);
            float reward = std::get<0>(sim_result);
            auto& moves = std::get<1>(sim_result);

            // Backpropagate reward and update AMAF (protecting shared updates)
            BackProp(nodePtr, reward);
            {
                std::lock_guard<std::mutex> lock(amaf_mutex);
                if (nodePtr->GetParent() != nullptr) {
                    UpdateAMAF(nodePtr->GetParent(), moves, reward);
                }
            }
        } else {
            // Node already expanded by some other thread. Do a simulation from this node's grid state.
            // Create a new doneMoves vector and run simulate (this modifies the grid copy)
            std::vector<Coord> doneMoves;
            float reward = Simulate(&g, nodePtr->GetAgent(), &doneMoves);
            BackProp(nodePtr, reward);
            {
                std::lock_guard<std::mutex> lock(amaf_mutex);
                if (nodePtr->GetParent() != nullptr) {
                    UpdateAMAF(nodePtr->GetParent(), doneMoves, reward);
                }
            }
        }
    }
}

// Threaded MCTS entry
Node* MCTSThreading(HexGrid grid, COLOUR agent, Node* initialTree, int seconds) {
    keepGoing.store(true);
    globalAgentAtomic.store(agent);

    // if we have no tree, create root and expand once
    if (initialTree == nullptr) {
        initialTree = new Node(agent);
        initialTree->SetRoot(true);
        HexGrid g(grid);
        Node* nodePtr = Traverse(initialTree, &g);
        auto [reward, moves] = Expand(nodePtr, &g);
        BackProp(nodePtr, reward);
        // UpdateAMAF(nodePtr->GetParent(), moves, reward);
    }

    int hw = (int)std::thread::hardware_concurrency();
    int numThreads = std::max(1, hw) - 1; // leave one CPU for main thread if possible
    Node* best_node = nullptr;

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);

    if (numThreads > 0) {
        std::vector<std::thread> threads;
        threads.reserve(numThreads + 1);

        // Stop-monitor thread (index 0)
        threads.emplace_back([&]() {
            Node* stopResult = MCTSStop(initialTree, agent, end);
            // store best_node in a thread-safe manner
            if (stopResult) {
                std::lock_guard<std::mutex> lock(selection_mutex);
                best_node = stopResult;
            }
        });

        // Worker threads
        for (int i = 1; i <= numThreads; ++i) {
            threads.emplace_back([&, i]() {
                MCTSInnerLoop(initialTree, end, grid);
            });
        }

        // join all threads
        for (auto &t : threads) {
            if (t.joinable()) t.join();
        }
    } else {
        // single-threaded fallback: run inner loop directly
        MCTSInnerLoop(initialTree, end, grid);
    }

    // After stopping, pick the best child from the root according to your mctsmetric
    {
        std::lock_guard<std::mutex> lock(selection_mutex);
        auto children = initialTree->GetChildren();
        if (children.size() == 0) {
            // no children -> fallback random legal move
            Coord fallback = grid.Moves()[randIndex(grid.GetFree())];
            std::cout << "mcts out: no children, playing fallback " << fallback.q << "," << fallback.r << std::endl;
            return nullptr;
        }
        // initialize best_node randomly
        best_node = children[randIndex((int)children.size())];
        float best_metric = best_node->mctsmetric();

        int visits_sum = 0;
        int minvisits = INT32_MAX;
        int maxvisits = -1;
        float mvmet = 0, mvscr = 0;

        for (Node* n : children) {
            float m = n->mctsmetric();
            if ((m > best_metric) == (agent == BLUE)) {
                best_metric = m;
                best_node = n;
            }
            int v = n->GetVisits();
            visits_sum += v;
            if (v < minvisits) minvisits = v;
            if (v > maxvisits) {
                maxvisits = v;
                mvmet = n->mctsmetric();
                mvscr = n->GetAveReward();
            }
        }

        initialTree->HealthyClean(best_node);

        Coord move;
        if (best_node != nullptr) {
            move = best_node->GetMove();
        } else {
            move = grid.Moves()[randIndex(grid.GetFree())];
        }

        std::cout << "mcts out:\n\tplaying " <<  move.q <<"," << move.r << std::endl <<
            "\twith score " << (best_node ? best_node->GetAveReward() : 0.0f) << std::endl <<
            "\twith metric " << (best_node ? best_node->mctsmetric() : 0.0f) << std::endl <<
            "\tand visits " << (best_node ? best_node->GetVisits() : 0) << std::endl <<
            "\tvisited " << visits_sum << std::endl <<
            "\tmin visits " << minvisits << std::endl <<
            "\tmax visits " << maxvisits << std::endl <<
            "\t\twith metric " << mvmet << std::endl <<
            "\t\twith score " << mvscr << std::endl;

        if (best_node != nullptr) best_node->MakeRoot();
    }

    return best_node;
}

// Update AMAF recursively (protected by caller if necessary)
void UpdateAMAF(Node* root, std::vector<Coord> moves, float value) {
    for (Node* n : root->GetChildren()) {
        auto it = std::find(moves.begin(), moves.end(), n->GetMove());
        if (it != moves.end()) {
            n->IncrementAMAFVisits();
            n->UpdateAMAFScore(value);
        }
    }
    if (!root->IsRoot()) {
        UpdateAMAF(root->GetParent(), moves, value);
    }
}

// Simulate uses globalAgentAtomic for final scoring
float Simulate(HexGrid* grid, COLOUR agent, std::vector<Coord>* doneMoves) {
    STATE state = GetState(*grid);
    if (state != CONT) {
        return WinLooseScore(state, (COLOUR)globalAgentAtomic.load());
    }
    auto moves = grid->Moves();
    if (moves.empty()) {
        return 0.0f;
    }
    int index = randIndex((int)moves.size());
    grid->Update(moves[index], agent);
    doneMoves->push_back(moves[index]);
    return Simulate(grid, flip(agent), doneMoves);
}

// Expand must be protected externally (we used expand_mutex in callers)
std::tuple<float, std::vector<Coord>> Expand(Node* rootPtr, HexGrid* grid) {
    COLOUR agent = rootPtr->GetAgent();
    auto moves = grid->Moves(); //get moves from new grid

    // Run simulation on the grid to get reward and the played moves (AMAF)
    std::vector<Coord>* doneMoves = new std::vector<Coord>();
    float reward = Simulate(grid, agent, doneMoves);

    // Create children for all moves (they will have the opposite agent for the next node)
    COLOUR nextAgent = flip(agent);
    std::vector<Node*> children;
    children.reserve(moves.size());
    for (auto move : moves) {
        children.push_back(new Node{ nextAgent, move, rootPtr });
    }

    {
        // set children under expand lock to avoid races with other threads reading children
        std::lock_guard<std::mutex> lock(expand_mutex);
        rootPtr->SetChildren(children);
    }

    // convert moves to vector to return (not the doneMoves which represents simulation path)
    std::vector<Coord> movesCopy = moves;
    delete doneMoves; // caller uses returned moves (we returned movesCopy)
    return { reward, movesCopy };
}

