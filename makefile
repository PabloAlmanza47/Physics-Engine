CXX = g++
CXXFLAGS = -std=c++17 -Wall
LIBS = -lSDL2

SRC = main.cpp
OUT = game

run: $(OUT)
	./$(OUT)

$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LIBS)

clean:
	rm -f $(OUT)