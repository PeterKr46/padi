//
// Created by Peter on 27/04/2022.
//
#pragma once

#include <queue>
#include <map>
#include <functional>
#include "SFML/Graphics/Transformable.hpp"
#include "SFML/Graphics/VertexArray.hpp"
#include "SFML/System/Clock.hpp"

namespace padi {

    class GridPlaceable {
    public:
        explicit GridPlaceable(sf::Vector2i const& pos) : m_position(pos) { }

        sf::Vector2i getPosition() const { return m_position; }
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

        size_t m_detail{0};
        sf::Color m_color{255,255,255};
    };

    class Entity;

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

        size_t getEntities(sf::Vector2i const& pos, std::vector<Entity*>& entities) const;

        void addEntity(Entity* entity, size_t lower_by = 0);
        void addEntity(Entity* entity, sf::Vector2i const& where, size_t lower_by = 0);

        bool removeEntity(const Entity* entity);
        bool removeEntity(const Entity* entity, sf::Vector2i const& where);

        bool moveEntity(Entity* entity, sf::Vector2i const& pos2, size_t lower_by = 0);

        size_t numQuads() const;

        size_t populate(sf::VertexArray & array, size_t vertexOffset, sf::Vector2i const & tile_size);

        sf::Vector2i getTileSize() const;

        sf::Time getCurrentCycleTime() const;
        int getCurrentCycleFrames() const;

        void for_each(const std::function<void(Tile*)>&);

    private:
        ManhattanMap<std::pair<Tile*, std::vector<Entity *>>> m_tiles;
        sf::Vector2i m_tileSize{32,32};

        sf::Clock m_cycle;
        sf::Time m_cycleCarry;
    };


} // padi

