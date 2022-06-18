//
// Created by Peter on 16/06/2022.
//

#pragma once

#include "SFML/Network.hpp"
#include "SFML/Graphics/Color.hpp"

namespace padi::content {

    template<typename T>
    sf::Packet &PackagePayload(sf::Packet &packet, T const& t) {
        if (packet.getDataSize()) {
            packet.clear();
        }
        packet.append(&t, sizeof(t));
        return packet;
    }
    template<typename T>
    sf::Packet PackagePayload(T const& t) {
        sf::Packet packet;
        packet.append(&t, sizeof(t));
        return packet;
    }

    enum PayloadType : uint8_t {
        ChatMessage,
        GameStart,
        LobbySize,
        PlayerName,
        GameSeed,
        CharacterSpawn,
        CharacterAbilityCast,
        CharacterAbilityAssign,
        CharacterTurnBegin,
    };

    struct alignas(64) ChatMessagePayload {
        const PayloadType type = ChatMessage;
        char message[32] = "\0";
    };

    struct alignas(64) GameStartPayload {
        const PayloadType type = GameStart;
    };

    struct alignas(64) LobbySizePayload {
        const PayloadType type = LobbySize;
        uint8_t players{};
    };

    struct alignas(64) PlayerNamePayload {
        const PayloadType type = PlayerName;
        uint8_t player{};
        char name[8] = "\0";
    };

    struct alignas(64) GameSeedPayload {
        const PayloadType type = GameSeed;
        uint32_t seed{};
    };

    struct alignas(64) PlayerSpawnPayload {
        const PayloadType type = CharacterSpawn;
        uint32_t id{};
        sf::Vector2i pos;
        sf::Color color;
        bool local{};
    };

    struct alignas(64) PlayerCastPayload {
        const PayloadType type = CharacterAbilityCast;
        uint8_t ability{};
        sf::Vector2i pos;
    };

    struct alignas(64) PlayerAssignAbilityPayload {
        const PayloadType type = CharacterAbilityAssign;
        uint8_t playerId{};
        uint8_t abilitySlot{};
        uint8_t abilityType{};
        uint8_t abilityProps[16]{};
    };

    struct alignas(64) CharacterTurnBeginPayload {
        explicit CharacterTurnBeginPayload(uint32_t id) : characterId(id) {}
        const PayloadType type = CharacterTurnBegin;
        uint32_t characterId{};
    };
}