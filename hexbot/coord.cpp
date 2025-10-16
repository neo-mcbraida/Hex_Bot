#include "coord.h"


// Add > < == operators for std::map
bool Coord::operator==(const Coord& rhs) const {
	return (q == rhs.q && r == rhs.r);
}

bool Coord::operator!=(const Coord& rhs) const {
	return (q != rhs.q || r != rhs.r);
}

bool Coord::operator<(const Coord& rhs) const {
	return (q < rhs.q || (q == rhs.q && r < rhs.r));
}

bool Coord::operator>(const Coord& rhs) const {
	return (q > rhs.q || (q == rhs.q && r > rhs.r));
}

bool Coord::operator<=(const Coord& rhs) const {
	return (q <= rhs.q || (q == rhs.q && r <= rhs.r));
}

bool Coord::operator>=(const Coord& rhs) const {
	return (q >= rhs.q || (q == rhs.q && r >= rhs.r));
}
