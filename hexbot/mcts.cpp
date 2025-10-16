
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
#include "colors.h"
#include "coord.h"
#include "grid.h"
#include "node.h"
#include <array>
#include "decisions.h"
#include "mcts.h"
#include "middleware.h"

//okay so basically what this needs do is
Node* Traverse(Node* root, HexGrid* grid) {
	// if node has not been visited, simulate game, and add children for next visit
	// this is triggered if it is a terminal or unvisited node
	if (root->GetChildren().size() == 0 ) {
		return root;
	}
	Node* best_node = root->GetChildren()[rand()%root->GetChildren().size()];
	for (Node* n : root->GetChildren()) {
		if ((n->GetRaveScore() > best_node->GetRaveScore()) == (root->GetAgent() == BLUE)) {
			best_node = n;
		}
		//if (n->GetVisits() > 0) {
		//	std::cout << n->UCB() << " : " << n->GetAMAF() << " : " << n->GetRaveScore()
		//		<< " visits: " << n->GetVisits() << " AMAF visits: " << n->GetAMAFVisits() << std::endl;
		//}
	}
	if(root->GetAgent() == best_node->GetAgent()){
		std::cout << "diff" <<std::endl;
	}
	grid->Update(best_node->GetMove(), root->GetAgent());
	return Traverse(best_node, grid);
}

void BackProp(Node* nodePtr, float reward) {
	nodePtr->IncrementVisits();
	nodePtr->UpdateReward(reward);
	if (!nodePtr->IsRoot()) {
		BackProp(nodePtr->GetParent(), mctsdamping*reward);
	}
}

bool keepGoing=true;
Node* MCTSStop(Node* tree,COLOUR agent,std::chrono::steady_clock::time_point end){
	Node* best_node = nullptr;
	while(std::chrono::steady_clock::now()<end){
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		if(tree->GetChildren().size()==0){
			continue;
		}
		Node* best_node =
			tree->GetChildren()[rand()%tree->GetChildren().size()];
		#define metty UCB
		float ucb=best_node->metty();
		int maxvisits=0;
		float nucb;
		for (Node* n : tree->GetChildren()) {
			nucb =n->metty();
			if (((nucb > ucb) == (agent == BLUE))) {
				ucb=nucb;
				best_node = n;
			}

			if(maxvisits*2<n->GetVisits()){
				maxvisits=n->GetVisits();
			}
		}
		if(best_node->GetVisits()==maxvisits){
			keepGoing=false;
			return best_node;
		}
	}
	keepGoing=false;
	return best_node;
}

void MCTSInnerLoop(Node* root, std::chrono::steady_clock::time_point end, HexGrid grid) {
	while (keepGoing) { // need to add break clause based on time or something
		HexGrid g(grid); //copy grid
		Node* nodePtr = Traverse(root, &g); //traverse
		auto [reward, moves] = Expand(nodePtr, &g);
		BackProp(nodePtr, reward);
		UpdateAMAF(nodePtr->GetParent(), moves, reward);
	}
}

COLOUR globalAgent;
Node* MCTSThreading(HexGrid grid, COLOUR agent, Node* initialTree, int seconds){
	keepGoing=true;
	globalAgent=agent;
	if(initialTree==nullptr){
		initialTree = new Node(agent);
		initialTree->SetRoot(true);
		HexGrid g(grid); //copy grid
		Node* nodePtr = Traverse(initialTree, &g); //traverse
		auto [reward, moves] = Expand(nodePtr, &g);
		BackProp(nodePtr, reward);
		//UpdateAMAF(nodePtr->GetParent(), moves, reward);// depth will always be one on first move
	}
	// returns 0 if value is not well defined
	int numThreads = std::thread::hardware_concurrency()-1;
	Node* best_node;

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
	if (numThreads != 0) {
		// Create a vector to hold the threads.
		std::vector<std::thread> threads(numThreads);

		threads[0] = std::thread([&](){
			best_node = MCTSStop(initialTree,agent,end);
		});
		// Create and start the threads.
		for (int i = 1; i < numThreads; ++i) {
			threads[i] = std::thread([&, i]() {
				MCTSInnerLoop(initialTree, end, grid);
			});
		}
		// Wait for all threads to finish.
		for (std::thread& t : threads) {
			t.join();
		}
	}
	else {
		MCTSInnerLoop(initialTree, end, grid);
	}
	// return best move

	int visits=0;
	int minvisits=999999;
	int maxvisits=-1;
	float mvmet=0;
	float mvscr=0;
	best_node = initialTree->GetChildren()[rand()%initialTree->GetChildren().size()];
    float ucb=best_node->mctsmetric();
    float nucb;
	for (Node* n : initialTree->GetChildren()) {
        nucb =n->mctsmetric();
		if ((nucb > ucb) == (agent == BLUE)) {
            ucb=nucb;
			best_node = n;
		}
		visits+=n->GetVisits();
		if(minvisits>n->GetVisits()){
			minvisits=n->GetVisits();
		}
		if(maxvisits<n->GetVisits()){
			maxvisits = n->GetVisits();
			mvmet = n->mctsmetric();
			mvscr = n->GetAveReward();
		}
	}

	initialTree->HealthyClean(best_node);

	Coord move;
	if(best_node!=nullptr){
		move = best_node->GetMove();
	}else{
		move=grid.Moves()[rand()%grid.GetFree()];
	}

	std::cout << "mcts out:\n\tplaying " <<  move.q <<"," << move.r << std::endl <<
		"\twith score " << best_node->GetAveReward() << std::endl <<
		"\twith metric " << best_node->mctsmetric() << std::endl <<
		"\tand visits " << best_node->GetVisits() << std::endl <<
		"\tvisited " << visits << std::endl <<
		"\tmin visits " << minvisits << std::endl <<
		"\tmax visits " << maxvisits << std::endl <<
		"\t\twith metric " << mvmet << std::endl <<
		"\t\twith score " << mvscr << std::endl;
	if(best_node!=nullptr){
		best_node->MakeRoot();
	}
	return best_node;
}


void UpdateAMAF(Node* root, std::vector<Coord> moves, float value) {
	for (Node* n : root->GetChildren()) {
		auto it = std::find(moves.begin(), moves.end(), n->GetMove());
		if (it != moves.end()) {
			n->IncrementAMAFVisits();
			n->UpdateAMAFScore(value);
		}
	}
	if (root->IsRoot() == false) {
		UpdateAMAF(root->GetParent(), moves, value);
	}
}

float Simulate(HexGrid* grid, COLOUR agent, std::vector<Coord>* doneMoves) {
	STATE state = GetState(*grid);
	if (state != CONT) {
		return WinLooseScore(state,globalAgent);
	}
	std::vector<Coord> moves = grid->Moves();
	int index = rand() % moves.size();
	grid->Update(moves[index], agent);
	doneMoves->push_back(moves[index]);
	return Simulate(grid, flip(agent), doneMoves);
}

std::tuple<float, std::vector<Coord>> Expand(Node* rootPtr, HexGrid* grid) {
	COLOUR agent = rootPtr->GetAgent();
	auto moves = grid->Moves(); //get moves from new grid
	std::vector<Coord>* doneMoves = new std::vector<Coord>();
	float reward = Simulate(grid, agent, doneMoves); //simulate on old grid
	agent = flip(agent);
	std::vector<Node*> children;
	for (auto move : moves) {
		children.push_back(new Node{ agent, move, rootPtr });
	}
	rootPtr->SetChildren(children);
	return { reward, moves };
}