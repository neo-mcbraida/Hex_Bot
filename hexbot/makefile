# Source files and executable
SRCS = colors.cpp coord.cpp decisions.cpp grid.cpp middleware.cpp node.cpp mcts.cpp
make2: 
	$(CXX) $(SRCS) middleware-test.cpp -O2 -lm -Wall --static -o hexbot2

make: 
	$(CXX) $(SRCS) middleware-test.cpp -O2 -lm -Wall --static -o hexbot

debug:
	$(CXX) $(SRCS) middleware-test.cpp -g -lm -Wall --static -o hexbot

testgrid:
	$(CXX) $(SRCS) grid-test.cpp -g -lm -Wall --static -o hexbot
