//
// Created by Peter on 06/05/2022.
//

#pragma once

#include "../../level/Level.h"

namespace padi::content {

    class MenuBackground : public sf::Drawable {
    public:
        MenuBackground();

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
        std::weak_ptr<Level> getLevel();
    private:
        std::shared_ptr<Level> m_level;

        void generateLevel();
    };

} // content

