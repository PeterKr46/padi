//
// Created by Peter on 01/05/2022.
//

#pragma once

#include "../level/SpawnEvent.h"

namespace padi {
    class Ability {
    public:
        virtual bool cast(padi::Level *level, sf::Vector2i const &pos) = 0;

        virtual void rangeIndicator(padi::Level *level, sf::Cursor *cursor) = 0;
    };

    namespace content {
        class AirStrike : public Ability {
        public:
            bool cast(padi::Level *lvl, const sf::Vector2i &pos) override;

            void rangeIndicator(padi::Level *level, sf::Cursor *cursor) override;
        };

        class Teleport : public Ability {
        public:
            std::shared_ptr<LivingEntity> user;
            sf::Vector2i target;

            bool cast(padi::Level *lvl, const sf::Vector2i &pos) override;

            void rangeIndicator(padi::Level *level, sf::Cursor *cursor) override;
        };
    };

} // padi
