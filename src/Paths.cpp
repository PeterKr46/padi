//
// Created by peter on 03/06/22.
//
#include "Paths.h"

namespace padi {

    int L1(sf::Vector2i const &a, sf::Vector2i const &b) {
        return abs(a.x - b.x) + abs(a.y - b.y);
    }

    std::vector<sf::Vector2i> Crawl(padi::Map *map, sf::Vector2i const &from, size_t range,
                                    const std::function<bool(const Map* map, std::shared_ptr<Tile> const &)>& walkable) {
        std::map<std::shared_ptr<Tile>, size_t> geodesicDistance;

        std::queue<std::pair<std::shared_ptr<Tile>, size_t>> frontier;

        std::shared_ptr<Tile> tile;
        size_t tileCost;

        std::shared_ptr<Tile> neighborTile;

        tile = map->getTile(from);
        if (tile) {
            frontier.push({tile, 0});
            geodesicDistance[tile] = 0;

            while (!frontier.empty()) {
                tile = frontier.front().first;
                frontier.pop();
                tileCost = geodesicDistance[tile];

                for (auto &dir: AllDirections) {
                    neighborTile = map->getTile(tile->getPosition() + dir);

                    if (neighborTile && walkable(map, neighborTile)) {
                        auto costIter = geodesicDistance.find(neighborTile);
                        if (costIter == geodesicDistance.end()) {
                            geodesicDistance[neighborTile] = geodesicDistance[tile] + 1;
                            if (tileCost+1 < range)
                                frontier.push({neighborTile, tileCost+1});
                        } else if (costIter->second > tileCost + 1) {
                            costIter->second = tileCost + 1;
                            if (tileCost+1 < range)
                               frontier.push({neighborTile, tileCost+1});
                        }
                    }
                }
            }
        }
        std::vector<sf::Vector2i> result;
        result.reserve(geodesicDistance.size());
        for(auto const& [t, cost] : geodesicDistance) {
            if (cost > 0) result.emplace_back(t->getPosition());
        }
        return result;
    }

    std::vector<sf::Vector2i> FindPath(padi::Map *map, const sf::Vector2i &from, const sf::Vector2i &to, const std::function<bool(const Map* map, std::shared_ptr<Tile> const &)>& walkable) {
        std::shared_ptr<Tile> tileFrom, tileTo;
        tileFrom = map->getTile(from);
        tileTo = map->getTile(to);

        std::map<std::shared_ptr<Tile>, size_t> geodesicDistance;
        std::map<std::shared_ptr<Tile>, std::shared_ptr<Tile>> cameFrom;

        std::queue<std::pair<std::shared_ptr<Tile>, size_t>> frontier;

        std::shared_ptr<Tile> tile;
        size_t tileCost;

        std::shared_ptr<Tile> neighborTile;

        tile = tileFrom;
        if (tileFrom && tileTo && walkable(map, tileTo)) {
            frontier.push({tile, 0});
            geodesicDistance[tile] = 0;
            geodesicDistance[tileTo] = -1;

            while (!frontier.empty()) {
                tile = frontier.front().first;
                frontier.pop();
                tileCost = geodesicDistance[tile];
                if(tileCost < geodesicDistance[tileTo]) {
                    for (auto &dir: AllDirections) {
                        neighborTile = map->getTile(tile->getPosition() + dir);
                        if (neighborTile && walkable(map,neighborTile)) {
                            auto costIter = geodesicDistance.find(neighborTile);
                            if (costIter == geodesicDistance.end()) {
                                geodesicDistance[neighborTile] = tileCost + 1;
                                cameFrom[neighborTile] = tile;
                                frontier.push({neighborTile, tileCost + 1 + L1(neighborTile->getPosition(), to)});
                            } else if (costIter->second > tileCost + 1) {
                                costIter->second = tileCost + 1;
                                cameFrom[neighborTile] = tile;
                                frontier.push({neighborTile, tileCost + 1 + L1(neighborTile->getPosition(), to)});
                            }
                        }
                    }
                }
            }
        } else {
            printf("[Paths] ERROR: Start or Target tile does not exist, or target not walkable.\n");
        }
        std::vector<sf::Vector2i> result;
        if (cameFrom.find(tileTo) != cameFrom.end()) {
            auto p = tileTo;
            while (p != tileFrom) {
                result.push_back(p->getPosition() - cameFrom.at(p)->getPosition());
                p = cameFrom.at(p);
            }
            std::reverse(result.begin(), result.end());
        } else {
            printf("[Paths] ERROR: Failed to find a path to target!\n");
        }
        return result;
    }

    std::vector<sf::Vector2i> FindPath(padi::Map *map, const sf::Vector2i &from, const sf::Vector2i &to) {
        auto t = map->getTile(from);
        auto result = std::vector<sf::Vector2i>();

        if (!t) return result;

        std::array<sf::Vector2i, 4> adjacency;
        adjacency[0] = {0, -1};
        adjacency[1] = {0, 1};
        adjacency[2] = {1, 0};
        adjacency[3] = {-1, 0};


        std::map<sf::Vector2i, sf::Vector2i, less> cameFrom;
        std::map<sf::Vector2i, size_t, less> geodesicDistance;

        std::priority_queue<std::pair<size_t, sf::Vector2i>, std::vector<std::pair<size_t, sf::Vector2i>>, less> frontier;

        frontier.push({L1(from, to), from});
        geodesicDistance.insert({from, 0});

        while (!frontier.empty()) {
            auto bestGuess = frontier.top();
            frontier.pop();
            if (bestGuess.second == to) {
                break;
            }
            for (auto d: adjacency) {
                auto neighbor = bestGuess.second + d;
                auto neighborTile = map->getTile(neighbor);
                if (neighborTile && neighborTile->m_walkable && !map->hasEntities(neighbor)) {
                    auto score = geodesicDistance.find(neighbor);
                    if (score == geodesicDistance.end()) {
                        frontier.push({bestGuess.first + 1 + L1(neighbor, to), neighbor});
                        geodesicDistance.insert({neighbor, bestGuess.first + 1});
                        cameFrom[neighbor] = bestGuess.second;
                    } else if (score->second > bestGuess.first + 1) {
                        score->second = bestGuess.first + 1;
                        cameFrom[neighbor] = bestGuess.second;
                    }
                }
            }
        }
        if (cameFrom.find(to) != cameFrom.end()) {
            auto p = to;
            while (p != from) {
                result.push_back(p - cameFrom.at(p));
                p = cameFrom.at(p);
            }
            std::reverse(result.begin(), result.end());
            for (auto n: result) {
                std::cout << n.x << "," << n.y << std::endl;
            }
        }
        return result;
    }
}