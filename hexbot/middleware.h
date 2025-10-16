#ifndef MIDDLEWARE_H__
#define MIDDLEWARE_H__

#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "colors.h"
#include "coord.h"
#include "grid.h"

// Operator
void HexConnector(
	bool (*isSwap)(int, int),
	Coord (*getMove)(HexGrid *, COLOUR,Coord));

#endif // MIDDLEWARE_H__
