//
// Created by Peter on 27/04/2022.
//

#include "Map.h"

#include <cmath>
#include <iostream>

#include "../entity/Entity.h"

namespace padi {

    sf::Vector2i Map::mapWorldPosToTile(const sf::Vector2f &world) const {
        sf::Vector2f local = world + sf::Vector2f (m_tileSize.x / 2.f, m_tileSize.y / 4.f);
        return {int(2 * local.y + (local.x - m_tileSize.x / 2)) / m_tileSize.x, int(2 * local.y - (local.x - m_tileSize.x / 2)) / m_tileSize.x};
    }

    sf::Vector2f Map::mapTilePosToWorld(const sf::Vector2i &tile, int z) const {
        return {float(tile.x - tile.y) * 0.5f * m_tileSize.x, float(tile.x + tile.y - z) * 0.25f * m_tileSize.y};
    }

    std::shared_ptr<padi::Tile> Map::getTile(const sf::Vector2i &pos) const {
        auto iter = m_tiles.find(pos);
        if (iter != m_tiles.end()) {
            return {iter->second.first};
        }
        return {nullptr};
    }

    std::shared_ptr<padi::Tile> Map::getTile(int x, int y) const {
        return getTile({x, y});
    }

    size_t Map::getEntities(const sf::Vector2i &pos, std::vector<std::shared_ptr<Entity>> &out) const {
        auto iter = m_tiles.find(pos);
        if (iter != m_tiles.end()) {
            std::copy(iter->second.second.begin(), iter->second.second.end(), out.begin());
            return iter->second.second.size();
        }
        return 0;
    }

    bool Map::moveEntity(const std::shared_ptr<Entity>& entity, const sf::Vector2i &pos2, size_t lower_by) {
        removeEntity(entity);
        addEntity(entity, pos2, lower_by);
        entity->m_position = pos2;
        return true; // TODO ?
    }

    bool Map::removeEntity(const std::shared_ptr<Entity>& entity) {
        return removeEntity(entity, entity->getPosition());
    }

    bool Map::removeEntity(const std::shared_ptr<Entity>& entity, const sf::Vector2i &pos) {
        auto iter = m_tiles.find(pos);
        if (iter != m_tiles.end()) { // Check entry at pos
            auto rmAt = std::find(iter->second.second.begin(), iter->second.second.end(), entity);
            if (rmAt != iter->second.second.end()) { // Check entry in vector
                iter->second.second.erase(rmAt);
                if(!iter->second.first && iter->second.second.empty()) { // Remove empty position from m_map
                    m_tiles.erase(iter);
                }
                return true;
            }
        }
        return false;
    }

    void Map::addEntity(const std::shared_ptr<Entity>& entity, const sf::Vector2i &where, size_t lower_by) {
        // potentially assert tile existence?
        auto iter = m_tiles.find(where);
        if (iter != m_tiles.end()) { // Check entry at pos
            if(lower_by >= iter->second.second.size()) {
                iter->second.second.insert(iter->second.second.begin(), entity);
            }
            else {
                iter->second.second.insert(iter->second.second.begin() + (iter->second.second.size() - lower_by), entity);
            }
            entity->m_position = where;
        }
    }

    size_t Map::numQuads() const {
        size_t total;
        total =  m_ui.size();
        // TODO: This could be streamlined by keeping track of Entity removals/additions..?
        for(const auto& loc : m_tiles) {
            if(loc.second.first) {
                ++total;
                for (const auto &entity: loc.second.second) {
                    total += entity->numQuads();
                }
            }
        }
        return total;
    }

    size_t Map::populate(sf::VertexArray &array, size_t vertexOffset, uint8_t frame) {
        size_t quads = numQuads();
        //std::cout << "QUAD: " << 1.f / population.restart().asSeconds() << std::endl;

        if(array.getVertexCount() <= vertexOffset + quads * 4) {
            std::cout << "VBO RESIZE: " << array.getVertexCount() << " > " << vertexOffset + quads * 6 << std::endl;
            array.resize(vertexOffset + quads * 6); // double to prevent minor increases causing trouble..?
        }
        auto tileIter = m_tiles.begin();

        size_t idx = 0;
        sf::Clock population;
        while(tileIter != m_tiles.end()) {
            if(tileIter->second.first) {
                idx += tileIter->second.first->populate(this, array, vertexOffset + idx, frame);
            }
            ++tileIter;
        }
        //std::cout << "TILES " << 1.f / population.restart().asSeconds() << std::endl;
        tileIter = m_tiles.begin();
        while(tileIter != m_tiles.end()) {
            for (auto &entity: tileIter->second.second) {
                idx += entity->populate(this, array, vertexOffset + idx, frame);
            }
            ++tileIter;
        }
        //std::cout << "ENT " << 1.f / population.restart().asSeconds() << std::endl;

        for(auto const& [loc, obj] : m_ui) {
            idx += obj->populate(this, array, vertexOffset + idx, frame);
        }

        //std::cout << "UI: " << 1.f / population.restart().asSeconds() << std::endl;

        return quads * 4;
    }

    sf::Vector2i Map::getTileSize() const {
        return m_tileSize;
    }

    void Map::for_each(const std::function<void(std::shared_ptr<padi::Tile>)>& func) {
        for(const auto& [key, value] : m_tiles) {
            func(value.first);
        }
    }

    bool Map::addTile(std::shared_ptr<padi::Tile> t) {
        if(m_tiles.find(t->getPosition()) == m_tiles.end()) {
            m_tiles[t->getPosition()] = {t,{}};
            return true;
        }
        return false;
    }

    void Map::addEntity(const std::shared_ptr<Entity> e, size_t lower_by) {
        addEntity(e, e->getPosition(), lower_by);
    }

    void Map::removeTile(const sf::Vector2i &p) {
        auto iter = m_tiles.find(p);
        if(iter != m_tiles.end()) {
            m_tiles.erase(iter);
        }
    }

    bool Map::addUIObject(const std::shared_ptr<padi::UIObject> &t) {
        if (m_ui.find(t->getPosition()) == m_ui.end()) {
            m_ui[t->getPosition()] = t;
            return true;
        }
        return false;
    }

    bool Map::removeUIObject(std::shared_ptr<padi::UIObject> obj, const sf::Vector2i &pos) {
        auto iter = m_ui.find(pos);
        if (iter != m_ui.end()) { // Check entry at pos
            if(iter->second == obj) {
                m_ui.erase(iter);
                return true;
            }
        }
        return false;
    }

    bool Map::removeUIObject(std::shared_ptr<padi::UIObject> obj) {
        return removeUIObject(obj, obj->getPosition());
    }

    bool Map::moveUIObject(const std::shared_ptr<padi::UIObject> &obj, const sf::Vector2i &pos) {
        removeUIObject(obj);
        obj->m_position = pos;
        addUIObject(obj);
        return true; // TODO
    }



} // padi