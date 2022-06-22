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
            : public padi::Ability,public std::enable_shared_from_this<GateUnlock> {
    public:
        explicit GateUnlock(std::shared_ptr<padi::LivingEntity> user);

        bool cast(const std::weak_ptr<Level> &lvl, const sf::Vector2i &pos) override;

        void castCancel(const std::weak_ptr<Level> &level) override;

        void castIndicator(const std::weak_ptr<Level> &level) override;

        bool isCastComplete() override;

        uint32_t getAbilityType() const override;

    };

    class EndGate : public padi::LivingEntity {
    public:
        EndGate(std::string name, padi::AnimationSet const *moveset, const sf::Vector2i &pos);

        bool takeTurn(const std::shared_ptr<OnlineGame> &, const std::shared_ptr<Character> &);

        Character asCharacter(uint32_t id);

    private:
        size_t requiredKills = 4;
    };
} // content

