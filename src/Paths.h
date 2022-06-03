//
// Created by Peter on 29/04/2022.
//
#pragma once

#include <vector>
#include <array>
#include <iostream>
#include <SFML/System/Vector2.hpp>
#include "map/Tile.h"
#include "Constants.h"


namespace padi {

    struct compair { // xd
        bool
        operator()(std::pair<size_t, sf::Vector2i> const &left, std::pair<size_t, sf::Vector2i> const &right) const {
            return left.first > right.first;
        }

        bool operator()(sf::Vector2i const &left, sf::Vector2i const &right) const {
            return left.x < right.x || (left.x == right.x && left.y < right.y);
        }
    };

    std::map<sf::Vector2i, sf::Vector2i, compair> Crawl(padi::Map *map, sf::Vector2i const &from, size_t range);

    std::vector<sf::Vector2i> FindPath(padi::Map *map, sf::Vector2i const &from, sf::Vector2i const &to);
    }
