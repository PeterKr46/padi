//
// Created by peter on 03/06/22.
//
#include "Paths.h"

namespace padi {

    int L1(sf::Vector2i const &a, sf::Vector2i const &b) {
        return abs(a.x - b.x) + abs(a.y - b.y);
    }

    std::map<sf::Vector2i, sf::Vector2i, compair> Crawl(padi::Map *map, const sf::Vector2i &from, size_t range) {
        std::map<sf::Vector2i, sf::Vector2i, compair> cameFrom;
        std::map<sf::Vector2i, size_t, compair> geodesicDistance;

        std::priority_queue<std::pair<size_t, sf::Vector2i>, std::vector<std::pair<size_t, sf::Vector2i>>, compair> frontier;

        frontier.push({0, from});
        geodesicDistance.insert({from, 0});

        while (!frontier.empty()) {
            auto bestGuess = frontier.top();
            frontier.pop();
            if(bestGuess.first <= range) {
                for (auto d: padi::AllDirections) {
                    sf::Vector2i neighbor = bestGuess.second + d;
                    auto neighborTile = map->getTile(neighbor);
                    if (neighborTile && neighborTile->m_walkable && !map->hasEntities(neighbor) ) {
                        auto score = geodesicDistance.find(neighbor);
                        if (score == geodesicDistance.end()) {
                            frontier.push({bestGuess.first + 1, neighbor});
                            geodesicDistance.insert({neighbor, bestGuess.first + 1});
                            cameFrom[neighbor] = bestGuess.second;
                        } else if (score->second > bestGuess.first + 1) {
                            score->second = bestGuess.first + 1;
                            cameFrom[neighbor] = bestGuess.second;
                        }
                    }
                }
            }
        }
        return cameFrom;
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


        std::map<sf::Vector2i, sf::Vector2i, compair> cameFrom;
        std::map<sf::Vector2i, size_t, compair> geodesicDistance;

        std::priority_queue<std::pair<size_t, sf::Vector2i>, std::vector<std::pair<size_t, sf::Vector2i>>, compair> frontier;

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
                if (neighborTile && neighborTile->m_walkable &&  !map->hasEntities(neighbor)) {
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