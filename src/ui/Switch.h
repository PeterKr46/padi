//
// Created by Peter on 05/05/2022.
//

#pragma once

#include "UIObject.h"
#include "SFML/Audio/Sound.hpp"

namespace padi {

    class Switch : public padi::UIObject, public std::enable_shared_from_this<Switch> {
    public:
        explicit Switch(padi::UIContext *context);

        size_t
        populate(const padi::UIContext *ctx, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) override;

    public:
        sf::Sound m_soundSource;
        bool m_state{false};
    };

} // padi