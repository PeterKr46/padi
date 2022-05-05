cmake_minimum_required(VERSION 3.22)
project(padi)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib/SFML)

set(PERLIN_NOISE lib/PerlinNoise/PerlinNoise.hpp )

set(PADI src/map/Map.cpp src/map/Map.h src/entity/LivingEntity.cpp src/entity/LivingEntity.h src/animation/Animation.cpp src/animation/Animation.h src/entity/Entity.h src/entity/Entity.cpp src/animation/Apollo.cpp src/animation/Apollo.h src/AStar.h src/level/Level.cpp src/level/Level.h src/Ability.cpp src/Ability.h src/Constants.h)

add_executable(padi main.cpp ${PERLIN_NOISE} ${PADI})
target_compile_features(padi PRIVATE cxx_std_17)
target_link_libraries(padi sfml-system sfml-graphics)

