//
// Created by Peter on 13/06/2022.
//

#include "Mob.h"

#include <utility>

namespace padi::content {
    Mob::Mob(std::string name, const padi::AnimationSet *moveset, const sf::Vector2i &pos)
            : LivingEntity(std::move(name), moveset, pos) {
        m_turnStarted = false;
        setColor(sf::Color(64, 64, 64));
    }

    bool Mob::takeTurn(const std::shared_ptr<Level> &level, const std::shared_ptr<Character> &chr) {
        if (!m_walk) {
            m_walk = std::make_shared<padi::content::Walk>(shared_from_this(), 5);
        }
        if (!m_turnStarted) {
            m_walk->castIndicator(level.get());
            auto const &targets = m_walk->getPossibleTargets();
            if (targets.empty()) {
                m_walk->castCancel(level.get());
                return true; // TODO
            }
            auto target = targets[std::rand() % (targets.size()-1)];
            for (auto pos: targets) {
                if (level->getMap()->hasEntities(pos)) {
                    target = pos;
                }
            }
            chr->entity->intentCast(m_walk, target);
            m_turnStarted = true;
        }
        if (!chr->entity->hasCastIntent() && m_walk->isCastComplete()) {
            m_turnStarted = false;
        }
        return !chr->entity->hasCastIntent() && m_walk->isCastComplete();
    }

    Character Mob::asCharacter() {
        return {shared_from_this(),
                {m_walk},
                [=](const std::shared_ptr<Level> &l, const std::shared_ptr<Character> &c) { return takeTurn(l, c); }
        };
    }
} // content