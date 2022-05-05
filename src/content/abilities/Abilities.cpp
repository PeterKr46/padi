//
// Created by Peter on 06/05/2022.
//

#include "Abilities.h"
#include "../../entity/OneshotEntity.h"
#include "../../map/Tile.h"
#include "../../level/Cursor.h"
#include "../../AStar.h"

namespace padi {

    bool content::Teleport::cast(padi::Level *lvl, const sf::Vector2i &pos) {
        lvl->hideCursor();

        auto ose = std::make_shared<padi::OneshotEntityStack>(user->getPosition());
        ose->m_animation = lvl->getApollo()->lookupAnim("lightning");
        ose->m_stackSize = 8;
        lvl->addCycleEndListener(ose);
        lvl->getMap()->addEntity(ose);

        auto ap = std::make_shared<padi::AudioPlayback>(lvl->getApollo()->lookupAudio("chord_01"));
        ap->sound.setPitch((abs(pos.x) + abs(pos.y)) % 2 == 0 ? 1.2 : 1.0);
        lvl->addCycleEndListener(ap);

        lvl->getMap()->removeEntity(user);
        auto spawnEvent = std::make_shared<padi::SpawnEvent>(user, lvl->getApollo()->lookupAnim("air_strike_large"),
                                                             pos);
        spawnEvent->onCycleBegin(lvl);
        return true;
    }

    void content::Teleport::castIndicator(padi::Level *level) {
        level->showCursor();
    }

    bool content::AirStrike::cast(padi::Level *lvl, const sf::Vector2i &pos) {
        lvl->hideCursor();
        strikePos = pos;
        auto strike = std::make_shared<padi::OneshotEntity>(pos);
        strike->m_animation = lvl->getApollo()->lookupAnim("air_strike_large");
        lvl->addCycleEndListener(strike);
        lvl->getMap()->addEntity(strike);
        lvl->getMap()->getTile(pos)->m_walkable = false;
        lvl->addFrameBeginListener(shared_from_this());
        return true;
    }

    void content::AirStrike::castIndicator(padi::Level *level) {
        level->showCursor();
    }

    bool content::AirStrike::onFrameBegin(Level * lvl, uint8_t frame) {
        if(frame == 8) {
            auto fire = std::make_shared<padi::StaticEntity>(strikePos);
            fire->m_animation = lvl->getApollo()->lookupAnim("fire");
            lvl->getMap()->addEntity(fire);
            return false;
        }
        return true;
    }

    bool content::Walk::cast(padi::Level *lvl, const sf::Vector2i &pos) {
        lvl->hideCursor();

        if (!path.empty()) {
            path.clear();
            return true;
        }
        path = padi::FindPath(lvl->getMap(), user->getPosition(), pos);
        if (path.empty()) {
            return false;
        }
        lvl->addCycleEndListener(shared_from_this());
        return false;
    }

    bool content::Walk::onCycleEnd(Level *lvl) {
        user->intentMove(path.front());
        lvl->addCycleBeginListener(shared_from_this());

        path.erase(path.begin());
        return !path.empty();
    }

    bool content::Walk::onCycleBegin(Level *lvl) {
        auto ap = std::make_shared<padi::AudioPlayback>(lvl->getApollo()->lookupAudio("chord_01"));
        float pitches[3]{1.0, 1.2, 0.8};
        ap->sound.setPitch(pitches[rand() % 3]);
        ap->sound.setVolume(30);
        lvl->addCycleEndListener(ap);
        return false;
    }

    void content::Walk::castIndicator(padi::Level *level) {
        level->showCursor();
        if(!level->getMap()->getTile(level->getCursorLocation())->m_walkable) {
            level->getCursor()->m_color = sf::Color::Red;
        }
    }
}