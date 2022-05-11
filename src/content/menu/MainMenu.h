//
// Created by Peter on 06/05/2022.
//

#pragma once

#include "../../ui/UIContext.h"
#include "MenuBackground.h"
#include "SFML/Graphics/Font.hpp"

namespace padi::content {

    class MainMenu : public padi::UIContext {
    public:
        MainMenu(sf::RenderTarget* renderTarget, std::string const& apollo, std::string const& spritesheet);

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

        void draw();


    private:
        padi::content::MenuBackground m_background;

        sf::Font m_font;
        std::vector<sf::Text> m_text;

        sf::RenderTarget* m_renderTarget;
    };

} // content
