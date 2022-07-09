//
// Created by Peter on 22/06/2022.
//

#include <random>
#include "../../entity/LivingEntity.h"
#include "../abilities/Abilities.h"
#include "../game/Character.h"
#include "SFML/Network/TcpSocket.hpp"
#include "../../net/InOutBox.h"

namespace padi::content {

    class GateUnlock
            : public padi::Ability, public CycleListener, public std::enable_shared_from_this<GateUnlock> {
    public:
        explicit GateUnlock(std::shared_ptr<padi::LivingEntity> user);

        bool cast(const std::weak_ptr<Level> &lvl, const sf::Vector2i &pos) override;

        void castCancel(const std::weak_ptr<Level> &level) override;

        void castIndicator(const std::weak_ptr<Level> &level) override;

        bool isCastComplete() override;

        uint32_t getAbilityType() const override;

        bool onFrameBegin(const std::weak_ptr<padi::Level> &lvl, uint8_t frame) override;
    private:
        bool m_open = false;
        bool m_complete = true;
    };

    class Beacon : public padi::LivingEntity {
    public:
        Beacon(std::string name, padi::AnimationSet const *moveset, const sf::Vector2i &pos);

        bool takeTurn(const std::shared_ptr<OnlineGame> &, const std::shared_ptr<Character> &);
        size_t countSlainEnemies(const std::shared_ptr<OnlineGame> & g) const;

        size_t populate(const padi::Map *map, sf::VertexArray &array, size_t vertexOffset, uint8_t frame, float tileVerticalOffset) const override;

        Character asCharacter();

        size_t m_requiredKills = 4;
    private:
        bool m_open = false;
        bool m_started = false;
        bool m_safe;
    };
} // content

