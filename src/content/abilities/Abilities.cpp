//
// Created by Peter on 06/05/2022.
//

#include "Abilities.h"

#include <utility>
#include "../../level/Cursor.h"
#include "../../Controls.h"
#include "../../entity/OneshotEntity.h"
#include "../../level/SpawnEvent.h"
#include "../../media/AudioPlayback.h"

namespace padi {

    bool content::Teleport::cast(const std::weak_ptr<Level> &level, const sf::Vector2i &pos) {
        auto lvl = level.lock();
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
        ap->start(lvl);

        lvl->getMap()->removeEntity(m_user);
        auto spawnEvent = std::make_shared<padi::SpawnEvent>(m_user, pos);
        spawnEvent->onCycleBegin(lvl);
        lvl->addCycleEndListener(shared_from_this());
        m_complete = false;
        return true;
    }

    void content::Teleport::castIndicator(const std::weak_ptr<Level> &lvl) {
        auto level = lvl.lock();
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
        if (!m_ghostFX->m_animation) {
            m_ghostFX->m_animation = level->getApollo()->lookupAnim("lightning_hold_end");
            m_ghostFX->m_stackAnimation = level->getApollo()->lookupAnim("lightning_hold");
        }
    }

    void content::Teleport::castCancel(const std::weak_ptr<Level> &lvl) {
        auto level = lvl.lock();
        level->hideCursor();
        level->getMap()->removeEntity(m_ghost);
        level->getMap()->removeEntity(m_ghostFX);
        m_complete = true;
    }

    content::Teleport::Teleport(std::shared_ptr<padi::LivingEntity> user) : Ability(std::move(user)) {
        m_ghost = std::make_shared<StaticEntity>(sf::Vector2i{0, 0});
        m_ghost->m_animation = m_user->getAnimationSet()->at("idle");
        m_ghostFX = std::make_shared<EntityColumn>(sf::Vector2i{0, 0});
        m_ghostFX->m_stackSize = 8;
        m_description = "TELEPORT\n\n  Instantly travel to a location of your choice!";
        m_iconId = "teleport";
    }

    bool content::Teleport::isCastComplete() {
        return m_complete;
    }

    bool content::Teleport::onCycleEnd(std::weak_ptr<padi::Level> const &lvl) {
        m_complete = true;
        return false;
    }

    uint32_t content::Teleport::getAbilityType() const {
        return AbilityType::Teleport;
    }

    bool content::Lighten::cast(const std::weak_ptr<Level> &level, const sf::Vector2i &pos) {
        auto lvl = level.lock();
        lvl->hideCursor();
        strikePos = pos;
        auto strike = std::make_shared<padi::OneshotEntity>(pos);
        strike->m_animation = lvl->getApollo()->lookupAnim("air_strike_large");
        lvl->addCycleEndListener(strike);
        lvl->getMap()->addEntity(strike);
        lvl->getMap()->getTile(pos)->m_walkable = false;
        lvl->addFrameBeginListener(shared_from_this());
        m_complete = false;
        return true;
    }

    void content::Lighten::castIndicator(const std::weak_ptr<Level> &level) {
        level.lock()->showCursor();
    }

    bool content::Lighten::onFrameBegin(std::weak_ptr<padi::Level> const &level, uint8_t frame) {
        auto lvl = level.lock();

        if (frame == 8) {
            auto fire = std::make_shared<padi::StaticEntity>(strikePos);
            fire->m_animation = lvl->getApollo()->lookupAnim("fire");
            lvl->getMap()->addEntity(fire);
        } else if (frame == 11) {
            m_complete = true;
            return false;
        }
        return true;
    }

    void content::Lighten::castCancel(const std::weak_ptr<Level> &level) {
        level.lock()->hideCursor();
        m_complete = true;
    }

    content::Lighten::Lighten(std::shared_ptr<LivingEntity> user) : Ability(std::move(user)) {

        m_iconId = "strike";
    }

    bool content::Lighten::isCastComplete() {
        return m_complete;
    }

    uint32_t content::Lighten::getAbilityType() const {
        return AbilityType::Lighten;
    }

    bool content::Darken::cast(const std::weak_ptr<Level> &level, const sf::Vector2i &pos) {
        auto lvl = level.lock();
        lvl->hideCursor();
        strikePos = pos;
        auto strike = std::make_shared<padi::OneshotEntity>(pos);
        strike->m_animation = lvl->getApollo()->lookupAnim("air_strike_large");
        strike->m_color = sf::Color::Black;
        lvl->addCycleEndListener(strike);
        lvl->getMap()->addEntity(strike);
        lvl->getMap()->getTile(pos)->m_walkable = false;
        lvl->addFrameBeginListener(shared_from_this());
        m_complete = false;
        return true;
    }

    void content::Darken::castIndicator(const std::weak_ptr<Level> &level) {
        level.lock()->showCursor();
    }

    bool content::Darken::onFrameBegin(std::weak_ptr<padi::Level> const &level, uint8_t frame) {
        auto lvl = level.lock();
        auto tile = lvl->getMap()->getTile(strikePos);
        if (frame < 8) {
            auto color = tile->getColor();
            color = sf::Color(std::max(48, color.r - 32), std::max(48, color.g - 32), std::max(48, color.b - 32), 255);
            tile->setColor(color);
            tile->setVerticalOffset(frame % 2);
        } else if (frame == 8) {
            tile->setVerticalOffset(0);
            auto fire = std::make_shared<padi::StaticEntity>(strikePos);
            fire->m_animation = lvl->getApollo()->lookupAnim("fire");
            fire->m_color = sf::Color::Black;
            lvl->getMap()->addEntity(fire);
        } else if (frame == 11) {
            m_complete = true;
            return false;
        }
        return true;
    }

    void content::Darken::castCancel(const std::weak_ptr<Level> &level) {
        level.lock()->hideCursor();
        m_complete = true;
    }

    content::Darken::Darken(std::shared_ptr<padi::LivingEntity> user) : Ability(std::move(user)) {

    }

    bool content::Darken::isCastComplete() {
        return m_complete;
    }

    uint32_t content::Darken::getAbilityType() const {
        return AbilityType::Darken;
    }

    bool content::Walk::cast(const std::weak_ptr<Level> &level, const sf::Vector2i &pos) {
        auto lvl = level.lock();
        LimitedRangeAbility::cast(level, pos);
        lvl->hideCursor();
        if (std::find(m_inRange.begin(), m_inRange.end(), pos) == m_inRange.end()) {
            return false;
        }
        if (!m_path.empty()) {
            m_path.clear();
            return true;
        }
        m_path = padi::FindPath(lvl->getMap(), m_user->getPosition(), pos);
        if (m_path.empty()) {
            return false;
        }
        lvl->addCycleEndListener(shared_from_this());
        m_complete = false;
        return true;
    }

    bool content::Walk::onCycleEnd(std::weak_ptr<padi::Level> const &lvl) {
        if (m_path.empty()) {
            m_complete = true;
            return false;
        } else {
            m_user->intentMove(m_path.front());
            lvl.lock()->addCycleBeginListener(shared_from_this());

            m_path.erase(m_path.begin());
            return true;
        }
    }

    bool content::Walk::onCycleBegin(std::weak_ptr<padi::Level> const &lvl) {
        /* auto ap = std::make_shared<padi::AudioPlayback>(lvl->getApollo()->lookupAudio("chord_01"));
         float pitches[3]{1.0, 1.2, 0.8};
         ap->sound.setPitch(pitches[rand() % 3]);
         ap->sound.setVolume(30);
         lvl->addCycleEndListener(ap);*/
        m_rangeChanged = true;
        return false;
    }

    void content::Walk::castIndicator(const std::weak_ptr<Level> &lvl) {
        auto level = lvl.lock();
        LimitedRangeAbility::castIndicator(lvl);
        level->showCursor();
        auto tile = level->getMap()->getTile(level->getCursorLocation());
        if (!tile || !tile->m_walkable) {
            level->getCursor()->m_color = sf::Color::Red;
        }
    }

    void content::Walk::castCancel(const std::weak_ptr<Level> &level) {
        LimitedRangeAbility::castCancel(level);
        level.lock()->hideCursor();
        m_complete = true;
    }

    content::Walk::Walk(std::shared_ptr<padi::LivingEntity> user, size_t range)
            : padi::LimitedRangeAbility(std::move(user), range) {
        m_description = "WALK\n\n  Travel to a location of your choice - the old-fashioned way.";
        m_iconId = "walk";
    }

    void content::Walk::recalculateRange(const std::weak_ptr<Level> &level) {
        m_shortestPaths = padi::Crawl(level.lock()->getMap(), m_user->getPosition(), getRange());
        m_inRange.clear();
        for (auto &m_shortestPath: m_shortestPaths) {
            m_inRange.emplace_back(m_shortestPath.first);
        }
        m_rangeChanged = false;
    }

    std::vector<sf::Vector2i> const &content::Walk::getPath() const {
        return m_path;
    }

    std::vector<sf::Vector2i> const &content::Walk::getPossibleTargets() const {
        return m_inRange;
    }

    bool content::Walk::isCastComplete() {
        return m_complete;
    }

    uint32_t content::Walk::getAbilityType() const {
        return AbilityType::Walk;
    }

    void content::Walk::writeProperties(uint8_t *data, uint32_t maxSize) {
        if(maxSize > 0) {
            data[0] = uint8_t(getRange());
        }
    }


    content::Dash::Dash(std::shared_ptr<padi::LivingEntity> user, size_t range)
            : padi::LimitedRangeAbility(std::move(user), range) {
        m_iconId = "dash";
    }

    bool content::Dash::cast(const std::weak_ptr<Level> &level, const sf::Vector2i &pos) {
        auto lvl = level.lock();
        auto delta = pos - m_user->getPosition();

        if (delta.x < 0) m_direction = Left;
        else if (delta.x > 0) m_direction = Right;
        else if (delta.y > 0) m_direction = Down;
        else if (delta.y < 0) m_direction = Up;

        if ((m_direction.x == 0 && m_direction.y == 0) || !lvl->getMap()->getTile(pos)->m_walkable) {
            castCancel(lvl);
            return false;
        }
        LimitedRangeAbility::cast(lvl, pos);
        bool x = m_direction.x != 0;
        for (size_t i = 0; i < getRange() - 1; ++i) {
            auto iPos = m_user->getPosition() + m_direction * int(i + 1);
            auto laserPart = std::make_shared<padi::OneshotEntity>(iPos);
            laserPart->m_animation = lvl->getApollo()->lookupAnim(x ? "laser_x_burst" : "laser_y_burst");
            laserPart->m_color = m_user->getColor();
            lvl->getMap()->addEntity(laserPart);
            auto tile = lvl->getMap()->getTile(iPos);
            tile->setColor(tile->getColor() + m_user->getColor());
            lvl->addCycleEndListener(laserPart);
        }
        lvl->getMap()->moveEntity(m_user, pos);
        auto strike = std::make_shared<padi::OneshotEntity>(pos);
        strike->m_animation = lvl->getApollo()->lookupAnim("air_strike_large");
        strike->m_color = m_user->getColor();
        lvl->getMap()->addEntity(strike);
        lvl->addCycleEndListener(strike);
        lvl->centerView(pos);
        m_direction = sf::Vector2i(0, 0);

        lvl->addCycleEndListener(shared_from_this());
        lvl->getCursor()->unlock();
        m_complete = false;
        return true;
    }

    void content::Dash::castIndicator(const std::weak_ptr<Level> &level) {
        auto lvl = level.lock();
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
        LimitedRangeAbility::castIndicator(level);
        lvl->moveCursor(m_user->getPosition() + m_direction * int(getRange()));
        lvl->hideCursor();
        lvl->getCursor()->lock();
    }

    void content::Dash::castCancel(const std::weak_ptr<Level> &lvl) {
        LimitedRangeAbility::castCancel(lvl);
        m_complete = true;
        lvl.lock()->getCursor()->unlock();
    }

    void content::Dash::recalculateRange(const std::weak_ptr<Level> &level) {
        m_inRange.resize(getRange());
        sf::Vector2i min = m_user->getPosition();
        for (int i = 0; i < getRange(); ++i) {
            m_inRange[i] = (min + m_direction * (i + 1));
        }
        m_rangeChanged = false;
    }

    bool content::Dash::isCastComplete() {
        return m_complete;
    }

    bool content::Dash::onCycleEnd(std::weak_ptr<padi::Level> const &lvl) {
        m_complete = true;
        return false;
    }

    uint32_t content::Dash::getAbilityType() const {
        return AbilityType::Dash;
    }

    bool content::Peep::isCastComplete() {
        return false;
    }

    void content::Peep::castCancel(const std::weak_ptr<Level> &level) {
        level.lock()->hideCursor();
    }

    void content::Peep::castIndicator(const std::weak_ptr<Level> &level) {
        level.lock()->showCursor();
    }

    bool content::Peep::cast(const std::weak_ptr<Level> &lvl, const sf::Vector2i &pos) {
        auto level = lvl.lock();
        auto blob = std::make_shared<OneshotEntity>(pos);
        blob->m_animation = level->getApollo()->lookupAnim("air_strike");
        blob->m_color = m_user->getColor();
        level->getMap()->addEntity(blob);
        level->addCycleEndListener(blob);
        auto ping = std::make_shared<AudioPlayback>(level->getApollo()->lookupAudio("attention_ping"));
        auto pingPos = Map::mapTilePosToWorld(pos);
        ping->sound.setPosition(pingPos.x, pingPos.y, 0);
        ping->start(level);
        return false;
    }

    content::Peep::Peep(std::shared_ptr<LivingEntity> user) : Ability(std::move(user)) {
        m_iconId = "view";
        m_description = "LOOK AROUND\n\n  Just look around - cannot be cast.";
    }

    uint32_t content::Peep::getAbilityType() const {
        return AbilityType::Peep;
    }
}