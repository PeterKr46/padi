//
// Created by Peter on 13/06/2022.
//

#pragma once

#include "../../entity/LivingEntity.h"
#include "../abilities/Abilities.h"
#include "../game/Character.h"
#include "SFML/Network/TcpSocket.hpp"
#include "../../net/InOutBox.h"

namespace padi::content {


    class SelfDestruct : public padi::Ability, public CycleListener, public std::enable_shared_from_this<SelfDestruct> {
    public:
        explicit SelfDestruct(std::shared_ptr<padi::LivingEntity> user);

        bool cast(const std::weak_ptr<Level> &lvl, const sf::Vector2i &pos) override;

        void castCancel(const std::weak_ptr<Level> &level) override;

        void castIndicator(const std::weak_ptr<Level> &level) override;

        bool isCastComplete() override;

        bool onFrameBegin(std::weak_ptr<padi::Level> const &lvl, uint8_t frame) override;

        uint32_t getAbilityType() const override;

    private:
        bool m_complete = false;
    };

    class ExplosiveMob : public padi::LivingEntity {
    public:
        ExplosiveMob(std::string name, padi::AnimationSet const *moveset, const sf::Vector2i &pos);
        bool takeTurn(const std::shared_ptr<OnlineGame> &, const std::shared_ptr<Character> &);

        Character asCharacter(uint32_t id);

    private:
        size_t usedAbility = 0;
        bool m_turnStarted = false;
    };

} // content

