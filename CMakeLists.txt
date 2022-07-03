cmake_minimum_required(VERSION 3.22)
project(padi)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib/SFML)

set(PERLIN_NOISE lib/PerlinNoise/PerlinNoise.hpp)

set(PADI
        src/map/Map.cpp src/map/Map.h src/entity/LivingEntity.cpp src/entity/LivingEntity.h src/media/Animation.cpp src/media/Animation.h src/media/AudioPlayback.cpp src/media/AudioPlayback.h src/entity/Entity.h src/entity/Entity.cpp src/media/Apollo.cpp src/media/Apollo.h src/Paths.h src/level/Level.cpp src/level/Level.h src/entity/Ability.cpp src/entity/Ability.h src/Constants.h src/level/LevelGenerator.cpp src/level/LevelGenerator.h src/level/SpawnEvent.cpp src/level/SpawnEvent.h src/level/Cursor.h src/level/Cursor.cpp src/Controls.cpp src/Controls.h src/entity/OneshotEntity.cpp src/entity/OneshotEntity.h src/map/GridObject.h src/map/Tile.cpp src/map/Tile.h src/entity/EntityStack.cpp src/entity/EntityStack.h src/entity/StaticEntity.cpp src/entity/StaticEntity.h src/ui/UIContext.cpp src/ui/UIContext.h src/content/abilities/Abilities.h src/content/abilities/Abilities.cpp src/content/menu/MainMenu.cpp src/content/menu/MainMenu.h src/content/menu/MenuBackground.h src/content/menu/MenuBackground.cpp src/ui/Immediate.h src/ui/Activity.h src/ui/Immediate.cpp src/Paths.cpp src/content/npc/ExplosiveMob.cpp src/content/npc/ExplosiveMob.h src/content/vfx/MapShaker.cpp src/content/vfx/MapShaker.h src/content/game/OnlineGame.cpp src/content/game/OnlineGame.h src/content/game/LocalPlayerTurn.cpp src/content/game/LocalPlayerTurn.h src/content/game/Character.h src/content/vfx/CRTMonitor.h src/content/vfx/CRTMonitor.cpp src/content/game/RemotePlayerTurn.cpp src/content/game/RemotePlayerTurn.h src/net/Packets.h src/content/game/OnlineGameHost.cpp src/content/game/OnlineGameClient.cpp src/net/InOutBox.cpp src/net/InOutBox.h src/content/ui/Chat.cpp src/content/ui/Chat.h src/map/TileDecoration.cpp src/map/TileDecoration.h src/content/game/Narrator.cpp src/content/game/Narrator.h src/content/npc/SlugMob.cpp src/content/npc/SlugMob.h src/content/npc/EndGate.cpp src/content/npc/EndGate.h src/Utils.h src/Utils.cpp src/content/npc/Event.cpp src/content/npc/Event.h)

add_executable(padi main.cpp ${PERLIN_NOISE} ${PADI})
#target_compile_features(padi PRIVATE cxx_std_17)
#set_property(TARGET padi PROPERTY CXX_STANDARD 17)
target_link_libraries(padi sfml-system sfml-graphics sfml-audio sfml-network)
