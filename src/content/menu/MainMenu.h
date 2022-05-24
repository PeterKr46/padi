//
// Created by Peter on 06/05/2022.
//

#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include "MenuBackground.h"
#include "../../ui/UIContext.h"
#include "../Activity.h"

namespace padi::content {

    class MainMenu : public padi::Activity {
    public:
        MainMenu(sf::RenderTarget *renderTarget, std::string const &apollo, std::string const &spritesheet);

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

        void draw() override;

        void handleResize(int width, int height) override;

        std::shared_ptr<padi::Activity> handoff() override;

    private:
        sf::RenderTarget *m_renderTarget;
        padi::UIContext m_uiContext;
        sf::RenderTexture m_vfxBuffer;
        sf::VertexArray m_screenQuad{sf::Quads, 4};

        padi::content::MenuBackground m_background;
        std::shared_ptr<padi::Activity> m_next{nullptr};

        sf::Font m_font;
        std::vector<sf::Text> m_text;

    };

} // content
