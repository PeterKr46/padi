cmake_minimum_required(VERSION 3.22)
project(padi)

set(CMAKE_CXX_STANDARD 17)

set(PERLIN_NOISE lib/PerlinNoise/PerlinNoise.hpp src/Map.cpp src/Map.h src/LivingEntity.cpp src/LivingEntity.h src/Actions.h src/Animation.cpp src/Animation.h src/Entity.h src/Entity.cpp src/Apollo.cpp src/Apollo.h src/AStar.h)

add_subdirectory(lib/SFML)

add_executable(padi main.cpp ${PERLIN_NOISE} src/Stage.cpp src/Stage.h)
target_compile_features(padi PRIVATE cxx_std_17)
target_link_libraries(padi sfml-system sfml-graphics)

