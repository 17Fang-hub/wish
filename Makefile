CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic

all: wish

wish: wish.cpp
	$(CXX) $(CXXFLAGS) wish.cpp -o wish

clean:
	rm -f wish

.PHONY: all clean
