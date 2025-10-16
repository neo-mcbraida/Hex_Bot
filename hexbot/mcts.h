#ifndef MCTS_H_
#define MCTS_H_
#include <vector>
#include "colors.h"
#include "coord.h"
#include "grid.h"
#include "node.h"

const float MCTSC=1.1f;
#define mctsmetric SecureScore

Node* MCTSThreading(HexGrid grid, COLOUR agent, int seconds);
Node* MCTSThreading(HexGrid grid, COLOUR agent, Node* initialTree,int seconds);
void MCTSInnerLoop(Node* root, std::chrono::steady_clock::time_point end, HexGrid grid);
Node* Traverse(Node* root, HexGrid* grid);
void BackProp(Node* nodePtr, float reward);
float Simulate(HexGrid* grid, COLOUR agent, std::vector<Coord>* doneMoves);
std::tuple<float, std::vector<Coord>> Expand(Node* rootPtr, HexGrid* grid);
void UpdateAMAF(Node* root, std::vector<Coord> moves, float value);
#endif
