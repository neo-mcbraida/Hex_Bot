#ifndef DECISIONS_H_
#define DECISIONS_H_
#include <vector>
#include "colors.h"
#include "coord.h"
#include "grid.h"
#include "node.h"


const int LOOSE_REWARD=-140;
const int WIN_REWARD=100;
const float damping=0.8;
const float mctsdamping=0.99;

const int MAXDEPTH=4;
const int OUTERRUNS=100;
const int INNERRUNS=1000;

//possible game states
// surely it makes more sense to name them REDWIN, BLUEWIN, CONT for minimax and MCTS?
enum STATE{ LOOSING, WINNING,CONT};

STATE GetState(HexGrid h );
float EvaluationFunction(HexGrid h);
float MinMaxEval(HexGrid grid,COLOUR agent,float alpha, float beta,int depth=0);
float MoveEval(HexGrid grid,COLOUR agent,float alpha, float beta,int depth=0);
float qMoveEval(HexGrid grid,COLOUR agent);
Coord BeamMM(HexGrid grid,COLOUR agent);
Coord MinMax(HexGrid grid,COLOUR agent);
Coord getMove(HexGrid *board, COLOUR ourColour,Coord pmove);
int WinLooseScore(STATE s,COLOUR c);

#endif
