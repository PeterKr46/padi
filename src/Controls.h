//
// Created by Peter on 01/05/2022.
//
#pragma once

#include <map>
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Graphics/RenderWindow.hpp"

namespace padi {


    class Controls {
    public:

        static bool isKeyDown(sf::Keyboard::Key key);
        static bool isKeyUp(sf::Keyboard::Key key);
        static bool isKeyPressed(sf::Keyboard::Key key);

        /**
         * Gets a single byte representing the cached key state
         * @param key SFML Keycode
         * @return 0 if none, 1 if just down, 2 if held, 3 if just released
         */
        static uint8_t pollKeyState(sf::Keyboard::Key key);

        template<class iterator_type>
        static bool isAnyKeyPressed(iterator_type it, iterator_type end) {
            while(it != end) {
                auto itr = s_state.find(*it);
                if(itr != s_state.cend() && itr->second < 2) return true;
                ++it;
            }
            return false;
        }


        static void keyDown(sf::Keyboard::Key key);
        static void keyReleased(sf::Keyboard::Key key);

        static void resetKeyStates();

    protected:
        inline static std::map<sf::Keyboard::Key, uint8_t> s_state;
    };
}
