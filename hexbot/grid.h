#ifndef GRID_H_
#define GRID_H_

#include <cstddef>
#include <vector>
#include <map>
#include "colors.h"
#include "coord.h"

class HexGrid{
	private:
		std::map<Coord, COLOUR> grid;
		int free;
		Coord lastMove;
		static const int BOARDSIZE;
	public:
		//deconstrcutor
		~HexGrid();
		HexGrid();
		HexGrid(HexGrid& grid); //copy constuctor
		COLOUR GetColour(int q, int r);
		COLOUR GetColour(Coord c);
		int GetSize();
		int GetFree();
		bool Update(Coord c,COLOUR col);
		bool Update(std::vector<std::vector<COLOUR>> newGrid, int BOARDSIZE);
		std::vector<Coord> Moves();
};

void Print(HexGrid h);

#endif
