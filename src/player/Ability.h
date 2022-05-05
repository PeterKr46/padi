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
    };

    namespace content {
        class AirStrike : public Ability {
        public:
            bool cast(padi::Level *lvl, const sf::Vector2i &pos) override;

            void castIndicator(padi::Level *level) override;
        };

        class Teleport : public Ability {
        public:
            std::shared_ptr<LivingEntity> user;

            bool cast(padi::Level *lvl, const sf::Vector2i &pos) override;

            void castIndicator(padi::Level *level) override;
        };

        class Walk : public Ability, public CycleListener, public std::enable_shared_from_this<Walk> {
        public:
            std::shared_ptr<LivingEntity> user;
            std::vector<sf::Vector2i> path;

            bool cast(padi::Level *lvl, const sf::Vector2i &pos) override;

            void castIndicator(padi::Level *level) override;

            bool onCycleEnd(Level *) override;

            bool onCycleBegin(Level *) override;
        };
    };

} // padi
