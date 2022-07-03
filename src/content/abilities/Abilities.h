//
// Created by Peter on 06/05/2022.
//

#pragma once

#include "../../entity/Ability.h"
#include "../../level/CycleListener.h"
#include "../../Paths.h"
#include "../../entity/EntityStack.h"

namespace padi {
    class StaticEntity;

    class Level;
}

namespace padi::content {

    namespace AbilityType {
        enum : uint8_t {
            Peep = 0,
            Walk,
            Lighten,
            Teleport,
            Dash,
            Darken,
            SelfDestruct,
            SlugWalk,
            GateUnlock
        };
    }

    class Peep : public padi::Ability {
    public:
        explicit Peep(std::shared_ptr<LivingEntity> user);
        bool isCastComplete() override;
        void castCancel(const std::weak_ptr<Level> &level) override;
        void castIndicator(const std::weak_ptr<Level> &level) override;
        bool cast(const std::weak_ptr<Level> &level, const sf::Vector2i &pos) override;
        [[nodiscard]] uint32_t getAbilityType() const override;
    private:
        std::shared_ptr<StaticEntity> m_infoEntity;
    };

    class Lighten : public padi::Ability, public padi::CycleListener, public std::enable_shared_from_this<Lighten> {
    public:
        explicit Lighten(std::shared_ptr<LivingEntity> user);

        sf::Vector2i strikePos;

        bool cast(const std::weak_ptr<Level> &lvl, const sf::Vector2i &pos) override;

        void castCancel(const std::weak_ptr<Level> &level) override;

        void castIndicator(const std::weak_ptr<Level> &level) override;

        bool onFrameBegin(std::weak_ptr<padi::Level> const &lvl, uint8_t frame) override;

        bool isCastComplete() override;

        uint32_t getAbilityType() const override;

    private:
        bool m_complete{true};
    };

    class Darken : public padi::Ability, public padi::CycleListener, public std::enable_shared_from_this<Darken> {

    public:
        explicit Darken(std::shared_ptr<padi::LivingEntity> user);

        sf::Vector2i strikePos;

        bool cast(const std::weak_ptr<Level> &lvl, const sf::Vector2i &pos) override;

        void castCancel(const std::weak_ptr<Level> &level) override;

        void castIndicator(const std::weak_ptr<Level> &level) override;

        bool onFrameBegin(std::weak_ptr<padi::Level> const &lvl, uint8_t frame) override;

        bool isCastComplete() override;

        uint32_t getAbilityType() const override;

    private:
        bool m_complete{true};
    };

    class Teleport : public padi::Ability, public padi::CycleListener, public std::enable_shared_from_this<Teleport> {
    public:
        explicit Teleport(std::shared_ptr<padi::LivingEntity> user);

        bool cast(const std::weak_ptr<Level> &lvl, const sf::Vector2i &pos) override;

        void castCancel(const std::weak_ptr<Level> &level) override;

        void castIndicator(const std::weak_ptr<Level> &level) override;

        bool isCastComplete() override;

        bool onCycleEnd(std::weak_ptr<padi::Level> const &lvl) override;

        uint32_t getAbilityType() const override;

    private:
        bool m_complete{true};
        std::shared_ptr<padi::StaticEntity> m_ghost;
        std::shared_ptr<padi::EntityColumn> m_ghostFX;
    };

    class Walk
            : public padi::LimitedRangeAbility, public padi::CycleListener, public std::enable_shared_from_this<Walk> {
    public:
        struct Walkable {
            int16_t cutOff;
            bool operator()(const Map* map, std::shared_ptr<Tile> const& t);
        };

        Walk(std::shared_ptr<padi::LivingEntity> user, size_t range, Walkable walkable = Walkable{100});

        bool cast(const std::weak_ptr<Level> &lvl, const sf::Vector2i &pos) override;

        void castCancel(const std::weak_ptr<Level> &level) override;

        void castIndicator(const std::weak_ptr<Level> &level) override;

        bool onCycleEnd(std::weak_ptr<padi::Level> const &lvl) override;

        bool onCycleBegin(std::weak_ptr<padi::Level> const &lvl) override;

        bool onFrameBegin(const std::weak_ptr<padi::Level> &lvl, uint8_t frame) override;

        void recalculateRange(const std::weak_ptr<Level> &level) override;

        bool isCastComplete() override;

        std::vector<sf::Vector2i> const &getPath() const;

        std::vector<sf::Vector2i> const &getPossibleTargets() const;

        uint32_t getAbilityType() const override;

        void writeProperties(uint8_t *data, uint32_t maxSize) override;

        Walkable walkable;
    private:
        bool m_complete{true};
        std::vector<sf::Vector2i> m_path;
    };

    class Dash
            : public padi::LimitedRangeAbility, public padi::CycleListener, public std::enable_shared_from_this<Dash> {
    public:

        Dash(std::shared_ptr<padi::LivingEntity> user, size_t range, Walk::Walkable walkable);

        bool cast(const std::weak_ptr<Level> &lvl, const sf::Vector2i &pos) override;

        void castCancel(const std::weak_ptr<Level> &level) override;

        void castIndicator(const std::weak_ptr<Level> &level) override;

        void recalculateRange(const std::weak_ptr<Level> &level) override;

        bool isCastComplete() override;

        bool onCycleEnd(std::weak_ptr<padi::Level> const &lvl) override;

        uint32_t getAbilityType() const override;

        void writeProperties(uint8_t *data, uint32_t maxSize) override;

    private:
        bool m_complete{true};
        sf::Vector2i m_direction{0, 0};
        Walk::Walkable m_walkable;
    };

}
