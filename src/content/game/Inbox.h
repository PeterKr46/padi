//
// Created by Peter on 17/06/2022.
//

#pragma once

#include <map>
#include <memory>
#include <queue>
#include "Packets.h"

namespace padi::content {

    class Inbox {
    public:
        explicit Inbox(std::shared_ptr<sf::TcpSocket> socket);

        size_t fetch();

        template<typename Payload>
        bool check(Payload &payload);

        [[nodiscard]] bool has(PayloadType payloadType) const;
        [[nodiscard]] size_t count(PayloadType payloadType) const;

        std::weak_ptr<sf::TcpSocket> getSocket();

    private:
        const std::shared_ptr<std::map<uint8_t, std::queue<std::vector<uint8_t>>>> m_inbox;
        const std::shared_ptr<sf::TcpSocket> m_socket;
    };

} // content
