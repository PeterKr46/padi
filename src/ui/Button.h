//
// Created by Peter on 04/05/2022.
//

#pragma once

#include "ScalableUIObject.h"

namespace padi {

    class Button : public padi::ScalableUIObject, public std::enable_shared_from_this<padi::Button> {

    public:
        explicit Button(padi::UIContext const *context);

        size_t populate(const padi::UIContext *ctx, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) override;

    };

} // padi
