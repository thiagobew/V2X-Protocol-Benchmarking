CXX := g++
CXXFLAGS := -std=c++17 -Wall -O2 -I./EPOS
LDFLAGS := -lcryptopp

SRC := $(wildcard *.cpp) $(wildcard *.cc) $(wildcard EPOS/*.cpp) $(wildcard EPOS/*.cc)
OBJ := $(SRC:.cpp=.o)
OBJ := $(OBJ:.cc=.o)

TARGET := main

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean