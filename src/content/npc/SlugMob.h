//
// Created by Peter on 22/06/2022.
//

#pragma once

#include <random>
#include "../../entity/LivingEntity.h"
#include "../abilities/Abilities.h"
#include "../game/Character.h"
#include "SFML/Network/TcpSocket.hpp"
#include "../../net/InOutBox.h"

namespace padi::content {

    class SlugWalk
            : public padi::content::Walk {
    public:
        SlugWalk(std::shared_ptr<padi::LivingEntity> user, size_t range, Walkable walkable = Walkable{100});

        bool cast(const std::weak_ptr<Level> &lvl, const sf::Vector2i &pos) override;
        bool onFrameBegin(const std::weak_ptr<padi::Level> &lvl, uint8_t frame) override;
        uint32_t getAbilityType() const override;
    };

    class SlugMob : public padi::LivingEntity {
    public:
        SlugMob(std::string name, padi::AnimationSet const *moveset, const sf::Vector2i &pos);

        bool takeTurn(const std::shared_ptr<OnlineGame> &, const std::shared_ptr<Character> &);

        Character asCharacter();
    private:

        // Very lazy init
        std::shared_ptr<std::mt19937> m_rand;
        bool m_turnStarted = false;
    };

} // content

