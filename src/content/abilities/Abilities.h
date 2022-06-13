//
// Created by Peter on 06/05/2022.
//

#pragma once

#include "../../player/Ability.h"
#include "../../level/CycleListener.h"
#include "../../Paths.h"
#include "../../entity/EntityStack.h"

namespace padi {
    class StaticEntity;
    class Level;
}

namespace padi::content {

    class Lighten : public padi::Ability, public padi::CycleListener, public std::enable_shared_from_this<Lighten> {
    public:
        explicit Lighten(std::shared_ptr<LivingEntity> user);
        sf::Vector2i strikePos;

        bool cast(padi::Level *lvl, const sf::Vector2i &pos) override;

        void castCancel(padi::Level *level) override;

        void castIndicator(padi::Level *level) override;

        bool onFrameBegin(padi::Level *, uint8_t frame) override;
    };

    class Darken : public padi::Ability, public padi::CycleListener, public std::enable_shared_from_this<Darken> {

    public:
        explicit Darken(std::shared_ptr<padi::LivingEntity> user);
        sf::Vector2i strikePos;

        bool cast(padi::Level *lvl, const sf::Vector2i &pos) override;

        void castCancel(padi::Level *level) override;

        void castIndicator(padi::Level *level) override;

        bool onFrameBegin(Level *, uint8_t frame) override;
    };

    class Teleport : public padi::Ability {
    public:
        explicit Teleport(std::shared_ptr<padi::LivingEntity> user);

        bool cast(padi::Level *lvl, const sf::Vector2i &pos) override;

        void castCancel(padi::Level *level) override;

        void castIndicator(padi::Level *level) override;
    private:
        std::shared_ptr<padi::StaticEntity> m_ghost;
        std::shared_ptr<padi::EntityStack> m_ghostFX;
    };

    class Walk : public padi::LimitedRangeAbility, public padi::CycleListener, public std::enable_shared_from_this<Walk> {
    public:
        Walk(std::shared_ptr<padi::LivingEntity> user, size_t range);

        bool cast(padi::Level *lvl, const sf::Vector2i &pos) override;

        void castCancel(padi::Level *level) override;

        void castIndicator(padi::Level *level) override;

        bool onCycleEnd(Level *) override;

        bool onCycleBegin(Level *) override;

        void recalculateRange(Level* level) override;

        std::vector<sf::Vector2i> const& getPath() const;
        std::vector<sf::Vector2i> const& getPossibleTargets() const;

    private:
        std::vector<sf::Vector2i> m_path;
        std::map<sf::Vector2i, sf::Vector2i, padi::compair> m_shortestPaths;
    };

    class Dash
            : public padi::LimitedRangeAbility, public padi::CycleListener, public std::enable_shared_from_this<Dash> {
    public:

        Dash(std::shared_ptr<padi::LivingEntity> user, size_t range);

        bool cast(padi::Level *lvl, const sf::Vector2i &pos) override;

        void castCancel(padi::Level *level) override;

        void castIndicator(padi::Level *level) override;

        void recalculateRange(Level* level) override;

    private:
        sf::Vector2i m_direction{0, 0};
    };

}
