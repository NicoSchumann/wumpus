# SPDX-FileCopyrightText: 2024 Nico Schumann <nico.schumann@startmail.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.22.1)

set(CXX_STANDARD 17)

project(Wumpus)

# use this if you want to build without debugging symbols
# set(CMAKE_BUILD_TYPE Release)

# Set the default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Add debugging flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Wall -Wno-reorder")

add_executable(wumpus main.cpp)
