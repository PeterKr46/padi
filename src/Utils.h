//
// Created by Peter on 01/07/2022.
//

#pragma once

#include <cmath>
#include <cstdint>
#include <array>
#include <SFML/System/Vector2.hpp>


namespace padi {

    size_t hash_c_string(const char *p, size_t s);

    uint32_t rgb(int r, int g, int b);
    uint32_t hsv(int hue, float sat, float val);

    sf::Vector2f round(sf::Vector2f f);
    sf::Vector2i orthAxis(sf::Vector2i const& dir);


    struct less {
        bool operator()(std::pair<size_t, sf::Vector2i> const &left, std::pair<size_t, sf::Vector2i> const &right) const {
            return left.first > right.first;
        }

        bool operator()(sf::Vector2i const &left, sf::Vector2i const &right) const {
            return left.x > right.x || (left.x == right.x && left.y > right.y);
        }
    };
}
