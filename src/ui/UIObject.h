//
// Created by Peter on 02/05/2022.
//

#pragma once
#include "../map/GridObject.h"
#include "../media/Animation.h"

namespace padi {

    class UIContext;

    class UIObject {
    public:
        UIObject();
        explicit UIObject(sf::IntRect const& rect);

        [[nodiscard]] virtual size_t numQuads() const;
        virtual size_t populate(const padi::UIContext* ctx, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) const;

    protected:
        sf::FloatRect m_bound = sf::FloatRect{0,0,32,32};
        sf::Color m_color = sf::Color::White;
        std::shared_ptr<padi::Animation> m_animation;
    };

} // padi