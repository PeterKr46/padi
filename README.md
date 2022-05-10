# PAdI22_01

---

The goal of this project is to create a turn-based rogue-lite game, centered around the following concept:

> Up to three player characters are spawned into a grid-based, randomly generated level of predefined shape.
> Their goal within each Level is to accomplish a specific quest (e.g., eliminate five enemies), before traveling to a specific point on the map, as the map begins to collapse around them.
> Combat takes place in a "color-themed" manner - enemies attempt to darken the map, the players' job is to maintain a certain level of light or color around them to prevent tiles in their proximity from collapsing.
> Once the players complete their task and reach their destination, they have the option to save their progress or to continue to another randomly generated level. 

## Features

The above concept will be realized the following features, optional/extra features listed in brackets.

 - [ ] Turn-based movement and combat on an isometric grid-based map.
 - [ ] Support for the following input devices:
   - [x] Keyboard
   - [ ] SFML-compatible Controllers as input devices.
 - [ ] Up to three (four?) player characters
 - [ ] Cooperative gameplay
   - [ ] Couch Co-op / Hot Seat
   - [ ] (Local Co-op)
   - [ ] (Network Multiplayer)
     - [ ] (Text-based chat)
 - [x] Randomly generated levels
   - [ ] of increasing difficulty
 - [ ] The option to save and load in between levels
 - [ ] Limited character customization:
   - [ ] Swappable abilites per character via an Inventory-like system
   - [ ] (Color selection per character)
 - [x] Sprite-based animation
 - [ ] Custom shaders
   - [ ] On the map itself
   - [ ] For screenspace effects
 - [ ] Cross-OS gameplay
 - [x] Audio
   - [ ] Soundtrack/Music
   - [ ] Audio Effects for Abilites
 - [ ] Basic AI for mobs
 - [ ] Genetic/Advanced AI for mobs