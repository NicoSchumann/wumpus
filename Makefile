wumpus:
	mkdir build && cd build && cmake .. && make

.PHONY:
	clean

clean:
	rm -rf ./build
