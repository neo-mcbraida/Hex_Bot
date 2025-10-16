#include <bits/stdc++.h>
#include "decisions.h"
#include "middleware.h"

using namespace std;

// It will be using an external class - HexConnector
// This class will be responsible for handling the requests and responses

bool isSwapDummy(int x, int y){
    return true;
}

bool isSwap11(int x, int y){
    // Swap suggestion for 11x11 board
    // As seen from https://www.hexwiki.net/index.php/Swap#Size_11
    if(x == 0 && y != 10){
        return false;
    }
    if(x == 1 && y <= 8){
        return false;
    }
    if(x == 2 && (y == 0 || y == 10)){
        return false;
    }
    if(x == 8 && (y == 0 || y == 10)){
        return false;
    }
    if(x == 9 && y >= 2){
        return false;
    }
    if(x == 10 && y != 0){
        return false;
    }
    return true;
}

Coord getMoveDummy(HexGrid *board, COLOUR ourColour){
    // Randomly find a place that happens to be NONE
    vector<Coord> noneCoords;
    noneCoords = board->Moves();
    Coord c = noneCoords[rand() % noneCoords.size()];
    return c;
}

int main(){
    HexConnector(isSwap11, getMove);
    printf("Middleware test completed.\n");
    return 0;
}
