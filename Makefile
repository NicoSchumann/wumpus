CXXFLAGS := -Wall -std=c++17

wumpus: main.o
	${CXX} ${LDFLAGS} -o wumpus main.o

main.o: main.cpp
	${CXX} ${CXXFLAGS} -c main.cpp

clean:
	rm -rf wumpus *.o
