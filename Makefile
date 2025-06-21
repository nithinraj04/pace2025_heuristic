#-*-MakeFile-*-

CXX = g++
CXXFLAGS = -O2 -std=c++17
LDFLAGS = -static -lglpk -lltdl -lz

SRC = pace25_heuristic.cpp
TARGET = solver

# Default target
all: $(TARGET)

# Build rule
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)

