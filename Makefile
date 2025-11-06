CXX := g++
CXXFLAGS := -O2 -std=c++20 -Wall -Wextra

all: mini_chain

mini_chain: blockchain_simple.cpp custom_hash.hpp user.hpp tx.hpp block.hpp
	$(CXX) $(CXXFLAGS) blockchain_simple.cpp -o $@

clean:
	rm -f mini_chain
