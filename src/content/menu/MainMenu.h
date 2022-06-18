//
// Created by Peter on 06/05/2022.
//

#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include "MenuBackground.h"
#include "../../ui/UIContext.h"
#include "../../ui/Activity.h"
#include "SFML/Network/TcpListener.hpp"
#include "SFML/Network/Packet.hpp"
#include "../vfx/CRTMonitor.h"
#include "../../net/Inbox.h"
#include "../ui/Chat.h"

namespace padi::content {

    class MainMenu : public padi::Activity, public std::enable_shared_from_this<Activity> {
    public:
        MainMenu(std::string const &apollo, std::string const &spritesheet);

        void draw(sf::RenderTarget *target) override;

        void handleResize(int width, int height) override;

        std::weak_ptr<padi::Activity> handoff() override;

        void appendChatMessage(const std::string &msg);

    private:
        padi::content::CRTMonitor m_crt;
        padi::UIContext m_uiContext;
        Chat m_chat;

        padi::content::MenuBackground m_background;
        std::shared_ptr<padi::Activity> m_next{nullptr};

        struct {
            bool active{false};
            sf::TcpListener listener;
            std::vector<Inbox> clients;
            // The next client to accept on
            std::shared_ptr<sf::TcpSocket> nextClient;
        } hostRole;

        struct {
            Inbox client;
        } clientRole;

        void initializeHostSession();

        void closeHostSession();

        void updateHost();

        void initializeClientSession();

        void updateClient();

        void sendChatMessage(const std::string &msg);

        void handleChatPacket(ChatMessagePayload &packet);

        void hostStartGame();

        void clientHandleGameStart(GameStartPayload const& payload);

        void closeClientSession();
    };

} // content
