//
// Created by Peter on 15/06/2022.
//

#include "RemotePlayerTurn.h"
#include <SFML/Network.hpp>
#include <utility>

#include "OnlineGame.h"
#include "../../entity/LivingEntity.h"
#include "../../entity/Ability.h"

namespace padi::content {
    RemotePlayerTurn::RemotePlayerTurn(std::shared_ptr<sf::TcpSocket> socket)
            : m_socket(std::move(socket)) {

    }

    bool RemotePlayerTurn::operator()(const std::shared_ptr<Level> &level, const std::shared_ptr<Character> &chr) {
        // TODO add something to deal with invalid cast attempts :c
        sf::Packet packet;
        PlayerCastPayload payload;
        if(!m_turnStarted) {
            auto status = m_socket->receive(packet);
            if (status == sf::Socket::Done) {
                ReconstructPayload(packet, payload);
                m_turnStarted = true;
                m_casting = payload.ability;
                auto ability = chr->abilities[m_casting];
                ability->castIndicator(level.get());
                chr->entity->intentCast(ability, payload.pos);
                printf("[RemotePlayerTurn] Casting %u at (%i, %i)\n", m_casting, payload.pos.x, payload.pos.y);
            }
        } else {
            if (!chr->entity->hasCastIntent() && chr->abilities[m_casting]->isCastComplete()) {
                m_turnStarted = false;
                return true;
            }
        }
        return false;
    }
} // content