CXX := g++
CXXFLAGS := -O2 -std=c++20 -Wall -Wextra

TARGET := blockchain

all: $(TARGET)

$(TARGET): blockchain.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)
