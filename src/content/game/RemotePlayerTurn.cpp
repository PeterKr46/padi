//
// Created by Peter on 15/06/2022.
//

#include "RemotePlayerTurn.h"

#include <SFML/Network.hpp>
#include <utility>

#include "Character.h"
#include "Packets.h"
#include "../../entity/LivingEntity.h"
#include "../../entity/Ability.h"
#include "../../level/Level.h"

namespace padi::content {

    enum RemoteTurnState : int {
        IDLE = 0,
        CASTING = 1,
        DONE = 2
    };

    RemotePlayerTurn::RemotePlayerTurn(std::shared_ptr<sf::TcpSocket> socket)
            : m_socket(std::move(socket)) {

    }

    bool RemotePlayerTurn::operator()(const std::shared_ptr<Level> &level, const std::shared_ptr<Character> &chr) {
        sf::Packet packet;
        PlayerCastPayload payload;
        RemoteTurnState state = IDLE;
        if (m_activeAbility != -1) {
            state = CASTING;
            if (!chr->entity->hasCastIntent()) {
                if (!chr->entity->hasFailedCast() &&
                        chr->abilities[m_activeAbility]->isCastComplete()) {
                    state = DONE;
                }
            }
        }
        if(state == IDLE) {
            auto status = m_socket->receive(packet);
            if (status == sf::Socket::Done) {
                ReconstructPayload(packet, payload);
                m_activeAbility = int8_t(payload.ability);
                auto ability = chr->abilities[m_activeAbility];
                level->centerView(payload.pos);
                ability->castIndicator(level.get());
                chr->entity->intentCast(ability, payload.pos);
                printf("[RemotePlayerTurn] Casting %u at (%i, %i)\n", m_activeAbility, payload.pos.x, payload.pos.y);
            }
        }
        else if(state == CASTING) {
            if(!chr->entity->hasCastIntent() && chr->entity->hasFailedCast()) {
                m_activeAbility = -1;
                printf("[RemotePlayerTurn] Miscast - waiting.\n");
            }
        }
        if (state == DONE) {
            m_activeAbility = -1;
        }
        return state == DONE;
    }
} // content