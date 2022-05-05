//
// Created by Peter on 01/05/2022.
//

#pragma once

#include "../entity/OneshotEntity.h"

namespace padi {
    class Ability {

        virtual bool cast(padi::Level* level, sf::Vector2i const& pos) = 0;
        virtual void rangeIndicator(padi::Level* level, sf::Cursor* cursor) = 0;
    };

    namespace content {
        class AirStrike : public Ability {
        public:
            bool cast(padi::Level *lvl, const sf::Vector2i &pos) override {
                auto ose = std::make_shared<padi::OneshotEntity>(pos);
                ose->m_animation = lvl->getApollo()->lookupAnim("air_strike_large");
                lvl->addCycleBeginListener(ose);
                return true;
            }

            void rangeIndicator(padi::Level *level, sf::Cursor *cursor) override {

            }
        };
    };

} // padi
