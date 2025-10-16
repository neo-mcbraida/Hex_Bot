#include <iostream>
#include "grid.h"
#include "colors.h"


int main(){
	Coord c[] = {{1,1},{2,2},{3,3},{4,4}};
	HexGrid gh;
	HexGrid gh2(gh);
	gh.Update(c[0],RED);
	HexGrid gh3(gh);
	gh2.Update(c[1],RED);
	gh3.Update(c[2],RED);
	gh.Update(c[3],RED);
	if(!(gh.GetColour(c[0])==RED && gh.GetColour(c[3])==RED)){
		std::cout<< "FAIL grid1 placement" << std::endl;
	}
	if(gh.GetColour(c[1])==RED || gh.GetColour(c[2]) ==RED){
		std::cout<< "FAIL grid1 unplaced" << std::endl;
	}

	if(gh2.GetColour(c[1])!=RED){
		std::cout<< "FAIL grid2 placement" << std::endl;
	}
	if(gh2.GetColour(c[0])==RED
	   || gh2.GetColour(c[2]) ==RED
	   || gh2.GetColour(c[3]) ==RED){
		std::cout<< "FAIL grid2 unplaced" << std::endl;
	}

	if(!(gh3.GetColour(c[0])==RED && gh3.GetColour(c[2])==RED)){
		std::cout<< "FAIL grid3 placement" << std::endl;
	}
	if(gh3.GetColour(c[1])==RED || gh3.GetColour(c[3]) ==RED){
		std::cout<< "FAIL grid3 unplaced" << std::endl;
	}
}
