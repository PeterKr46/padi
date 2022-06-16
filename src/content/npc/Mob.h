//
// Created by Peter on 13/06/2022.
//

#pragma once

#include "../../entity/LivingEntity.h"
#include "../abilities/Abilities.h"
#include "../game/Character.h"
#include "SFML/Network/TcpSocket.hpp"

namespace padi::content {

    class Corruption {
    public:
        explicit Corruption(sf::Vector2i const& origin, float power = 0.8, float decay = 0.5);


        bool expand(const std::shared_ptr<Level> &, const std::shared_ptr<Character> &);

    private:
        float m_power;
        float m_decay;
        std::vector<sf::Vector2i> m_positions;
    };

    class SelfDestruct : public padi::Ability, public CycleListener, public std::enable_shared_from_this<SelfDestruct> {
    public:
        explicit SelfDestruct(std::shared_ptr<padi::LivingEntity> user);

        bool cast(padi::Level *lvl, const sf::Vector2i &pos) override;

        void castCancel(padi::Level *level) override;

        void castIndicator(padi::Level *level) override;

        bool isCastComplete() override;

        bool onFrameBegin(padi::Level *, uint8_t frame) override;

    private:
        bool m_complete = false;
    };

    class Mob : public padi::LivingEntity {
    public:
        Mob(std::string name, padi::AnimationSet const *moveset, const sf::Vector2i &pos, std::vector<std::shared_ptr<sf::TcpSocket>> & sockets);

        bool takeTurn(const std::shared_ptr<Level> &, const std::shared_ptr<Character> &);

        Character asCharacter();

    private:
        std::shared_ptr<padi::content::Walk> m_walk;
        std::shared_ptr<padi::content::SelfDestruct> m_explode;
        bool m_turnStarted = false;
        std::vector<std::shared_ptr<sf::TcpSocket>> m_sockets;
    };

} // content

