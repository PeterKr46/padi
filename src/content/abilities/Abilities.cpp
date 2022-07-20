//
// Created by Peter on 06/05/2022.
//

#include "Abilities.h"

#include <utility>
#include <cstring>
#include <cmath>
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
        ose->dispatchImmediate(level);

        auto ap = std::make_shared<padi::AudioPlayback>(lvl->getApollo()->lookupAudio("chord_01"));
        ap->sound.setPitch((abs(pos.x) + abs(pos.y)) % 2 == 0 ? 1.2 : 1.0);
        ap->start(lvl);

        lvl->getMap()->removeEntity(m_user);
        auto spawnEvent = std::make_shared<padi::SpawnEvent>(m_user, pos, true);
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
        m_description = "Instantly teleport to a location of your choice.";
        m_iconId = "teleport";
        numUses = 1;
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
        if (LimitedRangeAbility::cast(level, pos)) {
            auto lvl = level.lock();
            lvl->hideCursor();
            strikePos = pos;
            auto strike = std::make_shared<padi::OneshotEntity>(pos);
            strike->m_animation = lvl->getApollo()->lookupAnim("air_strike_large");
            strike->dispatchImmediate(level);
            auto fire = std::make_shared<padi::OneshotEntity>(strikePos);
            fire->m_animation = lvl->getApollo()->lookupAnim("fire");
            fire->dispatchImmediate(level);
            lvl->addFrameBeginListener(shared_from_this());
            m_complete = false;
            return true;
        }
        return false;
    }

    void content::Lighten::castIndicator(const std::weak_ptr<Level> &level) {
        LimitedRangeAbility::castIndicator(level);
        m_rangeChanged = true;
        level.lock()->showCursor();
    }

    bool content::Lighten::onFrameBegin(std::weak_ptr<padi::Level> const &level, uint8_t frame) {
        auto lvl = level.lock();
        auto tile = lvl->getMap()->getTile(strikePos);
        if (frame == 4) {
            std::vector<std::shared_ptr<Entity>> ents;
            if (lvl->getMap()->getEntities(strikePos, ents)) {
                for (auto &entity: ents) {
                    if (entity->getType() & LIVING) {
                        auto livingEntity = std::static_pointer_cast<LivingEntity>(entity);
                        if (livingEntity->hasHPBar()) {
                            auto hpBar = livingEntity->getHPBar().lock();
                            auto hp = hpBar->getHP();
                            if(livingEntity->isDark()) {
                                hpBar->setHP(--hp);
                                if (hp == 0) m_user->enemiesSlain++;
                            } else if(livingEntity->isLight()) {
                                hpBar->setHP(++hp);
                                /*livingEntity->trySetAnimation("spawn");
                                auto apolloCtx = livingEntity->getAnimationSet();
                                if (apolloCtx->has("spawn_ray")) {
                                    auto ray = std::make_shared<OneshotEntityStack>(strikePos);
                                    ray->m_color = livingEntity->getColor();
                                    ray->m_animation = apolloCtx->at("spawn_ray");
                                    ray->m_stackSize = 16;
                                    ray->m_verticalStep = -float(padi::TileSize.y);
                                    ray->dispatchImmediate(lvl);
                                }*/
                                auto heal = std::make_shared<OneshotEntity>(strikePos);
                                heal->setVerticalOffset(-16);
                                heal->m_color = livingEntity->getColor();
                                heal->m_animation = lvl->getApollo()->lookupAnim("heal");
                                heal->dispatchImmediate(lvl);
                            }
                        }
                    }
                }
            }
        } else if (frame == 11) {
            m_complete = true;
            return false;
        }
        auto col = tile->getColor();
        uint16_t cSum = col.r + col.g + col.b;
        if (cSum > 100) tile->lerpAdditiveColor(sf::Color::White, 0.2);
        return true;
    }

    void content::Lighten::castCancel(const std::weak_ptr<Level> &level) {
        LimitedRangeAbility::castCancel(level);
        level.lock()->hideCursor();
        m_complete = true;
    }

    content::Lighten::Lighten(std::shared_ptr<LivingEntity> user) : LimitedRangeAbility(std::move(user), 8) {
        m_description = "Bless an UNCURSED Tile.\n"
                        " Heals LIGHT, Damages DARK Occupants.";
        m_iconId = "strike";
        numUses = 3;
    }

    bool content::Lighten::isCastComplete() {
        return m_complete;
    }

    uint32_t content::Lighten::getAbilityType() const {
        return AbilityType::Lighten;
    }

    void content::Lighten::recalculateRange(const std::weak_ptr<Level> &level) {
        LimitedRangeAbility::recalculateRange(level);

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
        auto col = tile->getColor();
        uint16_t cSum = col.r + col.g + col.b;
        if (frame < 8) {
            if (cSum < 700) {
                tile->lerpColor(sf::Color::Black, 0.2);
                tile->setVerticalOffset(frame % 2);
            }
        } else if (frame == 8) {
            if(cSum < 700) tile->setVerticalOffset(0);
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
            printf("[Walk] Target not in range.\n");
            return false;
        }
        if (!m_path.empty()) {
            m_path.clear();
            return true;
        }
        m_path = padi::FindPath(lvl->getMap(), m_user->getPosition(), pos, walkable);
        if (m_path.empty()) {
            printf("[Walk] Failed to find path.\n");
            return false;
        }
        lvl->addCycleEndListener(shared_from_this());
        lvl->addFrameBeginListener(shared_from_this());
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
        m_path.clear();
    }

    content::Walk::Walk(std::shared_ptr<padi::LivingEntity> user, size_t range, Walkable w)
            : padi::LimitedRangeAbility(std::move(user), range) {
        m_description = "Walk a short distance, lighting the way.\n"
                        "Can't cross CURSED tiles, but may BLESS.";
        m_iconId = "walk";
        walkable = w;
    }

    void content::Walk::recalculateRange(const std::weak_ptr<Level> &level) {
        m_inRange = padi::Crawl(level.lock()->getMap(), m_user->getPosition(), getRange(), walkable);
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
        if (maxSize >= sizeof(uint16_t) + sizeof(uint8_t)) {
            data[0] = uint8_t(getRange());
            std::memcpy(data + 1, &walkable.cutOff, sizeof(int16_t));
        }
    }

    bool content::Walk::onFrameBegin(const std::weak_ptr<padi::Level> &lvl, uint8_t frame) {
        if(m_user->isLight()) {
            auto level = lvl.lock();
            auto target = m_user->getPosition();
            if (m_user->isMoving() && frame >= CycleLength_F / 2) {
                target += m_user->currentMoveDirection();
            }
            auto tile = level->getMap()->getTile(target);

            tile->lerpAdditiveColor(m_user->getColor(), 0.3);
        }
        return !Walk::isCastComplete();
    }


    content::Dash::Dash(std::shared_ptr<padi::LivingEntity> user, size_t range, Walk::Walkable walkable)
            : padi::LimitedRangeAbility(std::move(user), range)
            , m_walkable(walkable) {
        m_iconId = "dash";
        m_description = "Dash to a walkable position.\n"
                        "Deals damage and lights UNCURSED tiles.";
    }

    bool content::Dash::cast(const std::weak_ptr<Level> &level, const sf::Vector2i &pos) {
        auto lvl = level.lock();
        auto delta = pos - m_user->getPosition();
        m_direction = sf::Vector2i (0,0);
        for(auto & dir : AllDirections) {
            if(delta == dir * int(getRange())) {
                m_direction = dir;
                break;
            }
        }

        if ((m_direction.x == 0 && m_direction.y == 0) || !lvl->getMap()->getTile(pos)->m_walkable) {
            castCancel(lvl);
            return false;
        }
        if(LimitedRangeAbility::cast(lvl, pos)) {
            auto anim = lvl->getApollo()->lookupAnim(m_direction.x != 0 ? "laser_x_burst" : "laser_y_burst");
            for (size_t i = 0; i < getRange() - 1; ++i) {
                auto iPos = m_user->getPosition() + m_direction * int(i + 1);
                auto laserPart = std::make_shared<padi::OneshotEntity>(iPos);
                laserPart->m_animation = anim;
                laserPart->m_color = m_user->getColor();
                lvl->getMap()->addEntity(laserPart);
                auto tile = lvl->getMap()->getTile(iPos);
                if (tile) {
                    auto col = tile->getColor();
                    uint16_t cSum = col.r + col.g + col.b;
                    if (cSum > 100) tile->lerpAdditiveColor(m_user->getColor(), 0.9);
                }
                lvl->addCycleEndListener(laserPart);
                std::vector<std::shared_ptr<Entity>> ents;
                if (lvl->getMap()->getEntities(iPos, ents)) {
                    for (auto &entity: ents) {
                        if (entity->getType() & LIVING) {
                            auto livingEntity = std::static_pointer_cast<LivingEntity>(entity);
                            if (livingEntity->hasHPBar()) {
                                auto hpBar = livingEntity->getHPBar().lock();
                                auto hp = hpBar->getHP();
                                hpBar->setHP(hp - 1);
                                if (hp == 1) m_user->enemiesSlain++;
                            }
                        }
                    }
                }
            }
            auto strike = std::make_shared<padi::OneshotEntity>(m_user->getPosition());
            strike->m_animation = lvl->getApollo()->lookupAnim("bubble");
            strike->m_color = m_user->getColor();
            lvl->getMap()->addEntity(strike);
            lvl->addCycleEndListener(strike);
            lvl->centerView(pos);
            lvl->getMap()->moveEntity(m_user, pos);

            strike = std::make_shared<padi::OneshotEntity>(pos);
            strike->m_animation = lvl->getApollo()->lookupAnim("air_strike_large");
            strike->m_color = m_user->getColor();
            lvl->getMap()->addEntity(strike);
            lvl->addCycleEndListener(strike);
            lvl->centerView(pos);
            m_direction = sf::Vector2i(0, 0);

            lvl->addCycleEndListener(shared_from_this());
            lvl->getCursor()->unlock();
            m_complete = false;

            auto ap = std::make_shared<AudioPlayback>(lvl->getApollo()->lookupAudio("zap"));
            ap->start(level);
            return true;
        }
        return false;
    }

    void content::Dash::castIndicator(const std::weak_ptr<Level> &level) {
        auto lvl = level.lock();
        if (padi::Controls::wasKeyPressed(sf::Keyboard::Up)) {
            m_direction = Up;
            m_rangeChanged = true;
            lvl->getMap()->moveEntity(lvl->getCursor(), m_user->getPosition() + m_direction * int(getRange()));
        } else if (padi::Controls::wasKeyPressed(sf::Keyboard::Down)) {
            m_direction = Down;
            m_rangeChanged = true;
            lvl->getMap()->moveEntity(lvl->getCursor(), m_user->getPosition() + m_direction * int(getRange()));
        } else if (padi::Controls::wasKeyPressed(sf::Keyboard::Left)) {
            m_direction = Left;
            m_rangeChanged = true;
            lvl->getMap()->moveEntity(lvl->getCursor(), m_user->getPosition() + m_direction * int(getRange()));
        } else if (padi::Controls::wasKeyPressed(sf::Keyboard::Right)) {
            m_direction = Right;
            lvl->getMap()->moveEntity(lvl->getCursor(), m_user->getPosition() + m_direction * int(getRange()));
            m_rangeChanged = true;
        }
        m_rangeChanged = true;
        LimitedRangeAbility::castIndicator(level);
        lvl->hideCursor();
        lvl->getCursor()->lock();
    }

    void content::Dash::castCancel(const std::weak_ptr<Level> &lvl) {
        LimitedRangeAbility::castCancel(lvl);
        m_complete = true;
        lvl.lock()->getCursor()->unlock();
        m_direction = sf::Vector2i {0,0};
    }

    void content::Dash::recalculateRange(const std::weak_ptr<Level> &l) {
        auto level = l.lock();
        auto map = level->getMap();
        if(m_direction == sf::Vector2i(0,0)) {
            m_inRange.clear();
            for (auto &dir: AllDirections) {
                auto tile = map->getTile(m_user->getPosition() + dir * int(getRange()));
                if (m_walkable(map, tile)) {
                    for (int i = 0; i < getRange(); ++i) {
                        m_inRange.emplace_back(m_user->getPosition() + dir * int(i+1));
                    }
                }
            }
        } else {
            m_inRange.resize(getRange());
            sf::Vector2i min = m_user->getPosition();
            for (int i = 0; i < getRange(); ++i) {
                m_inRange[i] = (min + m_direction * (i + 1));
            }
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

    void content::Dash::writeProperties(uint8_t *data, uint32_t maxSize) {
        if (maxSize >= sizeof(uint16_t) + sizeof(uint8_t)) {
            data[0] = uint8_t(getRange());
            std::memcpy(data + 1, &m_walkable.cutOff, sizeof(int16_t));
        }
    }

    bool content::Peep::isCastComplete() {
        return true;
    }

    void content::Peep::castCancel(const std::weak_ptr<Level> &level) {
        level.lock()->hideCursor();
        level.lock()->getMap()->removeEntity(m_infoEntity);
    }

    void content::Peep::castIndicator(const std::weak_ptr<Level> &lvl) {
        auto level = lvl.lock();
        auto map = level->getMap();
        auto cursor = level->getCursorLocation();
        level->showCursor();
        std::shared_ptr<LivingEntity> e = std::static_pointer_cast<LivingEntity>(map->getEntity(cursor, 8));
        if (e) {
            //m_infoEntity->m_animation = level->getApollo()->lookupAnim("debug");
            if(m_infoEntity->m_animation) {
                map->moveEntity(m_infoEntity, cursor);
            }
        } else {
            map->removeEntity(m_infoEntity);
        }
    }

    bool content::Peep::cast(const std::weak_ptr<Level> &lvl, const sf::Vector2i &pos) {
        auto level = lvl.lock();
        auto map = level->getMap();
        auto blob = std::make_shared<OneshotEntity>(pos);
        blob->m_animation = level->getApollo()->lookupAnim("air_strike");
        blob->m_color = m_user->getColor();
        level->getMap()->addEntity(blob);
        level->addCycleEndListener(blob);
        auto ping = std::make_shared<AudioPlayback>(level->getApollo()->lookupAudio("attention_ping"));
        ping->setPosition(pos);
        ping->start(level);

        map->removeEntity(m_infoEntity);

        std::vector<std::shared_ptr<padi::Entity>> entities;
        return false;
    }

    content::Peep::Peep(std::shared_ptr<LivingEntity> user) : Ability(std::move(user)) {
        m_iconId = "view";
        m_description = "Just look around.\n"
                        "Can be cast to ping other players.";
        m_infoEntity = std::make_shared<StaticEntity>(sf::Vector2i {0,0});
        m_infoEntity->setVerticalOffset(-32);
    }

    uint32_t content::Peep::getAbilityType() const {
        return AbilityType::Peep;
    }

    bool content::Walk::Walkable::operator()(const Map *map, const std::shared_ptr<Tile> &t) {
        if (t && t->m_walkable && (!map->hasEntities(t->getPosition(), LIVING) || (cutOff > 0 && map->hasEntities(t->getPosition(), BEACON)))) {
            auto col = t->getColor();
            if (cutOff < 0) {
                return col.r + col.g + col.b < -cutOff;
            }
            return col.r + col.g + col.b > cutOff;
        }
        return false;
    }

    bool content::Raze::cast(const std::weak_ptr<Level> &l, const sf::Vector2i &pos) {
        if (LimitedRangeAbility::cast(l, pos)) {
            auto lvl = l.lock();
            auto map = lvl->getMap();
            auto tile = map->getTile(pos);
            if (!tile || tile->m_walkable) {
                return false;
            }
            m_razePos = tile;
            for(auto const& dir : AllDirections) {
                tile = map->getTile(pos + dir);
                if (tile && !map->hasEntities(pos+dir, BEACON)) {
                    m_adjacent.push_back(tile);
                }
            }
            if(m_user->trySetAnimation("raze")) {
                map->moveEntity(m_user, pos);
                m_user->setVerticalOffset(-12);
                // TODO maybe sound?
                lvl->addFrameEndListener(shared_from_this());
            }
            return true;
        }
        return false;
    }

    content::Raze::Raze(std::shared_ptr<padi::LivingEntity> user)
            : LimitedRangeAbility(std::move(user), 1) {
        m_iconId = "raze";
        m_description = "Destroy a nearby obstacle,\n"
                        "then take its place.";
    }

    uint32_t content::Raze::getAbilityType() const {
        return AbilityType::Raze;
    }

    bool content::Raze::onFrameEnd(const std::weak_ptr<padi::Level> &l, uint8_t frame) {
        m_razePos->lerpColor(m_user->getColor(), 0.6);
        auto map = l.lock()->getMap();
        if(frame >= 3 && frame <= 8) {
            float height = -std::sin(float(frame - 3) * 3.141f / 5.f) * 4;
            m_razePos->setVerticalOffset(-height);
            for(auto & adj : m_adjacent) {
                adj->setVerticalOffset(height);
            }
        }
        if (frame == 5) {
            m_razePos->m_walkable = true;
            m_razePos->setDecoration(nullptr);
            m_razePos->setColor(m_user->getColor());
            for(auto & adj : m_adjacent) {
                auto livingEntity = std::static_pointer_cast<LivingEntity>(map->getEntity(adj->getPosition(), EntityType::LIVING));
                if (livingEntity && livingEntity->isLight() != m_user->isLight() && livingEntity->hasHPBar()) {
                    auto hpbar = livingEntity->getHPBar().lock();
                    hpbar->setHP(hpbar->getHP() - 1);
                }
            }
        }
        if(frame == 11) {
            m_user->setVerticalOffset(0);
            m_razePos.reset();
            m_adjacent.clear();
        }
        return frame != 11;
    }

    bool content::Raze::isCastComplete() {
        return !m_razePos;
    }

    void content::Raze::castIndicator(const std::weak_ptr<Level> &level) {
        level.lock()->showCursor();
        LimitedRangeAbility::castIndicator(level);
    }

    void content::Raze::recalculateRange(const std::weak_ptr<Level> &l) {
        auto level = l.lock();
        auto map = level->getMap();
        LimitedRangeAbility::recalculateRange(l);
        auto iter = m_inRange.begin();
        while (iter != m_inRange.end()) {
            auto tile = map->getTile(*iter);
            if(!tile || tile->m_walkable) {
                iter = m_inRange.erase(iter);
            } else {
                ++iter;
            }
        }
    }

    content::Wildfire::Wildfire(std::shared_ptr<LivingEntity> user, int range)
        : LimitedRangeAbility(std::move(user), range) {
        m_description = "Sets multiple tiles ALIGHT in your color.\n"
                        "Will affect CURSED tiles.";
        m_iconId = "wildfire";
        numUses = 1;
    }

    bool content::Wildfire::cast(const std::weak_ptr<Level> &lvl, const sf::Vector2i &pos) {
        auto level = lvl.lock();
        if (LimitedRangeAbility::cast(lvl, pos)) {
            auto map = level->getMap();
            for (auto &n: AllDirections) {
                auto tile = map->getTile(pos + n);
                if (tile && !map->hasEntities(pos + n, EntityType::BEACON)) {
                    auto fire = std::make_shared<OneshotEntity>(pos + n);
                    fire->m_animation = level->getApollo()->lookupAnim("air_strike");
                    fire->m_color = m_user->getColor();
                    fire->dispatchImmediate(level);
                }
            }
            auto airstrike = std::make_shared<OneshotEntity>(pos);
            airstrike->m_animation = level->getApollo()->lookupAnim("air_strike_large");
            airstrike->m_color = m_user->getColor();
            airstrike->dispatchImmediate(level);

            level->addFrameBeginListener(shared_from_this());
            m_strikePos = pos;
            m_complete = false;
            m_cycles = 0;
            return true;
        }
        return false;
    }

    void content::Wildfire::castCancel(const std::weak_ptr<Level> &level) {
        LimitedRangeAbility::castCancel(level);
        level.lock()->hideCursor();
        m_complete = true;
    }

    void content::Wildfire::castIndicator(const std::weak_ptr<Level> &level) {
        LimitedRangeAbility::castIndicator(level);
        level.lock()->showCursor();
    }

    bool content::Wildfire::onFrameBegin(const std::weak_ptr<padi::Level> &lvl, uint8_t frame) {
        auto level = lvl.lock();
        auto map = level->getMap();
        auto pos = m_strikePos;
        if(frame < 8 && !m_cycles) {
            for (auto &dir: Neighborhood) {
                auto tile = map->getTile(pos + dir);
                if (tile) {
                    tile->lerpAdditiveColor(m_user->getColor(), 0.1);
                    tile->setVerticalOffset(float(frame % 2));
                }
            }
        } else if (frame == 8 && !m_cycles) {
            auto fireAnim = level->getApollo()->lookupAnim("fire");
            for (auto &dir: Neighborhood) {
                auto tile = map->getTile(pos + dir);
                if (tile) {
                    tile->setVerticalOffset(0);
                    auto fire = std::make_shared<padi::OneshotEntity>(pos + dir);
                    fire->m_animation = fireAnim;
                    fire->m_color = m_user->getColor();
                    fire->dispatch(level);
                    auto ent = std::static_pointer_cast<LivingEntity>(map->getEntity(pos + dir, LIVING));
                    if(ent) {
                        if(ent->isDark() && ent->hasHPBar()) {
                            auto hp = ent->getHPBar().lock();
                            if(hp->getHP() > 0) {
                                hp->setHP(hp->getHP() - 2);
                                if(!hp->getHP()) m_user->enemiesSlain++;
                            }
                        }
                    }
                }
            }
            if (m_user->hasHPBar() && m_user->getHPBar().lock()->getHP() == 0) {
                map->removeEntity(m_user);
            }
        } else if (frame == 11) {
            m_complete = (++m_cycles == 2);
            return !m_complete;
        }
        return true;
    }

    bool content::Wildfire::isCastComplete() {
        return m_complete;
    }

    uint32_t content::Wildfire::getAbilityType() const {
        return AbilityType::Wildfire;
    }

    void content::Idle::castIndicator(const std::weak_ptr<Level> &level) {

    }

    bool content::Idle::isCastComplete() {
        return true;
    }

    uint32_t content::Idle::getAbilityType() const {
        return AbilityType::Idle;
    }

    bool content::Idle::cast(const std::weak_ptr<Level> &level, const sf::Vector2i &pos) {
        return true;
    }

    void content::Idle::castCancel(const std::weak_ptr<Level> &level) {

    }

    content::Idle::Idle(std::shared_ptr<LivingEntity> user) : Ability(std::move(user)) {

    }
}