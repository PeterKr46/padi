//
// Created by Peter on 15/06/2022.
//

#include "RemotePlayerTurn.h"

#include <SFML/Network.hpp>
#include <utility>

#include "Character.h"
#include "../../net/Packets.h"
#include "OnlineGame.h"
#include "../../entity/LivingEntity.h"
#include "../../entity/Ability.h"
#include "../../level/Level.h"
#include "../menu/MainMenu.h"

namespace padi::content {

    enum RemoteTurnState : int {
        IDLE = 0,
        CASTING = 1,
        DONE = 2
    };

    RemotePlayerTurn::RemotePlayerTurn(InOutBox socket, bool reflect, uint32_t ignore)
            : m_socket(std::move(socket)), m_reflect(reflect), m_ignore(ignore) {

    }

    bool RemotePlayerTurn::operator()(const std::shared_ptr<OnlineGame> &game, const std::shared_ptr<Character> &chr) {
        auto level = game->getLevel().lock();
        CharacterCastPayload payload;
        RemoteTurnState state = IDLE;
        if(!chr->entity) {
            printf("Non-Player turn.");
            return true;
        }
        if (m_activeAbility != -1) {
            state = CASTING;
            if (!chr->entity->hasCastIntent()) {
                if (!chr->entity->hasFailedCast() &&
                    chr->abilities[m_activeAbility]->isCastComplete()) {
                    state = DONE;
                }
            }
        }
        if (state == IDLE) {
            if (m_socket.fetch(payload)) {
                m_activeAbility = int8_t(payload.ability);
                auto ability = chr->abilities[m_activeAbility];
                level->centerView(payload.pos);
                ability->castIndicator(level);
                chr->entity->intentCast(ability, payload.pos);
                printf("[RemotePlayerTurn] Casting %hhd (%i) at (%i, %i)\n", m_activeAbility, chr->abilities[m_activeAbility]->getAbilityType(), payload.pos.x,
                       payload.pos.y);
                if(m_reflect) {
                    printf("[RemotePlayerTurn] Reflecting to all but %i.\n", m_ignore);
                    auto packet = PackagePayload(payload);
                    game->broadcast(packet, &m_ignore, 1);
                }
            }
        } else if (state == CASTING) {
            if (!chr->entity->hasCastIntent() && chr->entity->hasFailedCast()) {
                m_activeAbility = -1;
                printf("[RemotePlayerTurn] Miscast - waiting.\n");
            }
        }
        else /* if (state == DONE) */ {
            auto ability = chr->abilities[m_activeAbility];
            if (ability->numUses > 0) {
                ability->numUses--;
            }
            if (ability->numUses == 0) {
                chr->abilities.erase(std::remove(chr->abilities.begin(), chr->abilities.end(), ability));
                printf("[RemotePlayerTurn] Used up ability %i.\n", m_activeAbility);
            }
            m_activeAbility = -1;
        }
        if(chr->entity->hasHPBar() && chr->entity->getHPBar().lock()->getHP() == 0) {
            chr->alive = false;
        }
        return state == DONE;
    }
} // content