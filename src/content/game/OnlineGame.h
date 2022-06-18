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

namespace padi {
    class Level;
    namespace content {
        struct Character;
    }
}

namespace padi::content {

    class OnlineGame : public padi::Activity, public std::enable_shared_from_this<OnlineGame> {
    public:
        OnlineGame(std::vector<InOutBox> sockets, bool hosting, std::string const &name,
                   uint32_t seed = 8008135);

        void handleResize(int width, int height) override;

        void draw(sf::RenderTarget *target) override;

        std::weak_ptr<padi::Activity> handoff() override;

        void close() override;

        std::weak_ptr<Level> getLevel() const;

    private:

        void initializePlayers();
        void initializePlayersHost();
        void initializePlayersClient();

        void assignPlayerAbility(padi::content::PlayerAssignAbilityPayload & payload);

        void propagateLobby(std::string const &name);
        void propagateLobbyHost(const std::string &basicString);
        void propagateLobbyClient(const std::string &basicString);

        void propagateSeed();
        void propagateSeedHost();
        void propagateSeedClient();

        void updateClient();
        void updateHost();

    private:
        padi::content::CRTMonitor m_crt;
        padi::UIContext m_uiContext;
        Chat m_chat;

        uint32_t m_seed;
        std::mt19937 m_rand;
        std::shared_ptr<Level> m_level;
        struct {
            const bool isHost{false};
            std::vector<InOutBox> remotes;
            std::vector<std::string> names;
        } m_lobby;

        std::map<uint32_t, std::shared_ptr<Character>> m_characters;
        std::queue<uint32_t> m_turnQueue;
        std::shared_ptr<Character> m_activeChar;

        void takeTurn();

        void advanceTurnHost();

        std::shared_ptr<padi::Activity> m_next;
    };

} // content
