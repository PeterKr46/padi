//
// Created by Peter on 15/06/2022.
//

#pragma once

#include <memory>
#include <vector>
#include <functional>

namespace padi {
    class Ability;
    class Level;
    class LivingEntity;
}

namespace padi::content {

    struct Character {
        /**
         * A character *can* have an entity associated with it.
         * OPTIONAL: This may be null.
         */
        std::shared_ptr<padi::LivingEntity> entity;
        /**
         * Every ability that a character may cast should be found here.
         */
        std::vector<std::shared_ptr<padi::Ability>> abilities;
        /**
         * A function to be called repeatedly until it returns true which determiens a character's move.
         */
        std::function<bool(const std::shared_ptr<Level> &, const std::shared_ptr<Character> &)> controller;
        /**
         * Characters not marked alive at the end of their turn will be removed from play.
         */
        bool alive{true};
    };
}