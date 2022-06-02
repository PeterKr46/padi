//
// Created by Peter on 06/05/2022.
//

#include "MainMenu.h"
#include <SFML/Graphics.hpp>
#include "../../ui/Immediate.h"
#include "../game/Game.h"

namespace padi::content {

    MainMenu::MainMenu(sf::RenderTarget *renderTarget, std::string const &apollo, std::string const &spritesheet)
            : m_renderTarget(renderTarget)
            , m_runtime() {
        if (!m_vfxBuffer.create(float(renderTarget->getSize().x) / renderTarget->getSize().y * 256, 256)) {
            printf("[padi::content::MainMenu] Failed to create vfxBuffer.\n");
        }
        m_uiContext.init(apollo, spritesheet);
        m_font.setSmooth(false);
        m_font.loadFromFile("../media/prstartk.ttf");
        m_text.emplace_back("<Play>", m_font, 7);
        auto halfWidth = m_text.back().getGlobalBounds().width / 4;
        m_text.back().setPosition(32 + halfWidth, 42);
        m_text.emplace_back("Toggle Pause", m_font, 7);
        m_text.back().setPosition(40, 76);
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
            static bool state = false;
            if (Immediate::Button(&m_uiContext, "menu.play", {0, 0, 96, 32})) {
                m_next = std::make_shared<padi::content::Game>(m_renderTarget);
                if (m_background.getLevel()->isPaused()) m_background.getLevel()->play();
                else m_background.getLevel()->pause();
            }
            if (Immediate::Switch(&m_uiContext, "menu.toggle", {0, 32, 32, 32}, &state)) {
                printf("Toggle One");
            }
            if (state) {
                m_background.getLevel()->pause();
            } else {
                m_background.getLevel()->play();
            }
        }
        m_uiContext.popTransform();

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

        // draw the vertex array
        for (auto const &text: m_text) target.draw(text, states);
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