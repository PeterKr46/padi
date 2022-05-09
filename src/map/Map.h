//
// Created by Peter on 27/04/2022.
//
#pragma once

#include <queue>
#include <map>
#include <functional>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/View.hpp>
#include "GridObject.h"

namespace padi {

    struct depth_order {
        bool operator() (sf::Vector2i const& a, sf::Vector2i const& b) const {
            int a_sum = a.x + a.y, b_sum = b.x + b.y;
            return a_sum < b_sum || (a_sum == b_sum && a.x < b.x);
        }
        static bool equal(sf::Vector2i const& a, sf::Vector2i const& b) {
            return a.x == b.x && a.y == b.y;
        }
    };

    template<typename T>
    using ManhattanMap = std::map<const sf::Vector2i, T, depth_order>;

    class Tile;
    class Entity;

    class Map {
    public:
        [[nodiscard]] sf::Vector2i mapWorldPosToTile(sf::Vector2f const& world) const;
        [[nodiscard]] sf::Vector2f mapTilePosToWorld(sf::Vector2i const& tile, int z = 0) const;

        [[nodiscard]] std::shared_ptr<padi::Tile> getTile(sf::Vector2i const& pos) const;
        [[nodiscard]] std::shared_ptr<padi::Tile> getTile(int x, int y) const;

        bool addTile(std::shared_ptr<padi::Tile> tile);

        size_t getEntities(sf::Vector2i const& pos, std::vector<std::shared_ptr<Entity>>& entities) const;

        void addEntity(std::shared_ptr<Entity> entity, size_t lower_by = 0);
        void addEntity(const std::shared_ptr<Entity>& entity, sf::Vector2i const& where, size_t lower_by = 0);

        bool removeEntity(const std::shared_ptr<Entity>& entity);
        bool removeEntity(const std::shared_ptr<Entity>& entity, sf::Vector2i const& where);

        bool moveEntity(const std::shared_ptr<Entity>& entity, sf::Vector2i const& pos2, size_t lower_by = 0);

        [[nodiscard]] size_t numQuads() const;

        size_t populate(sf::VertexArray & array, size_t vertexOffset, uint8_t frame, sf::View const& viewport) const;

        [[nodiscard]] sf::Vector2i getTileSize() const;

        void for_each(const std::function<void(std::shared_ptr<padi::Tile>)>&);

        void removeTile(sf::Vector2i const &vector2);

        bool addUIObject(const std::shared_ptr<padi::GridObject>& obj);

        bool removeUIObject(std::shared_ptr<padi::GridObject> obj);
        bool removeUIObject(std::shared_ptr<padi::GridObject> obj, sf::Vector2i const& where);

        bool moveUIObject(const std::shared_ptr<padi::GridObject> & obj, sf::Vector2i const& pos);

    private:
        ManhattanMap<std::pair<std::shared_ptr<Tile>, std::vector<std::shared_ptr<Entity>>>> m_tiles;
        ManhattanMap<std::shared_ptr<padi::GridObject>> m_ui;
        sf::Vector2i m_tileSize{32,32};
    };


} // padi

