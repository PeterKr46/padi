//
// Created by Peter on 22/06/2022.
//

#include "SlugMob.h"

#include <utility>
#include "../game/OnlineGame.h"

namespace padi::content {
    SlugMob::SlugMob(std::string name, const padi::AnimationSet *moveset, const sf::Vector2i &pos)
            : LivingEntity(std::move(name), moveset, pos) {
        setColor(sf::Color(32, 32, 32));
    }

    bool SlugMob::takeTurn(const std::shared_ptr<OnlineGame> &g, const std::shared_ptr<Character> &chr) {

        auto game = std::static_pointer_cast<HostGame>(g);
        auto level = game->getLevel().lock();
        if(!m_rand) {
            m_rand = std::make_shared<std::mt19937>(game->getSeed() + chr->id * 7341);
        }

        auto walk = std::static_pointer_cast<SlugWalk>(chr->abilities[0]);
        if (!m_turnStarted) {
            walk->castIndicator(level);
            walk->recalculateRange(level);
            std::vector<sf::Vector2i> targets = walk->getPossibleTargets();
            if (targets.empty()) {
                walk->castCancel(level);
                return true;
            }
            auto map = level->getMap();
            auto target = std::max_element(targets.begin(), targets.end(),
                                           [map](const sf::Vector2i &a, const sf::Vector2i &b) {
                                               auto colA = map->getTile(a)->getColor();
                                               auto colB = map->getTile(b)->getColor();
                return colA.r + colA.b + colA.g < colB.r + colA.b + colB.g;
                        });
            chr->entity->intentCast(walk, *target);
            {
                sf::Packet packet;
                CharacterCastPayload payload;
                payload.ability = uint8_t(0);
                payload.pos = *target;
                printf("[SlugMob] Walking to (%i, %i)\n", payload.pos.x, payload.pos.y);
                packet.append(&payload, sizeof(payload));
                game->broadcast(packet);
            }
            m_turnStarted = true;
        }
        if (chr->entity) {
            if (!chr->entity->hasCastIntent() && chr->abilities[0]->isCastComplete()) {
                m_turnStarted = false;
            }
            return !chr->entity->hasCastIntent() && chr->abilities[0]->isCastComplete();
        } else {
            return true;
        }
    }

    Character SlugMob::asCharacter(uint32_t id) {
        return Character{id,
                         shared_from_this(),
                         {
                                 std::make_shared<SlugWalk>(shared_from_this(), 2, Walk::Walkable{-600}),
                         },
                         [=](const std::shared_ptr<OnlineGame> &l, const std::shared_ptr<Character> &c) {
                             return takeTurn(l, c);
                         }
        };

    }

    SlugWalk::SlugWalk(std::shared_ptr<padi::LivingEntity> user, size_t range, Walk::Walkable walkable) : Walk(std::move(user),
                                                                                                               range,
                                                                                                               walkable) {

    }

    bool SlugWalk::cast(const std::weak_ptr<Level> &lvl, const sf::Vector2i &pos) {
        bool walked = Walk::cast(lvl, pos);
        if(walked) {
            lvl.lock()->addFrameBeginListener(shared_from_this());
        }
        return walked;
    }


    uint32_t SlugWalk::getAbilityType() const {
        return AbilityType::SlugWalk;
    }

    bool SlugWalk::onFrameBegin(const std::weak_ptr<padi::Level> &lvl, uint8_t frame) {
        auto level = lvl.lock();
        auto target = m_user->getPosition();
        if(m_user->isMoving() && frame >= CycleLength_F / 2) {
            target += m_user->currentMoveDirection();
        }
        auto tile = level->getMap()->getTile(target);

        tile->lerpColor(sf::Color(0x1e1e1eff), 0.5);
        return !Walk::isCastComplete();
    }

} // content