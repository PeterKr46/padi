//
// Created by Peter on 06/05/2022.
//

#include "Abilities.h"
#include "../../map/Tile.h"
#include "../../level/Cursor.h"
#include "../../AStar.h"

namespace padi {

    bool content::Teleport::cast(padi::Level *lvl, const sf::Vector2i &pos) {
        lvl->hideCursor();

        auto ose = std::make_shared<padi::OneshotEntityStack>(user->getPosition());
        ose->m_animation = lvl->getApollo()->lookupAnim("lightning");
        ose->m_stackSize = 8;
        ose->m_color = user->getColor();
        lvl->addCycleEndListener(ose);
        lvl->getMap()->addEntity(ose);

        auto ap = std::make_shared<padi::AudioPlayback>(lvl->getApollo()->lookupAudio("chord_01"));
        ap->sound.setPitch((abs(pos.x) + abs(pos.y)) % 2 == 0 ? 1.2 : 1.0);
        lvl->addCycleEndListener(ap);

        lvl->getMap()->removeEntity(user);
        auto spawnEvent = std::make_shared<padi::SpawnEvent>(user, pos);
        spawnEvent->onCycleBegin(lvl);
        return true;
    }

    void content::Teleport::castIndicator(padi::Level *level) {
        level->showCursor();
    }

    bool content::Lighten::cast(padi::Level *lvl, const sf::Vector2i &pos) {
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

    void content::Lighten::castIndicator(padi::Level *level) {
        level->showCursor();
    }

    bool content::Lighten::onFrameBegin(Level * lvl, uint8_t frame) {
        if(frame == 8) {
            auto fire = std::make_shared<padi::StaticEntity>(strikePos);
            fire->m_animation = lvl->getApollo()->lookupAnim("fire");
            lvl->getMap()->addEntity(fire);
            return false;
        }
        return true;
    }

    bool content::Darken::cast(padi::Level *lvl, const sf::Vector2i &pos) {lvl->hideCursor();
        strikePos = pos;
        auto strike = std::make_shared<padi::OneshotEntity>(pos);
        strike->m_animation = lvl->getApollo()->lookupAnim("air_strike_large");
        strike->m_color = sf::Color::Black;
        lvl->addCycleEndListener(strike);
        lvl->getMap()->addEntity(strike);
        lvl->getMap()->getTile(pos)->m_walkable = false;
        lvl->addFrameBeginListener(shared_from_this());
        return true;
    }

    void content::Darken::castIndicator(padi::Level *level) {
        level->showCursor();
    }

    bool content::Darken::onFrameBegin(Level * lvl, uint8_t frame) {
        auto tile = lvl->getMap()->getTile(strikePos);
        auto color = tile->getColor();
        color = sf::Color(std::max(48,color.r - 32),std::max(48,color.g - 32),std::max(48,color.b - 32), 255);
        tile->setColor(color);

        if(frame == 8) {
            auto fire = std::make_shared<padi::StaticEntity>(strikePos);
            fire->m_animation = lvl->getApollo()->lookupAnim("fire");
            fire->m_color = sf::Color::Black;
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
        auto tile = level->getMap()->getTile(level->getCursorLocation());
        if(!tile || !tile->m_walkable) {
            level->getCursor()->m_color = sf::Color::Red;
        }
    }

    bool content::Dash::cast(padi::Level *lvl, const sf::Vector2i &pos) {
        lvl->hideCursor();
        auto delta = user->getPosition() - pos;
        bool x = false;
        sf::Vector2i dir{Up};
        if(abs(delta.x) > abs(delta.y)) {
            x = true;
            dir = delta.x < 0 ? Right : Left;
        } else {
            dir = delta.y < 0 ? Down : Up;
        }
        auto finalPos = user->getPosition() + dir * 8;
        for(int i = 1; i < 8; ++i) {
            auto laserPart = std::make_shared<padi::OneshotEntity>(user->getPosition() + dir * i);
            laserPart->m_animation = lvl->getApollo()->lookupAnim(x ? "laser_x_burst" : "laser_y_burst");
            laserPart->m_color = user->getColor();
            lvl->getMap()->addEntity(laserPart);
            lvl->addCycleEndListener(laserPart);
        }
        lvl->getMap()->moveEntity(user, finalPos);
        auto strike = std::make_shared<padi::OneshotEntity>(finalPos);
        strike->m_animation = lvl->getApollo()->lookupAnim("air_strike_large");
        strike->m_color = user->getColor();
        lvl->getMap()->addEntity(strike);
        lvl->addCycleEndListener(strike);
        //auto spawnEvent = std::make_shared<padi::SpawnEvent>(user, finalPos);
        //spawnEvent->onCycleBegin(lvl);
        lvl->centerView(finalPos);
        lvl->moveCursor(finalPos);
        return true;
    }

    void content::Dash::castIndicator(padi::Level *level) {
        level->showCursor();
    }
}