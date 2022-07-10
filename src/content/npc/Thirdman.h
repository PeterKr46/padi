//
// Created by Peter on 08/07/2022.
//

#pragma once

#include <random>
#include <SFML/Network/TcpSocket.hpp>

#include "../../entity/LivingEntity.h"
#include "../abilities/Abilities.h"
#include "../game/Character.h"
#include "../../net/InOutBox.h"

namespace padi::content {

    class Eruption : public padi::Ability, public CycleListener, public std::enable_shared_from_this<Eruption> {
    public:
        explicit Eruption(std::shared_ptr<LivingEntity> user);

        bool cast(const std::weak_ptr<Level> &lvl, const sf::Vector2i &pos) override;

        void castCancel(const std::weak_ptr<Level> &level) override;

        void castIndicator(const std::weak_ptr<Level> &level) override;

        bool isCastComplete() override;

        bool onFrameBegin(std::weak_ptr<padi::Level> const &lvl, uint8_t frame) override;

        bool onCycleEnd(const std::weak_ptr<padi::Level> &lvl) override;

        uint32_t getAbilityType() const override;
    private:
        sf::Vector2i m_strikePos;
        bool m_complete = true;
    };

    class DownPour : public padi::Ability, public CycleListener, public std::enable_shared_from_this<DownPour> {
    public:
        explicit DownPour(std::shared_ptr<LivingEntity> user, int radius = 2);

        bool cast(const std::weak_ptr<Level> &lvl, const sf::Vector2i &pos) override;

        void castCancel(const std::weak_ptr<Level> &level) override;

        void castIndicator(const std::weak_ptr<Level> &level) override;

        bool isCastComplete() override;

        bool onCycleEnd(const std::weak_ptr<padi::Level> &lvl) override;

        bool onFrameBegin(const std::weak_ptr<padi::Level> &lvl, uint8_t frame) override;

        uint32_t getAbilityType() const override;
    private:
        int  m_radius = 2;
        sf::Vector2i m_strikePos;
        bool m_complete = true;
    };

    class EruptionTarget : public padi::LivingEntity {
    public:
        EruptionTarget(const std::string &name, const AnimationSet* moveset, const sf::Vector2i &pos);

        bool takeTurn(const std::shared_ptr<OnlineGame> &, const std::shared_ptr<Character> &);

        Character asCharacter();
        int radius;
    private:

        bool m_turnStarted = false;
    };

    class Thirdman : public padi::LivingEntity {
    public:

        Thirdman(const std::string &name, const AnimationSet *moveset, const sf::Vector2i &pos);

        bool takeTurn(const std::shared_ptr<OnlineGame> &, const std::shared_ptr<Character> &);

        Character asCharacter(bool awake);

    private:
        int m_lastAbility = 0;
        // Very lazy init
        std::shared_ptr<std::mt19937> m_rand;
        bool m_turnStarted = false;
    };

} // content