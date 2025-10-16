#ifndef COORD_H_
#define COORD_H_

//simple coordinates
class Coord{
public:
	int q,r;

	// Add > < == operators for std::map
	bool operator==(const Coord& rhs) const;
	bool operator!=(const Coord& rhs) const;
	bool operator<(const Coord& rhs) const;
	bool operator>(const Coord& rhs) const;
	bool operator<=(const Coord& rhs) const;
	bool operator>=(const Coord& rhs) const;
};

#endif // COORD_H_
