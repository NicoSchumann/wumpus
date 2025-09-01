# SPDX-FileCopyrightText: 2024 Nico Schumann <nico.schumann@startmail.com>
# SPDX-License-Identifier: MIT

wumpus:
	mkdir build && cd build && cmake .. && make

.PHONY:
	clean

clean:
	rm -rf ./build

