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

    std::vector<sf::Vector2i> Crawl(padi::Map *map, sf::Vector2i const &from, size_t range, const std::function<bool(const Map* map, std::shared_ptr<Tile> const&)>& walkable);

    std::vector<sf::Vector2i> FindPath(padi::Map *map, const sf::Vector2i &from, const sf::Vector2i &to, const std::function<bool(const Map* map, std::shared_ptr<Tile> const &)>& walkable);
}
