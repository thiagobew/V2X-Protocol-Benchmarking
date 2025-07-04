CXX := g++
CXXFLAGS := -std=c++17 -Wall -O2 -I./EPOS
LDFLAGS := -lcryptopp

# EPOS source files
EPOS_SRC := $(wildcard EPOS/*.cpp) $(wildcard EPOS/*.cc)
EPOS_OBJ := $(EPOS_SRC:.cpp=.o)
EPOS_OBJ := $(EPOS_OBJ:.cc=.o)

# Main target
TARGET := main
MAIN_SRC := main.cc
MAIN_OBJ := $(MAIN_SRC:.cc=.o)

all: $(TARGET)

$(TARGET): $(MAIN_OBJ) $(EPOS_OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(MAIN_OBJ) $(EPOS_OBJ) $(TARGET)

.PHONY: all clean