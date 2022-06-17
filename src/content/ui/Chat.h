//
// Created by Peter on 17/06/2022.
//

#pragma once

#include <functional>
#include "../../ui/UIContext.h"

namespace padi::content {

    class Chat {
    public:
        explicit Chat(sf::FloatRect const &bounds);

        void init(padi::UIContext *context);

        void draw(padi::UIContext *context);

        void setHidden(padi::UIContext *context, bool hidden);

        void write(padi::UIContext *context, std::string const& msg);

        std::function<void(std::string const &msg)> submit = nullptr;

    private:
        bool m_hidden{false};
        const sf::FloatRect m_bounds{0, 0, 208, 60};
        std::string m_inputBuf{};
        std::string m_logBuf{};
        uint8_t     m_lineLength{8};
    };

} // content
