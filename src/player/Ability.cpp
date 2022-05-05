//
// Created by Peter on 01/05/2022.
//

#include "Ability.h"
#include "../entity/OneshotEntity.h"

namespace padi {
    bool content::Teleport::cast(padi::Level *lvl, const sf::Vector2i &pos) {
        target = pos;

        auto ose = std::make_shared<padi::OneshotEntityStack>(user->getPosition());
        ose->m_animation = lvl->getApollo()->lookupAnim("lightning");
        ose->m_stackSize = 8;
        lvl->addCycleEndListener(ose);
        lvl->getMap()->addEntity(ose);

        lvl->getMap()->removeEntity(user);
        auto spawnEvent = std::make_shared<padi::SpawnEvent>(user, lvl->getApollo()->lookupAnim("air_strike_large"), pos);
        //spawnEvent->dispatch(lvl);
        spawnEvent->onCycleBegin(lvl);
        return true;
    }

    void content::Teleport::rangeIndicator(padi::Level *level, sf::Cursor *cursor) {

    }

    bool content::AirStrike::cast(padi::Level *lvl, const sf::Vector2i &pos) {
        auto ose = std::make_shared<padi::OneshotEntity>(pos);
        ose->m_animation = lvl->getApollo()->lookupAnim("air_strike_large");
        lvl->addCycleEndListener(ose);
        lvl->getMap()->addEntity(ose);
        return true;
    }

    void content::AirStrike::rangeIndicator(padi::Level *level, sf::Cursor *cursor) {

    }
} // padi