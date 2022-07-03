//
// Created by Peter on 16/06/2022.
//

#pragma once

#include <cstring>
#include <cmath>

#include <SFML/Network.hpp>
#include <SFML/Graphics/Color.hpp>
#include "../entity/LivingEntity.h"

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
        EntitySpawn,
        CharacterAbilityCast,
        CharacterAbilityAssign,
        CharacterTurnBegin,
        PlayerDespawn,
        NextLevel,
        InitHP,
        EventSpawn,
        EventDespawn,
        AskPlayer,
        PlayerResponse,
    };

    struct alignas(64) ChatMessagePayload {
        ChatMessagePayload() = default;
        explicit ChatMessagePayload(uint32_t origin, const char* msg) : cid(origin) {
            memcpy(message, msg, std::min<size_t>(strlen(msg), sizeof(message) / sizeof(char)));
        }
        const PayloadType type = ChatMessage;
        uint32_t cid{};
        char message[22] = "\0";
    };

    struct alignas(64) GameStartPayload {
        const PayloadType type = GameStart;
    };

    struct alignas(64) LobbySizePayload {
        LobbySizePayload() = default;
        explicit LobbySizePayload(uint32_t size) : numPlayers(size) {}
        const PayloadType type = LobbySize;
        uint8_t numPlayers{};
    };

    struct alignas(64) PlayerNamePayload {
        const PayloadType type = PlayerName;
        uint32_t cid{};
        char    name[8] = "\0";
    };

    struct alignas(64) GameSeedPayload {
        const PayloadType type = GameSeed;
        uint32_t seed{};
    };

    /**
     * Declares the creation of a new *Character*.
     */
    struct alignas(64) CharacterSpawnPayload {
        enum ControllerType : uint8_t {
            LocalPlayer,
            RemotePlayer,
            Narrator
        };
        const PayloadType type = CharacterSpawn;
        uint32_t        cid{};
        ControllerType  controller{};
    };

    /**
     * Declares the creation of a new *LivingEntity* that belongs to a specific *Character*
     */
    struct alignas(64) EntitySpawnPayload {
        const PayloadType type = EntitySpawn;
        uint32_t        cid{};
        uint32_t        entitytype{EntityType::LIVING};
        sf::Vector2i    pos;
        sf::Color       color;
        char            animations[12] = "\0";
    };

    struct alignas(64) CharacterCastPayload {
        const PayloadType type = CharacterAbilityCast;
        uint8_t         ability{};
        sf::Vector2i    pos;
    };

    struct alignas(64) CharacterAbilityAssignPayload {
        const PayloadType type = CharacterAbilityAssign;
        uint32_t    cid{};
        uint8_t     abilitySlot{};
        uint32_t    abilityType{};
        uint8_t     abilityProps[16]{};
    };

    struct alignas(64) CharacterTurnBeginPayload {
        CharacterTurnBeginPayload() = default;
        explicit CharacterTurnBeginPayload(uint32_t id) : cid(id) {}
        const PayloadType type = CharacterTurnBegin;
        uint32_t cid{};
    };

    struct alignas(64) PlayerDespawnPayload {
        PlayerDespawnPayload() = default;
        explicit PlayerDespawnPayload(uint32_t id) : cid(id) {}
        const PayloadType type = PlayerDespawn;
        uint32_t cid{};
    };

    struct alignas(64) NextLevelPayload {
        const PayloadType type = NextLevel;
    };

    struct alignas(64) InitHPPayload {
        const PayloadType type = InitHP;
        uint32_t cid{};
        uint16_t maxHP{};
        sf::Color color{0x0};
    };

    struct alignas(64) EventSpawnPayload {
        const PayloadType type = EventSpawn;
        sf::Vector2i    pos;
        uint32_t        abilityType{};
        uint8_t         abilityProps[16]{};
    };

    struct alignas(64) EventDespawnPayload {
        const PayloadType type = EventDespawn;
        sf::Vector2i    pos;
    };

    struct alignas(64) AskPlayerPayload {
        const PayloadType type = AskPlayer;
        char message[64] = "Yes/No?\0";
    };

    struct alignas(64) PlayerResponsePayload {
        const PayloadType type = PlayerResponse;
        bool response{};
    };

}