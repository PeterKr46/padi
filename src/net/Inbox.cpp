//
// Created by Peter on 17/06/2022.
//

#include "Inbox.h"
#include "SFML/Network.hpp"

#include <utility>

namespace padi::content {
    Inbox::Inbox(std::shared_ptr<sf::TcpSocket> socket)
            : m_socket(std::move(socket)),
              m_inbox(std::make_shared<std::map<uint8_t, std::queue<std::vector<uint8_t>>>>()) {
    }

    size_t Inbox::fetch() {
        if(!m_socket) return -1;

        if(m_socket->isBlocking())
            m_socket->setBlocking(false);
        size_t received = 0;
        {
            sf::Packet incoming;
            sf::Socket::Status status = m_socket->receive(incoming);
            while (status == sf::Socket::Done) {
                ++received;
                auto data = reinterpret_cast<const uint8_t *>(incoming.getData());
                auto &queue = (*m_inbox)[data[0]];
                queue.emplace(data, data + incoming.getDataSize());
                status = m_socket->receive(incoming);
            }
            if (status == sf::Socket::Disconnected) {
                printf("[Inbox] Connection to %s lost.\n", m_socket->getRemoteAddress().toString().c_str());
                return -1;
            }
        }
        return received;
    }

    bool Inbox::has(PayloadType payloadType) const {
        if(!m_inbox) return false;

        auto queueIter = m_inbox->find(payloadType);
        if (queueIter != m_inbox->end()) {
            return !queueIter->second.empty();
        }
        return false;
    }

    std::weak_ptr<sf::TcpSocket> Inbox::getSocket() {
        return m_socket;
    }

    size_t Inbox::count(PayloadType payloadType) const {
        if(!m_inbox) return -1;

        auto queueIter = m_inbox->find(payloadType);
        if (queueIter != m_inbox->end()) {
            return queueIter->second.size();
        }
        return 0;
    }

    Inbox::operator bool() {
        return bool(m_socket);
    }

} // content