cmake_minimum_required(VERSION 3.22)
project(padi)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib/SFML)

set(PERLIN_NOISE lib/PerlinNoise/PerlinNoise.hpp)

set(PADI src/map/Map.cpp src/map/Map.h src/entity/LivingEntity.cpp src/entity/LivingEntity.h src/media/Animation.cpp src/media/Animation.h src/entity/Entity.h src/entity/Entity.cpp src/media/Apollo.cpp src/media/Apollo.h src/AStar.h src/level/Level.cpp src/level/Level.h src/player/Ability.cpp src/player/Ability.h src/Constants.h src/level/LevelGenerator.cpp src/level/LevelGenerator.h src/level/SpawnEvent.cpp src/level/SpawnEvent.h src/turn/TurnManager.cpp src/turn/TurnManager.h src/level/Cursor.h src/level/Cursor.cpp src/Controls.cpp src/Controls.h src/entity/OneshotEntity.cpp src/entity/OneshotEntity.h src/player/Player.cpp src/player/Player.h src/map/MapButton.cpp src/map/MapButton.h src/ui/UIObject.cpp src/ui/UIObject.h src/map/GridObject.h src/player/Controller.cpp src/player/Controller.h src/map/Tile.cpp src/map/Tile.h src/entity/EntityStack.cpp src/entity/EntityStack.h src/entity/StaticEntity.cpp src/entity/StaticEntity.h src/ui/Button.cpp src/ui/Button.h src/ui/UIContext.cpp src/ui/UIContext.h)

add_executable(padi main.cpp ${PERLIN_NOISE} ${PADI})
target_compile_features(padi PRIVATE cxx_std_17)
target_link_libraries(padi sfml-system sfml-graphics sfml-audio)
