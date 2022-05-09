//
// Created by Peter on 06/05/2022.
//

#include "MainMenu.h"
#include <SFML/Graphics.hpp>
#include "../../ui/Immediate.h"

namespace padi::content {

    MainMenu::MainMenu(sf::RenderTarget* renderTarget, std::string const& apollo, std::string const& spritesheet) :
            m_renderTarget(renderTarget) {
        init(apollo, spritesheet);
        auto bound = sf::FloatRect(16, 32, 96, 32);
    }

    void MainMenu::draw() {
        m_background.getLevel()->centerView({-3,3});
        m_renderTarget->clear();

        m_background.getLevel()->update(m_renderTarget);

        m_background.getLevel()->populateVBO();
        m_background.draw(*m_renderTarget, sf::RenderStates::Default);

        static bool state = false;
        if(Immediate::Button(this, "menu.play", {16, 32, 96, 32})) {
            printf("Test!");
        }
        if(Immediate::Switch(this,  "menu.toggle", {16, 64, 32,32}, &state)) {

        }
        if(Immediate::Switch(this,  "menu.toggle2", {48, 64, 32,32}, &state)) {

        }
        if(Immediate::Switch(this,  "menu.toggle3", {80, 64, 32,32}, &state)) {

        }

        UIContext::draw(*m_renderTarget, sf::RenderStates::Default);
        UIContext::clear();
    }
} // content