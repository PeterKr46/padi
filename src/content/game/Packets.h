//
// Created by Peter on 16/06/2022.
//

#pragma once

#include <SFML/Network.hpp>
#include "SFML/Graphics/Color.hpp"

namespace padi::content {

    template<typename T>
    bool ReconstructPayload(sf::Packet &packet, T &t) {
        PayloadType recvType = *reinterpret_cast<const PayloadType *>(packet.getData());
        if (recvType != t.type) {
            printf("[Packet] PAYLOAD ERROR - Type mismatch (expected %hhu, got %hhu).\n", t.type, recvType);
        } else if (packet.getDataSize() != sizeof(T)) {
            printf("[Packet] PAYLOAD ERROR - Size mismatch (expected %zu, got %zu).\n", sizeof(T),
                   packet.getDataSize());
        } else {
            std::memcpy(&t, packet.getData(), packet.getDataSize());
            return true;
        }
        return false;
    }

    enum PayloadType : uint8_t {
        ChatMessage,
        GameStart,
        LobbySize,
        PlayerName,
        GameSeed,
        PlayerSpawn,
        PlayerAbilityCast,
        PlayerAbilityAssign
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
        const PayloadType type = PlayerSpawn;
        sf::Vector2i pos;
        sf::Color color;
        bool local{};
    };

    struct alignas(64) PlayerCastPayload {
        const PayloadType type = PlayerAbilityCast;
        uint8_t ability{};
        sf::Vector2i pos;
    };

    struct alignas(64) PlayerAssignAbilityPayload {
        const PayloadType type = PlayerAbilityAssign;
        uint8_t abilitySlot{};
        uint8_t abilityId{};
    };
}