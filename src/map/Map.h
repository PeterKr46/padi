//
// Created by Peter on 27/04/2022.
//
#pragma once

#include <queue>
#include <map>
#include <unordered_set>
#include <functional>
#include <memory>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/View.hpp>
#include "GridObject.h"

namespace padi {

    struct depth_order {
        bool operator()(sf::Vector2i const &a, sf::Vector2i const &b) const {
            int a_sum = a.x + a.y, b_sum = b.x + b.y;
            return a_sum < b_sum || (a_sum == b_sum && a.x < b.x);
        }

        static bool equal(sf::Vector2i const &a, sf::Vector2i const &b) {
            return a.x == b.x && a.y == b.y;
        }
    };

    template<typename T>
    using ManhattanMap = std::map<const sf::Vector2i, T, depth_order>;

    class Tile;

    class Entity;

    /**
     * Map is a storage class which can be used to represent a board state.
     * It maps out Tiles, Entities and UIObjects.
     *
     */

    class Map {
    public:
        [[nodiscard]] static sf::Vector2i mapWorldPosToTile(sf::Vector2f const &world);

        [[nodiscard]] static sf::Vector2f mapTilePosToWorld(sf::Vector2i const &tile, int z = 0);

        [[nodiscard]] std::shared_ptr<padi::Tile> getTile(sf::Vector2i const &pos) const;

        [[nodiscard]] std::shared_ptr<padi::Tile> getTile(int x, int y) const;

        bool addTile(const std::shared_ptr<padi::Tile> &tile);
        std::shared_ptr<Tile> addTileIfNone(sf::Vector2i const &pos, bool* added = nullptr);

        size_t getEntities(sf::Vector2i const &pos, std::vector<std::shared_ptr<Entity>> &entities) const;

        void addEntity(const std::shared_ptr<Entity> &entity, size_t lower_by = 0);

        void addEntity(const std::shared_ptr<Entity> &entity, sf::Vector2i const &where, size_t lower_by = 0);

        bool removeEntity(const std::shared_ptr<Entity> &entity);

        bool removeEntity(const std::shared_ptr<Entity> &entity, sf::Vector2i const &where);

        bool moveEntity(const std::shared_ptr<Entity> &entity, sf::Vector2i const &pos2, size_t lower_by = 0);

        [[nodiscard]] bool hasEntities(sf::Vector2i const &pos) const;
        [[nodiscard]] bool hasEntities(sf::Vector2i const &pos, uint32_t entityType) const;

        std::shared_ptr<Entity> getEntity(const sf::Vector2i &pos, uint32_t t);

        [[nodiscard]] size_t numQuads() const;

        size_t populate(sf::VertexArray &array, size_t vertexOffset, uint8_t frame, sf::View const &viewport) const;

        void for_each(const std::function<void(std::shared_ptr<padi::Tile>)> &);
        [[nodiscard]] const std::unordered_set<std::shared_ptr<Entity>> & allEntities() const;


        void removeTile(int x, int y);
        void removeTile(sf::Vector2i const &vector2);

        bool addUIObject(const std::shared_ptr<padi::GridObject> &obj);

        bool removeUIObject(const std::shared_ptr<padi::GridObject> &obj);

        bool removeUIObject(const std::shared_ptr<padi::GridObject> &obj, sf::Vector2i const &where);

        bool moveUIObject(const std::shared_ptr<padi::GridObject> &obj, sf::Vector2i const &pos);

        size_t numTiles();

    private:
        ManhattanMap<std::pair<std::shared_ptr<Tile>, std::vector<std::shared_ptr<Entity>>>> m_tiles;
        std::unordered_set<std::shared_ptr<Entity>> m_entities;
        ManhattanMap<std::shared_ptr<padi::GridObject>> m_ui;
    };


} // padi

