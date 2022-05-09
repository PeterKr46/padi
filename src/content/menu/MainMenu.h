//
// Created by Peter on 06/05/2022.
//

#pragma once

#include "../../ui/UIContext.h"
#include "MenuBackground.h"

namespace padi::content {

    class MainMenu : public padi::UIContext {
    public:
        explicit MainMenu(sf::RenderTarget* renderTarget);

        void draw();

    private:
        padi::content::MenuBackground m_background;
        sf::RenderTarget* m_renderTarget;
    };

} // content
