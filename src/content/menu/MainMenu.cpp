//
// Created by Peter on 06/05/2022.
//

#include "MainMenu.h"
#include <SFML/Graphics.hpp>
#include "../../ui/Immediate.h"
#include "../game/Game.h"

namespace padi::content {

    MainMenu::MainMenu(sf::RenderTarget *renderTarget, std::string const &apollo, std::string const &spritesheet)
            : m_renderTarget(renderTarget), m_runtime() {
        if (!m_vfxBuffer.create(float(renderTarget->getSize().x) / renderTarget->getSize().y * 256, 256)) {
            printf("[padi::content::MainMenu] Failed to create vfxBuffer.\n");
        }
        m_uiContext.init(apollo, spritesheet);
        m_uiContext.pushTransform().translate(16, 32);
        m_uiContext.setText("play", "Play (SP)",        {8, 6}, false);
        m_uiContext.setText("ip_label", "IP",           {8, 48}, false);
        m_uiContext.setText("ip_input", "127.0.0.1",    {24, 48}, false);
        m_uiContext.setText("connect", "Connect",       {8, 66}, false);
        m_uiContext.popTransform();
    }

    void MainMenu::draw() {
        m_background.getLevel()->update(&m_vfxBuffer);
        m_background.getLevel()->centerView({-3, 3});
        m_background.getLevel()->populateVBO();
        m_vfxBuffer.clear();

        auto states = sf::RenderStates::Default;
        states.transform.scale(
                sf::Vector2f(256.f / m_vfxBuffer.getView().getSize().y, 256.f / m_vfxBuffer.getView().getSize().y));
        m_vfxBuffer.draw(m_background, states);

        m_uiContext.nextFrame();

        {
            m_uiContext.pushTransform().translate(16, 32);
            m_uiContext.updateTextColor("play", Immediate::isFocused(&m_uiContext, "play") ? sf::Color::White : sf::Color(64));
            if (Immediate::Button(&m_uiContext, "play", {0, 0, 96, 24})) {
                m_next = std::make_shared<padi::content::Game>(m_renderTarget);
            }
            std::string t = m_uiContext.getText("ip_input");
            m_uiContext.updateTextColor("ip_input", Immediate::isFocused(&m_uiContext, "ip_input") ? sf::Color::Yellow : sf::Color::White);
            if (Immediate::TextInput(&m_uiContext, "ip_input", &t)) {
                m_uiContext.updateTextString("ip_input", t);
            }
            m_uiContext.updateTextColor("connect", Immediate::isFocused(&m_uiContext, "connect") ? sf::Color::White : sf::Color(64));
            if (Immediate::Button(&m_uiContext, "connect", {0, 60, 96, 24})) {
                printf("asd");
            }
            m_uiContext.popTransform();
        }

        m_vfxBuffer.draw(*this);

        auto rState = sf::RenderStates::Default;
        auto shader = m_background.getLevel()->getApollo()->lookupShader("fpa");
        shader->setUniform("time", m_runtime.getElapsedTime().asSeconds());
        shader->setUniform("paused", m_background.getLevel()->isPaused());
        rState.shader = shader.get();
        rState.texture = &m_vfxBuffer.getTexture();
        m_renderTarget->setView(m_renderTarget->getDefaultView());
        m_renderTarget->draw(m_screenQuad, rState);
    }

    void MainMenu::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        m_uiContext.draw(target, states);
        states.transform.translate(target.getView().getCenter() - target.getView().getSize() / 2.f);
        // TODO
        states.transform.scale(sf::Vector2f(target.getView().getSize().y / 256, target.getView().getSize().y / 256));
    }

    std::shared_ptr<padi::Activity> MainMenu::handoff() {
        return m_next ? m_next : shared_from_this();
    }

    void MainMenu::handleResize(int width, int height) {
        sf::Vector2f halfSize{float(width), float(height)};
        halfSize /= 2.f;
        sf::Vector2f imgSize{m_vfxBuffer.getSize()};
        m_screenQuad[0].position = {0, 0};
        m_screenQuad[0].texCoords = {0, imgSize.y};

        m_screenQuad[1].position = {0, float(height)};
        m_screenQuad[1].texCoords = {0, 0};

        m_screenQuad[2].position = {float(width), float(height)};
        m_screenQuad[2].texCoords = {imgSize.x, 0};

        m_screenQuad[3].position = {float(width), 0};
        m_screenQuad[3].texCoords = imgSize;
    }
} // content