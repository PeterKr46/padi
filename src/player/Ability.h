//
// Created by Peter on 01/05/2022.
//

#pragma once

#include "../level/SpawnEvent.h"

namespace padi {
    class Ability {
    public:
        virtual bool cast(padi::Level *level, sf::Vector2i const &pos) = 0;

        virtual void castIndicator(padi::Level *level) = 0;

        virtual void castCancel(padi::Level * level) = 0;
    };
    
} // padi
