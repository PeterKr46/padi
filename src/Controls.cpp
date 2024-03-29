//
// Created by Peter on 01/05/2022.
//

#include "Controls.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <cstring>

namespace padi {

    void Controls::keyDown(sf::Keyboard::Key key) {
        s_state[key] = 0;
    }

    void Controls::keyReleased(sf::Keyboard::Key key) {
        s_state[key] = 2;
    }

    void Controls::resetKeyStates() {
        for (auto i = s_state.cbegin(); i != s_state.cend(); /* */  ) {
            if (i->second == 2) i = s_state.erase(i);
            else ++i;
        }
        for (auto &i: s_state) {
            if (i.second == 0) i.second++;
        }
        s_text.clear();
        s_mouse.moved = false;
    }

    bool Controls::wasKeyPressed(sf::Keyboard::Key key) {
        const auto itr = s_state.find(key);
        return !(itr == s_state.cend()) && itr->second == 0;
    }

    bool Controls::isKeyDown(sf::Keyboard::Key key) {
        const auto itr = s_state.find(key);
        return !(itr == s_state.cend()) && itr->second < 2;
    }

    bool Controls::wasKeyReleased(sf::Keyboard::Key key) {
        const auto itr = s_state.find(key);
        return !(itr == s_state.cend()) && itr->second == 2;
    }

    uint8_t Controls::pollKeyState(sf::Keyboard::Key key) {
        const auto itr = s_state.find(key);
        return (itr == s_state.cend()) ? 0 : itr->second + 1;
    }

    std::string const &Controls::textInput() {
        return s_text;
    }

    void Controls::textEntered(uint32_t chr) {
        s_text += chr;
    }

    bool Controls::textInput(std::string &out, size_t max_len, const char *whitelist) {
        if (!whitelist) {
            for (uint32_t chr: s_text) {
                if (chr == '\b') {
                    if (!out.empty()) out.pop_back(); // TODO filter properly
                } else if (chr != '\n' && chr != '\r') {
                    if (out.size() < max_len)
                        out += chr;
                }
            }
        } else {
            for (uint32_t chr: s_text) {
                if (chr == '\b') {
                    if (!out.empty()) out.pop_back();
                } else {
                    if (strchr(whitelist, chr)) {
                        if (out.size() < max_len)
                            out += chr;
                    }
                }
            }
        }
        return !s_text.empty();
    }

    sf::Vector2f Controls::getRelativeMousePosition() {
        return s_mouse.relative;
    }

    sf::Vector2i Controls::getPixelMousePosition() {
        return s_mouse.pixels;
    }

    void Controls::mouseMoved(const sf::Vector2f &rel, sf::Vector2i const& px) {
        s_mouse.relative = rel;
        s_mouse.pixels = px;
        s_mouse.moved = true;
    }

    bool Controls::didMouseMove() {
        return s_mouse.moved;
    }
}
