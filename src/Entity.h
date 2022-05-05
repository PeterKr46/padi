//
// Created by Peter on 28/04/2022.
//

#pragma once

#include "Map.h"

namespace padi {
    class Entity : public padi::GridPlaceable {
    public:
        explicit Entity(sf::Vector2i const &pos);

        virtual void populate(Map const* map, sf::Vertex *pVertex) const;

        [[nodiscard]] virtual sf::Vector2i getSize() const = 0;

    };
}