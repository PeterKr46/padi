//
// Created by Peter on 06/05/2022.
//

#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include "MenuBackground.h"
#include "../../ui/UIContext.h"
#include "../Activity.h"
#include "SFML/Network/TcpListener.hpp"
#include "SFML/Network/Packet.hpp"

namespace padi::content {

    class MainMenu : public padi::Activity {
    public:
        MainMenu(sf::RenderTarget *renderTarget, std::string const &apollo, std::string const &spritesheet);

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

        void draw(sf::RenderTarget* target) override;

        void handleResize(int width, int height) override;

        std::shared_ptr<padi::Activity> handoff() override;

    private:
        sf::RenderTarget *m_renderTarget;
        padi::UIContext m_uiContext;
        sf::RenderTexture m_vfxBuffer;
        sf::VertexArray m_screenQuad{sf::Quads, 4};

        padi::content::MenuBackground m_background;
        std::shared_ptr<padi::Activity> m_next{nullptr};

        sf::Clock m_runtime;

        struct {
            bool active{false};
            sf::TcpListener listener;
            std::vector<std::shared_ptr<sf::TcpSocket>> clients;
            // The next client to accept on
            std::shared_ptr<sf::TcpSocket> nextClient;
        } hostRole;

        struct {
            std::shared_ptr<sf::TcpSocket> client;
        } clientRole;

        void initializeHostSession();

        void closeHostSession();

        void updateHost();

        void initializeClientSession();

        void updateClient();

        void appendChatMessage(const std::string& msg);
        void sendChatMessage(const std::string& msg);

        void handleChatPacket(sf::Packet &packet);
    };

} // content
