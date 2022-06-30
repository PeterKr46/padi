//
// Created by Peter on 15/06/2022.
//

#pragma once

#include <queue>
#include <random>

#include "../../ui/UIContext.h"
#include "../../ui/Activity.h"
#include "../vfx/CRTMonitor.h"
#include "../../net/Packets.h"
#include "../../net/InOutBox.h"
#include "../ui/Chat.h"
#include "../../entity/OneshotEntity.h"

namespace padi {
    class Level;

    class AudioPlayback;
    namespace content {
        struct Character;
        class Narrator;
    }
}

namespace padi::content {

    class OnlineGame : public padi::Activity, public std::enable_shared_from_this<OnlineGame> {
    public:
        void handleResize(int width, int height) override;

        void draw(sf::RenderTarget *target) override;

        std::weak_ptr<padi::Activity> handoff() override;

        void close() override = 0;

        std::weak_ptr<Level> getLevel() const;

        UIContext* getUIContext();

        virtual void broadcast(sf::Packet &packet) = 0;

        virtual void broadcast(sf::Packet &packet, const uint32_t *ignore, uint32_t num_ignored) = 0;

        const std::map<uint32_t, std::shared_ptr<Character>, std::less<>> & getCharacters() const;


        size_t getSeed() const;

    protected:

        void synchronize(std::string const &ownName);

        void assignPlayerAbility(padi::content::CharacterAbilityAssignPayload &payload);

        void printChatMessage(std::string const &msg, bool notify = false);

        virtual void initializeCharacters() = 0;

        virtual void synchronizeLobby(std::string const &ownName) = 0;

        virtual void synchronizeSeed() = 0;

        virtual void update() = 0;

        virtual void sendChatMessage(const std::string &msg) = 0;

    protected:
        padi::content::CRTMonitor m_crt;

        padi::UIContext m_uiContext;
        struct {
            Chat ui{{300, 194, 150, 60}};
            std::shared_ptr<padi::AudioPlayback> notification{nullptr};
        } m_chat;

        uint32_t m_seed;
        std::mt19937 m_rand;
        std::shared_ptr<Level> m_level;

        std::shared_ptr<Narrator> m_narrator;

        std::map<uint32_t, std::shared_ptr<Character>, std::less<>> m_characters;
        std::shared_ptr<Character> m_activeChar;

        std::shared_ptr<padi::Activity> m_next;
    };

    class HostGame : public OnlineGame {
    public:
        HostGame(std::vector<InOutBox> const &clients, std::string const &name, size_t seed = 8008135);

        void initializeCharacters() override;

        void synchronizeLobby(const std::string &name) override;

        void synchronizeSeed() override;

        void close() override;

        void update() override;

        void takeTurn();

        void advanceTurn();

        size_t getLobbySize() const;

        void sendChatMessage(const std::string &msg) override;

        void sendChatGeneric(const std::string &msg, uint32_t from = -1);

        void broadcast(sf::Packet &packet) override;

        void broadcast(sf::Packet &packet, const uint32_t *ignore, uint32_t num_ignored) override;

        uint32_t spawnCharacter(Character const &c, uint32_t owner = 0);

        void signalLevelAdvance();
    private:
        std::queue<uint32_t> m_turnQueue;
        struct {
            std::vector<InOutBox> remotes;
            std::vector<std::string> names;
            uint8_t size;
        } m_lobby;
        sf::Clock m_roundCooldown;

        void advanceLevel();

        void endOfRound();

        bool m_levelComplete = false;
    };

    class ClientGame : public OnlineGame {
    public:
        ClientGame(InOutBox &host, std::string const &name);

        void initializeCharacters() override;

        void synchronizeLobby(const std::string &name) override;

        void synchronizeSeed() override;

        void close() override;

        void update() override;

        void takeTurn();

        void broadcast(sf::Packet &packet) override;

        void broadcast(sf::Packet &packet, const uint32_t *ignore, uint32_t num_ignored) override;

        void sendChatMessage(const std::string &msg) override;

    private:
        struct {
            InOutBox host;
            std::vector<std::string> names;
            uint8_t size;
        } m_lobby;

        void spawnNewCharacter(CharacterSpawnPayload payload);

        void spawnNewEntity(EntitySpawnPayload payload);
    };
} // content
