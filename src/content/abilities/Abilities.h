//
// Created by Peter on 06/05/2022.
//

#pragma once

#include "../../player/Ability.h"

namespace padi::content {
class AirStrike : public padi::Ability, public padi::CycleListener, public std::enable_shared_from_this<AirStrike> {
    public:
        sf::Vector2i strikePos;

        bool cast(padi::Level *lvl, const sf::Vector2i &pos) override;

        void castIndicator(padi::Level *level) override;

        bool onFrameBegin(Level *, uint8_t frame) override;
    };

    class Teleport : public padi::Ability {
    public:
        std::shared_ptr <padi::LivingEntity> user;

        bool cast(padi::Level *lvl, const sf::Vector2i &pos) override;

        void castIndicator(padi::Level *level) override;
    };

    class Walk : public padi::Ability, public padi::CycleListener, public std::enable_shared_from_this<Walk> {
    public:
        std::shared_ptr <padi::LivingEntity> user;
        std::vector <sf::Vector2i> path;

        bool cast(padi::Level *lvl, const sf::Vector2i &pos) override;

        void castIndicator(padi::Level *level) override;

        bool onCycleEnd(Level *) override;

        bool onCycleBegin(Level *) override;
    };

}
