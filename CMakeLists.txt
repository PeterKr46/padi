cmake_minimum_required(VERSION 3.22)
project(padi)

set(CMAKE_CXX_STANDARD 17)

set(PERLIN_NOISE lib/PerlinNoise/PerlinNoise.hpp)

add_subdirectory(lib/SFML)

add_executable(padi main.cpp ${PERLIN_NOISE} src/WorldMap.cpp src/WorldMap.h)
target_compile_features(padi PRIVATE cxx_std_17)
target_link_libraries(padi sfml-system sfml-graphics)

