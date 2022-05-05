//
// Created by Peter on 05/05/2022.
//

#pragma once

#include "UIObject.h"

namespace padi {


    class ScalableUIObject : public padi::UIObject {
    public:
        size_t numQuads() const override;
        size_t populate(const padi::UIContext *ctx, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) override;
    };

} // padi
