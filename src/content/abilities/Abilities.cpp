//
// Created by Peter on 06/05/2022.
//

#include "Abilities.h"

#include <utility>
#include "../../level/Cursor.h"
#include "../../Controls.h"
#include "../../entity/OneshotEntity.h"
#include "../../level/SpawnEvent.h"

namespace padi {

    bool content::Teleport::cast(padi::Level *lvl, const sf::Vector2i &pos) {
        auto tile = lvl->getMap()->getTile(pos);
        if (!tile || !tile->m_walkable) {
            castCancel(lvl);
            return false;
        }

        lvl->hideCursor();
        lvl->getMap()->removeEntity(m_ghost);
        lvl->getMap()->removeEntity(m_ghostFX);

        auto ose = std::make_shared<padi::OneshotEntityStack>(m_user->getPosition());
        ose->m_animation = lvl->getApollo()->lookupAnim("lightning");
        ose->m_stackSize = 8;
        ose->m_color = m_user->getColor();
        lvl->addCycleEndListener(ose);
        lvl->getMap()->addEntity(ose);

        auto ap = std::make_shared<padi::AudioPlayback>(lvl->getApollo()->lookupAudio("chord_01"));
        ap->sound.setPitch((abs(pos.x) + abs(pos.y)) % 2 == 0 ? 1.2 : 1.0);
        lvl->addCycleEndListener(ap);

        lvl->getMap()->removeEntity(m_user);
        auto spawnEvent = std::make_shared<padi::SpawnEvent>(m_user, pos);
        spawnEvent->onCycleBegin(lvl);
        return true;
    }

    void content::Teleport::castIndicator(padi::Level *level) {
        level->showCursor();
        auto pos = level->getCursorLocation();
        auto tile = level->getMap()->getTile(pos);
        if (!tile || !tile->m_walkable) {
            level->getMap()->removeEntity(m_ghost);
            level->getMap()->removeEntity(m_ghostFX);
            level->getCursor()->m_color = sf::Color::Red;
        } else {
            level->getMap()->moveEntity(m_ghost, level->getCursorLocation());
            level->getMap()->moveEntity(m_ghostFX, level->getCursorLocation());
        }
        m_ghost->m_color = m_user->getColor();
        m_ghost->m_color.a = 128;
        m_ghostFX->m_color = m_ghost->m_color;
        if (!m_ghostFX->m_animation) m_ghostFX->m_animation = level->getApollo()->lookupAnim("lightning_hold");
    }

    void content::Teleport::castCancel(padi::Level *level) {
        level->hideCursor();
        level->getMap()->removeEntity(m_ghost);
        level->getMap()->removeEntity(m_ghostFX);
    }

    content::Teleport::Teleport(std::shared_ptr<padi::LivingEntity> user) : Ability(std::move(user)) {
        m_ghost = std::make_shared<StaticEntity>(sf::Vector2i{0, 0});
        m_ghost->m_animation = m_user->getAnimationSet()->at("idle");
        m_ghostFX = std::make_shared<EntityStack>(sf::Vector2i{0, 0});
        m_ghostFX->m_stackSize = 8;
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

    content::Lighten::Lighten(std::shared_ptr<LivingEntity> user) : Ability(std::move(user)) {

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
        tile->setVerticalOffset(frame % 2);
        if (frame == 8) {
            tile->setVerticalOffset(0);
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

    content::Darken::Darken(std::shared_ptr<padi::LivingEntity> user) : Ability(std::move(user)) {

    }

    bool content::Walk::cast(padi::Level *lvl, const sf::Vector2i &pos) {
        LimitedRangeAbility::cast(lvl, pos);
        lvl->hideCursor();
        if (std::find(m_inRange.begin(), m_inRange.end(), pos) == m_inRange.end()) {
            return false;
        }
        if (!path.empty()) {
            path.clear();
            return true;
        }
        path = padi::FindPath(lvl->getMap(), m_user->getPosition(), pos);
        if (path.empty()) {
            return false;
        }
        lvl->addCycleEndListener(shared_from_this());
        return false;
    }

    bool content::Walk::onCycleEnd(Level *lvl) {
        m_user->intentMove(path.front());
        lvl->addCycleBeginListener(shared_from_this());

        path.erase(path.begin());
        return !path.empty();
    }

    bool content::Walk::onCycleBegin(Level *lvl) {
       /* auto ap = std::make_shared<padi::AudioPlayback>(lvl->getApollo()->lookupAudio("chord_01"));
        float pitches[3]{1.0, 1.2, 0.8};
        ap->sound.setPitch(pitches[rand() % 3]);
        ap->sound.setVolume(30);
        lvl->addCycleEndListener(ap);*/
        m_rangeChanged = true;
        return false;
    }

    void content::Walk::castIndicator(padi::Level *level) {
        LimitedRangeAbility::castIndicator(level);
        level->showCursor();
        auto tile = level->getMap()->getTile(level->getCursorLocation());
        if (!tile || !tile->m_walkable) {
            level->getCursor()->m_color = sf::Color::Red;
        }
    }

    void content::Walk::castCancel(padi::Level *level) {
        LimitedRangeAbility::castCancel(level);
        level->hideCursor();
    }

    content::Walk::Walk(std::shared_ptr<padi::LivingEntity> user, size_t range)
            : padi::LimitedRangeAbility(std::move(user), range) {

    }

    void content::Walk::recalculateRange(Level *level) {
        m_shortestPaths = padi::Crawl(level->getMap(), m_user->getPosition(), getRange());
        m_inRange.clear();
        for (auto &m_shortestPath: m_shortestPaths) {
            m_inRange.emplace_back(m_shortestPath.first);
        }
        std::cout << m_shortestPaths.size() << std::endl;
        m_rangeChanged = false;
    }


    content::Dash::Dash(std::shared_ptr<padi::LivingEntity> user, size_t range)
            : padi::LimitedRangeAbility(std::move(user), range) {
    }

    bool content::Dash::cast(padi::Level *lvl, const sf::Vector2i &pos) {
        if (m_direction.x == 0 && m_direction.y == 0) {
            castCancel(lvl);
            return false;
        }
        LimitedRangeAbility::cast(lvl, pos);
        auto delta = m_user->getPosition() - pos;
        bool x = m_direction.x != 0;
        auto finalPos = m_user->getPosition() + m_direction * int(getRange());
        for (size_t i = 0; i < getRange()-1; ++i) {
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
        m_direction = sf::Vector2i(0, 0);
        return true;
    }

    void content::Dash::castIndicator(padi::Level *lvl) {
        if (padi::Controls::wasKeyPressed(sf::Keyboard::Up)) {
            m_direction = Up;
            m_rangeChanged = true;
        } else if (padi::Controls::wasKeyPressed(sf::Keyboard::Down)) {
            m_direction = Down;
            m_rangeChanged = true;
        } else if (padi::Controls::wasKeyPressed(sf::Keyboard::Left)) {
            m_direction = Left;
            m_rangeChanged = true;
        } else if (padi::Controls::wasKeyPressed(sf::Keyboard::Right)) {
            m_direction = Right;
            m_rangeChanged = true;
        }
        LimitedRangeAbility::castIndicator(lvl);
    }

    void content::Dash::castCancel(padi::Level *lvl) {
        LimitedRangeAbility::castCancel(lvl);
    }

    void content::Dash::recalculateRange(Level *level) {
        m_inRange.resize(getRange());
        sf::Vector2i min = m_user->getPosition();
        for (int i = 0; i < getRange(); ++i) {
            m_inRange[i] = (min + m_direction * (i+1));
        }
        m_rangeChanged = false;
    }
}