//
// Created by Peter on 27/04/2022.
//
#pragma once

#include <queue>
#include <map>
#include "SFML/Graphics/Transformable.hpp"
#include "SFML/Graphics/VertexArray.hpp"

namespace padi {

    class GridPlaceable {
    public:
        explicit GridPlaceable(sf::Vector2i const& pos) : m_position(pos) { }

        sf::Vector2i getPosition() const { return m_position; };
    private:
        friend class Map;
        sf::Vector2i m_position{0,0};
    };

    struct depth_order {
        bool operator() (sf::Vector2i const& a, sf::Vector2i const& b) const {
            int a_sum = a.x + a.y, b_sum = b.x + b.y;
            return a_sum < b_sum || (a_sum == b_sum && a.x < b.x);
        }
        bool equal(sf::Vector2i const& a, sf::Vector2i const& b) const {
            return a.x == b.x && a.y == b.y;
        }
    };

    template<typename T>
    using ManhattanMap = std::map<const sf::Vector2i, T, depth_order>;


    class Tile : public GridPlaceable {
    public:
        explicit Tile(sf::Vector2i const& pos);
        void populate(Map const& context, sf::Vertex* quad) const;
    };

    class Entity : public GridPlaceable{
    public:
        explicit Entity(sf::Vector2i const& pos);
        void populate(Map const &map, sf::Vertex *pVertex) const;
    };

    class Map
        : public sf::Transformable {
    public:
        sf::Vector2i mapWorldPosToTile(sf::Vector2f const& world) const;
        sf::Vector2f mapTilePosToWorld(sf::Vector2i const& tile, int z = 0) const;

        Tile* getTile(sf::Vector2i const& pos);
        const Tile* getTile(sf::Vector2i const& pos) const;

        Tile* getTile(int x, int y);
        const Tile* getTile(int x, int y) const;

        bool addTile(Tile* tile);

        size_t getEntities(sf::Vector2i const& pos, std::vector<Entity*>& entities);
        size_t getEntities(sf::Vector2i const& pos, std::vector<Entity*>& entities) const;

        void addEntity(Entity* entity);
        void addEntity(Entity* entity, sf::Vector2i const& where);

        bool removeEntity(const Entity* entity);
        bool removeEntity(const Entity* entity, sf::Vector2i const& where);

        bool moveEntity(Entity* entity, sf::Vector2i const& pos1, sf::Vector2i const& pos2);

        size_t numQuads() const;

        size_t populate(sf::VertexArray & array, size_t vertexOffset, sf::Vector2i const & tile_size);

        const sf::Vector2i& getTileSize() const;

    private:
        ManhattanMap<Tile *> m_tiles;
        ManhattanMap<std::vector<Entity *>> m_entities;
        sf::Vector2i m_tileSize{32,32};
    };


} // padi

