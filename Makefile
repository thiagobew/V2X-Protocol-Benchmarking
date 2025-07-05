CXX := g++
CXXFLAGS := -std=c++17 -Wall -O3 -I./EPOS
LDFLAGS := -lcryptopp

# EPOS source files
EPOS_SRC := $(wildcard EPOS/*.cpp) $(wildcard EPOS/*.cc)
EPOS_OBJ := $(EPOS_SRC:.cpp=.o)
EPOS_OBJ := $(EPOS_OBJ:.cc=.o)

# Main targets and sources
TARGETS := benchmark energy
BENCHMARK_SRC := benchmark.cc
BENCHMARK_OBJ := $(BENCHMARK_SRC:.cc=.o)
ENERGY_SRC := energy.cc
ENERGY_OBJ := $(ENERGY_SRC:.cc=.o)

all: $(TARGETS)

benchmark: $(BENCHMARK_OBJ) $(EPOS_OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS)

energy: $(ENERGY_OBJ) $(EPOS_OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(BENCHMARK_OBJ) $(ENERGY_OBJ) $(EPOS_OBJ) $(TARGETS)

.PHONY: all clean