//
// Created by Peter on 06/05/2022.
//

#include "MainMenu.h"
#include <SFML/Graphics.hpp>
#include "../../ui/Immediate.h"
#include "../game/Game.h"

namespace padi::content {

    MainMenu::MainMenu(sf::RenderTarget* renderTarget, std::string const& apollo, std::string const& spritesheet) :
            m_renderTarget(renderTarget) {
        m_uiContext.init(apollo, spritesheet);
        m_font.setSmooth(false);
        m_font.loadFromFile("../media/prstartk.ttf");
        m_text.emplace_back("<Play>", m_font, 7);
        auto halfWidth = m_text.back().getGlobalBounds().width / 4;
        m_text.back().setPosition(32 + halfWidth, 42);
    }

    void MainMenu::draw() {
        m_background.getLevel()->centerView({-3,3});
        m_renderTarget->clear();
        m_uiContext.clear();

        m_background.getLevel()->update(m_renderTarget);

        m_background.getLevel()->populateVBO();
        m_background.draw(*m_renderTarget, sf::RenderStates::Default);

        static bool state = false;
        if(Immediate::Button(&m_uiContext, "menu.play", {16, 32, 96, 32})) {
            m_next = std::make_shared<padi::content::Game>(m_renderTarget);
            if(m_background.getLevel()->isPaused()) m_background.getLevel()->play();
            else m_background.getLevel()->pause();
        }
        if(Immediate::Switch(&m_uiContext,  "menu.toggle", {16, 64, 32,32}, &state)) {
            printf("Toggle One");
        }
        if(Immediate::Switch(&m_uiContext,  "menu.toggle2", {48, 64, 32,32}, &state)) {
            printf("Toggle Two");
        }
        if(Immediate::Switch(&m_uiContext,  "menu.toggle3", {80, 64, 32,32}, &state)) {
            printf("Toggle Three");
        }
        m_renderTarget->draw(*this);
    }

    void MainMenu::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        m_uiContext.draw(target, states);
        states.transform.translate(target.getView().getCenter() - target.getView().getSize() / 2.f);
        // TODO
        states.transform.scale(sf::Vector2f(target.getView().getSize().y / 256, target.getView().getSize().y / 256));

        // draw the vertex array
        for(auto const& text : m_text) m_renderTarget->draw(text, states);
    }

    std::shared_ptr<padi::Activity> MainMenu::handoff() {
        return m_next ? m_next : shared_from_this();
    }
} // content