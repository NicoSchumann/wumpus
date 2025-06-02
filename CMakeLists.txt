cmake_minimum_required(VERSION 3.22.1)

project(Wumpus)

# Define build options
option(C_BUILD "Build C code" ON)
option(DEBUG_MODE "Enable debug mode" ON)

# Set the default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Add debugging flags if DEBUG_MODE is enabled
if(DEBUG_MODE)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Wall -Wno-reorder")
    add_compile_definitions(DEBUG)
endif()

# Configure sources based on C_BUILD option
if(C_BUILD )
    set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/c")
    add_executable(wumpus ${SOURCES}/main.c)
else()
    set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/cxx")
    add_executable(wumpus ${SOURCES}/main.cxx)
endif()

# Print configuration summary
message("Build type: ${CMAKE_BUILD_TYPE}")
message("Building C code: ${C_BUILD}")
message("Debug mode: ${DEBUG_MODE}")