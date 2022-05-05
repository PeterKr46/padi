//
// Created by Peter on 01/05/2022.
//

#pragma once

#include "map/Map.h"

namespace padi {

    class AbilityCaster {

    };


    class Ability {

        virtual bool cast(padi::Map* map, std::shared_ptr<AbilityCaster> caster, sf::Vector2i const& pos) = 0;


    };

} // padi
