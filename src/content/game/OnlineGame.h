//
// Created by Peter on 15/06/2022.
//

#pragma once

#include <queue>
#include <random>

#include "../../ui/UIContext.h"
#include "../Activity.h"
#include "../vfx/CRTMonitor.h"
#include "Packets.h"
#include "Inbox.h"

namespace padi {
    class Level;
    namespace content {
        struct Character;
    }
}

namespace padi::content {

    class OnlineGame : public padi::Activity, public std::enable_shared_from_this<OnlineGame> {
    public:
        OnlineGame(std::vector<Inbox> sockets, bool hosting, std::string const &name,
                   uint32_t seed = 8008135);

        void handleResize(int width, int height) override;

        void draw(sf::RenderTarget *target) override;

        std::shared_ptr<padi::Activity> handoff() override;

        void close() override;

        std::shared_ptr<Level> getLevel() const;

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

    private:
        padi::content::CRTMonitor m_crt;
        padi::UIContext m_uiContext;

        uint32_t m_seed;
        std::mt19937 m_rand;
        std::shared_ptr<Level> m_level;
        struct {
            const bool isHost{false};
            std::vector<Inbox> remotes;
            std::vector<std::string> names;
        } m_lobby;

        std::map<uint32_t, std::shared_ptr<Character>> m_characters;
        std::queue<std::shared_ptr<Character>> m_turnQueue;
        std::shared_ptr<Character> m_activeChar;
    };

} // content
