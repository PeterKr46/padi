//
// Created by Peter on 17/06/2022.
//

#pragma once

#include <map>
#include <memory>
#include <queue>
#include "Packets.h"

namespace padi::content {

    class InOutBox {
    public:
        InOutBox() = default;
        explicit InOutBox(std::shared_ptr<sf::TcpSocket> socket);

        size_t receive();

        void send(sf::Packet& packet);

        template<typename Payload>
        bool fetch(Payload &payload) {
            auto queueIter = m_inbox->find(payload.type);
            if (queueIter != m_inbox->end()) {
                std::queue<std::vector<uint8_t>> &queue = queueIter->second;
                auto &data = queue.front();
                std::memcpy(&payload, data.data(), data.size());
                queue.pop();
                if (queue.empty()) {
                    m_inbox->erase(queueIter);
                }
                return true;
            }
            return false;
        }

        template<typename Payload>
        bool fetchBlocking(Payload & payload) {
            while(!fetch(payload)) {
                if(receive() == -1) {
                    printf("[InOutBox] Lost connection.\n");
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]] bool has(PayloadType payloadType) const;

        [[nodiscard]] size_t count(PayloadType payloadType) const;

        std::weak_ptr<sf::TcpSocket> getSocket();

        explicit operator bool();

    private:
        std::shared_ptr<std::map<uint8_t, std::queue<std::vector<uint8_t>>>> m_inbox{nullptr};
        std::shared_ptr<sf::TcpSocket> m_socket{nullptr};
    };

} // content
