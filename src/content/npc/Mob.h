//
// Created by Peter on 13/06/2022.
//

#pragma once

#include "../../entity/LivingEntity.h"
#include "../abilities/Abilities.h"
#include "../game/Game.h"

namespace padi::content {


    class Mob : public padi::LivingEntity {
    public:
        Mob(std::string  name, padi::AnimationSet const* moveset, const sf::Vector2i &pos);
        bool takeTurn(const std::shared_ptr<Level> &, const std::shared_ptr<Character> &);
        Character asCharacter();
    private:
        std::shared_ptr<padi::content::Walk> m_walk;
        bool m_turnStarted = false;
    };

} // content

