wumpus:
	mkdir build && cd build && cmake .. && make

.PHONY:
	clean wumpus

clean:
	rm -rf ./build
