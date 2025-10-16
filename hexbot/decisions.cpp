#include <cmath>
#include <algorithm>
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

int WinLooseScore(STATE s,COLOUR c){
	if(s==WINNING){
		if(c==BLUE){
			return WIN_REWARD; //bluewin + blue
		}else{
			return -LOOSE_REWARD; //bluewin + red
		}
	}else{
		if(c==BLUE){
			return LOOSE_REWARD; //redwin + blue
		}else{
			return -WIN_REWARD; //redwin + red
		}
	}
}


// some breadth first style thing, change if there is a better way
// Treates REDWIN as LOOSING, BLUEWIN as WINNING, i think this would make more sense, also need to edit the Simulate() function to this
// would be faster to pass the agent who has just moved and check if they one only...
STATE GetState(HexGrid h ){
	// ASSUMES BLUE IS LEFT HAND SIDE, cant remember if it is other way round
	STATE state = CONT;
	int q, r;
	std::vector<std::vector<int>> offsets = {{ -1, 0 },	{ -1, 1 },	{ 0, -1 },	{ 0, 1 },	{ 1, -1 },	{ 1, 0 }};
	int BOARDSIZE = h.GetSize();
	// for red and blue
	for (int colour = 1; colour < 3; colour++) { // Red == 1, Blue == 2, (N == 0)
		std::vector<Coord> visited;
		std::queue<Coord> que;
		// along a respective edge
		for (int i = 0; i < BOARDSIZE; i++) {
			if (colour == BLUE) {
				Coord co{ i,0 };
				if(h.GetColour(co)==BLUE){
					visited.push_back(co);
					que.push(co);
				}
			} else {
				Coord co{ 0,i }; 
				if(h.GetColour(co)==RED){
					visited.push_back(co);
					que.push(co);
				}
			}
			while (!que.empty()) {
				Coord coord = que.front();
				que.pop();
				// check win
				if ((coord.r == BOARDSIZE - 1 && colour == BLUE) || (coord.q == BOARDSIZE - 1 && colour == RED)) {
					state = static_cast<STATE>(colour-1); // LOOSING and RED are 0, WINNING and BLUE are 1 so this works
					i = BOARDSIZE;
					colour = 3;
					break;
				}
				// else add neighbours to queue (6 neighbouring hexagons)
				for (int u = 0; u < offsets.size(); u++) {
					q = coord.q + offsets[u][0];
					r = coord.r + offsets[u][1];
					// if both r and q are proper range
					if (q < 0 || q >= BOARDSIZE || r < 0 || r >= BOARDSIZE) { 
						continue;
					}
					Coord neighbour{ q, r };
					if(h.GetColour(q, r) == colour &&
							std::find_if(visited.begin(), visited.end(), [&neighbour](const Coord& c) { return c == neighbour; }) == visited.end()) {
						que.push(neighbour);
						visited.push_back(neighbour);
					}
				}
			}
		}
	}
	return state;
}

float EvaluationFunction(HexGrid h){
	int boardwidth = 11;
	
	//TODO
	//ASSUME THAT RED IS GOING FROM TOP TO BOTTOM AND BLUE IS GOING FROM LEFT TO RIGHT

	//RED
	int hori = 0;//how far along horizontally - left of board is 0
	int vert = 0;//how far along  - top of board is 0
	int bluehoripairs = 0;
	int redvertpairs = 0;
	std::array<std::array<int,2>,3> bfneighbours{{{-1,1},{0,1},{1,0}}};
	//int bfneighbours [3][2] = {{-1,1},{0,1},{1,0}};
	std::array<std::array<int,2>,3> rfneighbours{{{0,1},{1,0},{1,-1}}};
	//int rfneighbours [3][2] = {{0,1},{1,0},{1,-1}};
	while (vert < boardwidth){
		while (hori < boardwidth){
			COLOUR c = h.GetColour(vert, hori);
			if (c == BLUE){
				int count = 0;
				for (std::array<int,2> item:bfneighbours){
					int v = vert+item[0];
					int ho = hori+item[1];
					if (0<=v and v<=10 and 0<=ho and ho<=10){
						COLOUR d = h.GetColour(v, ho);
						if (d == BLUE){
							bluehoripairs = bluehoripairs + 1;
						}
						else if (d == RED){
							count = count + 1;
							bluehoripairs = bluehoripairs - 0.3;
						}
						else{
							//if colour null
							if (item == bfneighbours[0]){
								if ((0<=v-1 and v-1<=10) && (0<=ho+2 and ho+2<=10)){
									COLOUR e = h.GetColour(v-1, ho+2);
									if (e == BLUE){
										bluehoripairs = bluehoripairs + 0.25;
									}else if(e== RED){
										bluehoripairs = bluehoripairs - 0.1;
									}else{
										//do nothing - two empty in a row?
									}
								}
							}
							else if (item == bfneighbours[1]){
								if ((0<=v-1 and v-1<=10) && (0<=ho+2 and ho+2<=10)){
									COLOUR e = h.GetColour(v-1, ho+2);
									if (e == BLUE){
										bluehoripairs = bluehoripairs + 0.25;
									}else if(e== RED){
										bluehoripairs = bluehoripairs - 0.1;
									}else{
										//do nothing - two empty in a row?
									}
								}
								if ((0<=v+1 and v+1<=10) && (0<=ho+1 and ho+1 <=10)){
									COLOUR e = h.GetColour(v+1, ho+1);
									if (e == BLUE){
										bluehoripairs = bluehoripairs + 0.25;
									}else if(e== RED){
										bluehoripairs = bluehoripairs - 0.1;
									}else{
										//do nothing - two empty in a row?
									}
								}
							}
							else{
								//item == 1,0
								if ((0<=v+1 and v+1<=10) && (0<=ho+1 and ho+1<=10)){
									COLOUR e = h.GetColour(v+1, ho+1);
									if (e == BLUE){
										bluehoripairs = bluehoripairs + 0.25;
									}else if(e== RED){
										bluehoripairs = bluehoripairs - 0.1;
									}else{
										//do nothing - two empty in a row?
									}
								}
							}
						}
						if (count >=3){
							bluehoripairs = bluehoripairs - 0.1;
						}
					}
					else{
						//do nothing, skip
					}	
				}
			}else if (c == RED){
				int count = 0;
				for (std::array<int,2> item:rfneighbours){
					int v = vert+item[0];
					int ho = hori+item[1];
					if (0<=v and v<=10 and 0<=ho and ho<=10){
						COLOUR d = h.GetColour(v, ho);
						if (d == RED){
							redvertpairs = redvertpairs + 1;
						}
						else if (d == BLUE){
							count = count + 1;
							redvertpairs = redvertpairs - 0.3;
						}
						else{
							//if colour null
							if (item == rfneighbours[0]){
								if ((0<=v+1 and v+1<=10) && (0<=ho+1 and ho+1<=10)){
									COLOUR e = h.GetColour(v+1, ho+1);
									if (e == RED){
										redvertpairs = redvertpairs + 0.25;
									}else if(e== RED){
										redvertpairs = redvertpairs - 0.1;
									}
								}
							}else if (item == rfneighbours[1]){
								if ((0<=v+1 and v+1<=10) && (0<=ho+1 and ho+1<=10)){
									COLOUR e = h.GetColour(v+1, ho+1);
									if (e == RED){
										redvertpairs = redvertpairs + 0.25;
									}else if(e== BLUE){
										redvertpairs = redvertpairs - 0.1;
									}else{
										//do nothing - two empty in a row?
									}
								}
								if ((0<=v+2 and v+2<=10) && (0<=ho-1 and ho-1<=10)){
									COLOUR e = h.GetColour(v+2, ho-1);
									if (e == RED){
										redvertpairs = redvertpairs + 0.25;
									}else if(e== BLUE){
										redvertpairs = redvertpairs - 0.1;
									}
								}
							}
							else{
								//item == 1,-1
								if ((0<=v+2 and v+2<=10) && (0<=ho-1 and ho-1<=10)){
									COLOUR e = h.GetColour(v+1, ho+1);
									if (e == BLUE){
										bluehoripairs = bluehoripairs + 0.25;
									}else if(e==RED){
										bluehoripairs = bluehoripairs - 0.1;
									}
								}
							}

						}
						if (count >=3){
							redvertpairs = redvertpairs - 0.1;
						}
					}
				}
			}
			hori = hori + 1;
		}
		hori = 0;
		vert = vert + 1;
	}
	float x = (bluehoripairs+1.0)/(redvertpairs+1.0);
	return x;
}

float qMoveEval(HexGrid grid,COLOUR agent){
	STATE s = GetState(grid);
	if(s==CONT){
		return EvaluationFunction(grid);
	} else{
		return WinLooseScore(s,agent);
	}
}

float MinMaxEval(HexGrid grid,COLOUR agent,float alpha, float beta,int depth){
    //gets list of moves
	auto moves = grid.Moves();
	float valuation=colour_compare(99999, -9999, flip(agent));
	float nv;

	//for each move, apply it on a new grid, then evalute this
	for(Coord move : moves){
		HexGrid ng{grid};
		ng.Update(move, agent);
		//continue down tree
		nv= MoveEval(ng, agent, alpha,beta,depth+1);
		valuation = colour_compare(nv ,valuation,agent);
		if (agent==COLOUR::BLUE){
			if (valuation>beta){
				break;
			}
			alpha = std::max(alpha,valuation);
		}else{
			if(valuation < alpha){
				break;
			}
			beta = std::min(beta,valuation);
		}
	}

	//return the minmax
	return valuation;
}

Coord BeamMM(HexGrid grid,COLOUR agent){
	//TODO beam
	//TODO ab pruning

    //gets list of moves
	auto moves = grid.Moves();
	float valuation=colour_compare(9999,-9999, flip(agent));
	float nv;

	auto s = moves.size();
	std::cout << log2(s) << std::endl;
	std::cout << s << std::endl;
	std::cout << std::ceil(s/log2(s)) << std::endl;
	s = std::ceil(s/log2(s));

	int j=0;
	std::vector<float> scores(s,colour_compare(9999,-9999, flip(agent)));
	std::vector<Coord> evaled_moves(s);
	std::vector<HexGrid> goodmoves(s);
	for(Coord move : moves){
		HexGrid ng{grid};
		ng.Update(move, agent);
		float score = qMoveEval(ng,agent);
		for(int i=0;i<s;i++){
			if(colour_compare(scores[i],score,agent)==score){
				nv = scores[i];
				scores[i]=score;
				score=nv;
				j++;
				HexGrid gm = goodmoves[i];
				goodmoves[i]=ng;
				ng=gm;
				Coord c;
				c=evaled_moves[i];
				evaled_moves[i]=move;
				move=c;
			}
		}
		// nv= MoveEval(grid, flip(agent), beta, alpha,depth+1);
		// valuation = compare(nv ,valuation,agent);
	}
	float alpha = -999999;
	float beta = 99999;
	valuation = colour_compare(9999, -9999, flip(agent));

	//for each move, apply it on a new grid, then evalute this
	Coord bestMove;
	for(int i=0;i<s;i++){
		nv= MoveEval(goodmoves[i], agent, alpha, beta,1);
		valuation = colour_compare(nv ,valuation,agent);
		if(valuation==nv){
			bestMove=evaled_moves[i];
			valuation=valuation;
		}
		if (agent==COLOUR::BLUE){
			alpha = std::max(alpha,valuation);
		}else{
			beta = std::min(beta,valuation);
		}
	}
	std::cout << "playing " << bestMove.q << bestMove.r << " with score " << valuation << std::endl;

	//return the minmax
	return bestMove;
}

Coord MinMax(HexGrid grid,COLOUR agent){
    //gets list of moves
	auto moves = grid.Moves();
	float valuation=colour_compare(9999,-9999, flip(agent));
	float nv;
	Coord c;

	float alpha=-9999;
	float beta=-9999;
	//for each move, apply it on a new grid, then evalute this
	for(Coord move : moves){
		HexGrid ng{grid};
		ng.Update(move, agent);
		//continue down tree
		nv= MoveEval(ng, agent, alpha, beta,1);
		valuation = colour_compare(nv ,valuation,agent);
		if(valuation==nv){
			c=move;
		}
		if (agent==COLOUR::BLUE){
			alpha = std::max(alpha,valuation);
		}else{
			beta = std::min(beta,valuation);
		}
	}

	std::cout << "playing " << c.q << c.r << " with score " << valuation << std::endl;
	//return the minmax
	return c;
}

//if winning give reward, if loosing give reward,
//if continuing evaluate the tree
float MoveEval(HexGrid grid,COLOUR agent,float alpha, float beta,int depth){
	STATE s = GetState(grid);
	if(s==CONT){
		if(depth<MAXDEPTH){
			return damping*MinMaxEval(grid, flip(agent), alpha, beta,depth);
		}else{
			return damping*EvaluationFunction(grid);
		}
	} else{
		return damping*WinLooseScore(s,agent);
	}
}


Node* mctsTree=nullptr;
Coord getMove(HexGrid *board, COLOUR ourColour,Coord pmove){
    Coord c;
    int MCTS_time = 20;
    if(board->GetFree() == 121 && ourColour==RED){
        //good openings according to
        //https://www.hexwiki.net/index.php/Opening#Opening_theory
        //https://www.hexwiki.net/index.php/Openings_on_11_x_11#Specific_openings

        Coord openings[] = {{0,1},{1,1},{2,1},{0,2},
                            {3,1},{0,3},{0,4},{0,5},
                            {0,6},{0,7},{0,8},{0,0},
                            {0,0},{8,1},{5,2},{7,2}};
        c = openings[rand()%16];
        std::cout<< "opening move picking " << c.q << ", " << c.r << std::endl;
	} else if(board->GetFree() > 48){
		if(mctsTree!=nullptr && (pmove.q !=-4 && pmove.r!=-2)){
			//make opponents move
			for(auto n : mctsTree->GetChildren()){
				if(pmove == n->GetMove()){
					mctsTree->HealthyClean(n);
					mctsTree=n;
					mctsTree->MakeRoot();
				}
			}
		}
        // To be replaced by MCTS solution.
        std::cout << "playing mcts as " << ((ourColour==COLOUR::RED) ? "red" : "blue") << std::endl;
        mctsTree = MCTSThreading(*board, ourColour, mctsTree,MCTS_time);
		c = mctsTree->GetMove();
    } else if(board->GetFree() >= 12){
        std::cout << "playing beam minimax as " << ((ourColour==COLOUR::RED) ? "red" : "blue") << std::endl;
        c = BeamMM(*board, ourColour);
    }else{
        std::cout << "playing minimax as " << ((ourColour==COLOUR::RED) ? "red" : "blue") << std::endl;
        c = MinMax(*board, ourColour);
    }
    HexGrid ng{*board};
    ng.Update(c,ourColour);
    Print(ng);
    return c;
}
