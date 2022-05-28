//
// Created by Peter on 06/05/2022.
//

#include "Abilities.h"

#include <utility>
#include "../../map/Tile.h"
#include "../../level/Cursor.h"
#include "../../AStar.h"
#include "../../Controls.h"

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

    void content::Teleport::castCancel(padi::Level *level) {
        level->hideCursor();
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

    bool content::Lighten::onFrameBegin(Level *lvl, uint8_t frame) {
        if (frame == 8) {
            auto fire = std::make_shared<padi::StaticEntity>(strikePos);
            fire->m_animation = lvl->getApollo()->lookupAnim("fire");
            lvl->getMap()->addEntity(fire);
            return false;
        }
        return true;
    }

    void content::Lighten::castCancel(padi::Level *level) {
        level->hideCursor();
    }

    bool content::Darken::cast(padi::Level *lvl, const sf::Vector2i &pos) {
        lvl->hideCursor();
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

    bool content::Darken::onFrameBegin(Level *lvl, uint8_t frame) {
        auto tile = lvl->getMap()->getTile(strikePos);
        auto color = tile->getColor();
        color = sf::Color(std::max(48, color.r - 32), std::max(48, color.g - 32), std::max(48, color.b - 32), 255);
        tile->setColor(color);

        if (frame == 8) {
            auto fire = std::make_shared<padi::StaticEntity>(strikePos);
            fire->m_animation = lvl->getApollo()->lookupAnim("fire");
            fire->m_color = sf::Color::Black;
            lvl->getMap()->addEntity(fire);
            return false;
        }
        return true;
    }

    void content::Darken::castCancel(padi::Level *level) {
        level->hideCursor();
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
        if (!tile || !tile->m_walkable) {
            level->getCursor()->m_color = sf::Color::Red;
        }
    }

    void content::Walk::castCancel(padi::Level *level) {
        level->hideCursor();
    }


    content::Dash::Dash(std::shared_ptr<padi::LivingEntity> user, size_t range)
            : m_user(std::move(user)) {
        for (size_t i = 0; i < range; ++i) {
            m_rangeFX.push_back(std::make_shared<padi::StaticEntity>(m_user->getPosition()));
        }
    }

    bool content::Dash::cast(padi::Level *lvl, const sf::Vector2i &pos) {
        if(m_direction.x == 0 && m_direction.y == 0) {
            castCancel(lvl);
            return false;
        }
        for (auto const &i: m_rangeFX) lvl->getMap()->removeUIObject(i);

        auto delta = m_user->getPosition() - pos;
        bool x = m_direction.x != 0;
        auto finalPos = m_user->getPosition() + m_direction * int(m_rangeFX.size() + 1);
        for (size_t i = 0; i < m_rangeFX.size(); ++i) {
            auto laserPart = std::make_shared<padi::OneshotEntity>(m_user->getPosition() + m_direction * int(i + 1));
            laserPart->m_animation = lvl->getApollo()->lookupAnim(x ? "laser_x_burst" : "laser_y_burst");
            laserPart->m_color = m_user->getColor();
            lvl->getMap()->addEntity(laserPart);
            lvl->addCycleEndListener(laserPart);
        }
        lvl->getMap()->moveEntity(m_user, finalPos);
        auto strike = std::make_shared<padi::OneshotEntity>(finalPos);
        strike->m_animation = lvl->getApollo()->lookupAnim("air_strike_large");
        strike->m_color = m_user->getColor();
        lvl->getMap()->addEntity(strike);
        lvl->addCycleEndListener(strike);
        lvl->centerView(finalPos);
        m_indicatorOOD = true;
        m_direction = sf::Vector2i(0, 0);
        return true;
    }

    void content::Dash::castIndicator(padi::Level *lvl) {
        if (padi::Controls::wasKeyPressed(sf::Keyboard::Up)) {
            m_direction = Up;
            m_indicatorOOD = true;
        } else if (padi::Controls::wasKeyPressed(sf::Keyboard::Down)) {
            m_direction = Down;
            m_indicatorOOD = true;
        } else if (padi::Controls::wasKeyPressed(sf::Keyboard::Left)) {
            m_direction = Left;
            m_indicatorOOD = true;
        } else if (padi::Controls::wasKeyPressed(sf::Keyboard::Right)) {
            m_direction = Right;
            m_indicatorOOD = true;
        }
        if (m_indicatorOOD) {
            m_indicatorOOD = false;
            for (size_t i = 0; i < m_rangeFX.size(); ++i) {
                m_rangeFX[i]->m_animation = lvl->getApollo()->lookupAnim("indicator");
                m_rangeFX[i]->m_color = m_user->getColor();
                lvl->getMap()->moveUIObject(m_rangeFX[i], m_user->getPosition() + m_direction * int(i + 1));
            }
        }
    }

    void content::Dash::castCancel(padi::Level *lvl) {
        for (auto const &i: m_rangeFX) lvl->getMap()->removeUIObject(i);
    }
}