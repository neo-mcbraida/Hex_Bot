#include <iostream>
#include "grid.h"

HexGrid::~HexGrid(){}
const int HexGrid::BOARDSIZE = 11;
HexGrid::HexGrid(){free = BOARDSIZE*BOARDSIZE;}
HexGrid::HexGrid(HexGrid& grid){this->grid = grid.grid; this->free = grid.free;}
COLOUR HexGrid::GetColour(Coord c) {return grid[c];}
COLOUR HexGrid::GetColour(int q, int r) {return GetColour(Coord{q,r});}
int HexGrid::GetFree() {return free;}
int HexGrid::GetSize() {return BOARDSIZE;}

//modify the board
bool HexGrid::Update(Coord c,COLOUR col){
	if(grid[c]==COLOUR::N){
		grid[c]=col;
		lastMove=c;
		free--;
		return true;
	}
	return true;
}

bool HexGrid::Update(std::vector<std::vector<COLOUR>> newGrid, int BOARDSIZE){
	for (int i = 0; i < BOARDSIZE; i++){
		for (int j = 0; j < BOARDSIZE; j++){
			Coord c{ i,j };
			if (newGrid[i][j] != COLOUR::N && grid[c] != newGrid[i][j]){
				grid[c] = newGrid[i][j];
				free--;
			}
		}
	}
	return true;
}

//return list of moves
std::vector<Coord> HexGrid::Moves(){
	std::vector<Coord> m(free);
	int finds=0;
	int i{0},j;
	for(i=0;i<BOARDSIZE;i++){
		for(j=0;j<BOARDSIZE;j++){
			Coord c{i,j};
			if(grid[c]==COLOUR::N || grid[c]==NULL){
				m[finds]=c;
				finds++;
			}
		}
	}
	return m;
}

#define redansi "\033[41;30m"
#define blueansi "\033[44;30m"
#define resetansi "\033[0m"
void Print(HexGrid h){
	for(int q=0;q<11;q++){
		for(int i=0;i<q;i++){
			std::cout << " ";
		}
		for(int r=0;r<11;r++){
			COLOUR c = h.GetColour(q,r);
			if(c==COLOUR::RED){
				std::cout <<redansi<< "R";
			}else if (c==COLOUR::BLUE){
				std::cout <<blueansi<< "B";
			}else{
				std::cout << "0";
			}
			std::cout << resetansi<<" ";
		}
		std::cout <<std::endl;
	}
}
