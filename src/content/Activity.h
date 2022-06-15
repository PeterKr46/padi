//
// Created by peter on 21/05/22.
//

#pragma once

#include <memory>
#include "SFML/Graphics/Drawable.hpp"

namespace padi {

    class Activity : public std::enable_shared_from_this<padi::Activity> {
    public:
        virtual std::shared_ptr<padi::Activity> handoff() = 0;

        virtual void handleResize(int width, int height) { };

        virtual void draw(sf::RenderTarget* target) = 0;

    };

}
