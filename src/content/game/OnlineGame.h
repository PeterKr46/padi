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

    protected:

        void synchronize(std::string const &ownName);

        virtual void initializeCharacters() = 0;

        void assignPlayerAbility(padi::content::PlayerAssignAbilityPayload &payload);

        virtual void synchronizeLobby(std::string const &ownName) = 0;

        virtual void synchronizeSeed() = 0;

        virtual void update() = 0;

        virtual void sendChatMessage(const std::string &msg) = 0;

        void printChatMessage(std::string const &msg);

    protected:
        padi::content::CRTMonitor m_crt;
        padi::UIContext m_uiContext;
        struct {
            Chat ui{{250, 194, 200, 60}};
            std::shared_ptr<padi::AudioPlayback> notification{nullptr};
        } m_chat;

        uint32_t m_seed;
        std::mt19937 m_rand;
        std::shared_ptr<Level> m_level;

        std::map<uint32_t, std::shared_ptr<Character>> m_characters;
        std::shared_ptr<Character> m_activeChar;

        std::shared_ptr<padi::Activity> m_next;
    };

    class HostGame : public OnlineGame {
    public:
        HostGame(std::vector<InOutBox> const & clients, std::string const &name, size_t seed = 8008135);
        void initializeCharacters() override;
        void synchronizeLobby(const std::string &name) override;
        void synchronizeSeed() override;
        void close() override;
        void update() override;
        void takeTurn();
        void advanceTurn();
        void sendChatMessage(const std::string &msg) override;
    private:
        std::queue<uint32_t> m_turnQueue;
        struct {
            std::vector<InOutBox> remotes;
            std::vector<std::string> names;
            uint8_t size;
        } m_lobby;
    };

    class ClientGame : public OnlineGame {
    public:
        ClientGame(InOutBox & host, std::string const &name);
        void initializeCharacters() override;
        void synchronizeLobby(const std::string &name) override;
        void synchronizeSeed() override;
        void close() override;
        void update() override;
        void takeTurn();

        void sendChatMessage(const std::string &msg) override;
    private:
        struct {
            InOutBox host;
            std::vector<std::string> names;
            uint8_t size;
        } m_lobby;
    };
} // content
