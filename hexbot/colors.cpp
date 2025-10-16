#include "colors.h"

//flip colour value
COLOUR flip(COLOUR c){
	switch (c) {
		case RED:
			return BLUE;
		case BLUE:
			return RED;
		case N:
			#if DEBUG
			std::cout << " Warn: Program asked to flip a colour of N. " << std::endl;
			#endif // DEBUG
			return N;
	}
}
