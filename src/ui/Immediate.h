//
// Created by Peter on 09/05/2022.
//

#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <string>

#include "../media/Animation.h"

namespace padi {
    class UIContext;

    class Immediate {
    public:
        static bool Button(padi::UIContext *ctx, std::string const &label, sf::FloatRect const &size);

        static bool Switch(padi::UIContext *ctx, std::string const &label, sf::FloatRect const &size, bool *ptr = nullptr);

        static void Sprite(padi::UIContext *ctx, sf::FloatRect const &size, size_t frame, const std::shared_ptr<Animation> &anim, sf::Color const& color = sf::Color::White);

        static bool checkFocusSwitch(padi::UIContext *ctx, size_t id);

        static bool isFocused(padi::UIContext *ctx, std::string const &id);

    protected:
        static size_t
        drawScalable(padi::UIContext *ctx, const std::shared_ptr<Animation> &anim, const sf::FloatRect &bound,
                     uint8_t frame, sf::Color);

        static size_t
        draw(UIContext *ctx, const std::shared_ptr<Animation> &anim, const sf::FloatRect &bound, uint8_t frame,
             sf::Color color);
    };
} // padi
