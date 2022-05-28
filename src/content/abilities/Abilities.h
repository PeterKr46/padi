//
// Created by Peter on 06/05/2022.
//

#pragma once

#include "../../player/Ability.h"

namespace padi::content {

    class Lighten : public padi::Ability, public padi::CycleListener, public std::enable_shared_from_this<Lighten> {
    public:
        sf::Vector2i strikePos;

        bool cast(padi::Level *lvl, const sf::Vector2i &pos) override;

        void castCancel(padi::Level *level) override;

        void castIndicator(padi::Level *level) override;

        bool onFrameBegin(Level *, uint8_t frame) override;
    };

    class Darken : public padi::Ability, public padi::CycleListener, public std::enable_shared_from_this<Darken> {
    public:
        sf::Vector2i strikePos;

        bool cast(padi::Level *lvl, const sf::Vector2i &pos) override;

        void castCancel(padi::Level *level) override;

        void castIndicator(padi::Level *level) override;

        bool onFrameBegin(Level *, uint8_t frame) override;
    };

    class Teleport : public padi::Ability {
    public:
        std::shared_ptr<padi::LivingEntity> user;

        bool cast(padi::Level *lvl, const sf::Vector2i &pos) override;

        void castCancel(padi::Level *level) override;

        void castIndicator(padi::Level *level) override;
    };

    class Walk : public padi::Ability, public padi::CycleListener, public std::enable_shared_from_this<Walk> {
    public:
        std::shared_ptr<padi::LivingEntity> user;
        std::vector<sf::Vector2i> path;

        bool cast(padi::Level *lvl, const sf::Vector2i &pos) override;

        void castCancel(padi::Level *level) override;

        void castIndicator(padi::Level *level) override;

        bool onCycleEnd(Level *) override;

        bool onCycleBegin(Level *) override;
    };

    class Dash : public padi::Ability, public padi::CycleListener, public std::enable_shared_from_this<Dash> {
    public:

        Dash(std::shared_ptr<padi::LivingEntity> user, size_t range);

        bool cast(padi::Level *lvl, const sf::Vector2i &pos) override;

        void castCancel(padi::Level *level) override;

        void castIndicator(padi::Level *level) override;

    private:
        std::shared_ptr<padi::LivingEntity> m_user;
        std::vector<std::shared_ptr<padi::StaticEntity>> m_rangeFX;
        bool m_indicatorOOD{true};
        sf::Vector2i m_direction{0,0};
    };

}
