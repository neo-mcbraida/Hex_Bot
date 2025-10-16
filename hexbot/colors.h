#ifndef COLORS_H_
#define COLORS_H_

#if DEBUG
#include <iostream>
#endif
enum COLOUR{ N,RED,BLUE};
COLOUR flip(COLOUR c);

//if red min, if blue, max
#define colour_compare(a,b,colour) ((colour==RED) ? std::min(a,b) : std::max(a,b))

#endif // COLORS_H_
