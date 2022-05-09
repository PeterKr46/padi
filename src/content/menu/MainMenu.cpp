//
// Created by Peter on 06/05/2022.
//

#include "MainMenu.h"
#include "../../ui/Button.h"
#include "../../ui/Switch.h"
#include <SFML/Graphics.hpp>

namespace padi::content {
    MainMenu::MainMenu(sf::RenderTarget *renderTarget) :
            m_renderTarget(renderTarget) {
    }

    void MainMenu::draw() {
        m_background.getLevel()->centerView({0,0});
        m_renderTarget->clear();

        m_background.getLevel()->update(m_renderTarget);

        m_background.getLevel()->populateVBO();
        m_background.draw(*m_renderTarget, sf::RenderStates::Default);

        if(!m_focused) m_focused = m_objects.front();
        UIContext::populateVBO();
        UIContext::draw(*m_renderTarget, sf::RenderStates::Default);
    }
} // content