//
// Created by Peter on 27/04/2022.
//

#include "Map.h"

#include <cmath>
#include <iostream>

#include "../entity/Entity.h"
#include "Tile.h"
#include "../Constants.h"

namespace padi {

    sf::Vector2i Map::mapWorldPosToTile(const sf::Vector2f &world) {
        sf::Vector2f local = world + sf::Vector2f(padi::TileSize.x / 2.f, padi::TileSize.y / 4.f);
        return {int(2 * local.y + (local.x - padi::TileSize.x / 2)) / padi::TileSize.x,
                int(2 * local.y - (local.x - padi::TileSize.x / 2)) / padi::TileSize.x};
    }

    sf::Vector2f Map::mapTilePosToWorld(const sf::Vector2i &tile, int z) {
        return {float(tile.x - tile.y) * 0.5f * padi::TileSize.x,
                float(tile.x + tile.y - z) * 0.25f * padi::TileSize.y};
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
            out.resize(iter->second.second.size());
            std::copy(iter->second.second.begin(), iter->second.second.end(), out.begin());
            return iter->second.second.size();
        }
        return 0;
    }

    bool Map::moveEntity(const std::shared_ptr<Entity> &entity, const sf::Vector2i &pos2, size_t lower_by) {
        removeEntity(entity);
        addEntity(entity, pos2, lower_by);
        return true; // TODO ?
    }

    bool Map::removeEntity(const std::shared_ptr<Entity> &entity) {
        return removeEntity(entity, entity->getPosition());
    }

    bool Map::removeEntity(const std::shared_ptr<Entity> &entity, const sf::Vector2i &pos) {
        auto tileIter = m_tiles.find(pos);
        if (tileIter != m_tiles.end()) { // Check entry at pos
            auto tileEntityIter = std::find(tileIter->second.second.begin(), tileIter->second.second.end(), entity);
            if (tileEntityIter != tileIter->second.second.end()) { // Check entry in vector
                tileIter->second.second.erase(tileEntityIter);
                if (!tileIter->second.first && tileIter->second.second.empty()) { // Remove empty position from m_map
                    m_tiles.erase(tileIter);
                }
                auto const &entitySetIter = m_entities.find(entity);
                if (entitySetIter != m_entities.end()) {
                    m_entities.erase(entitySetIter);
                }
                return true;
            }
        }
        return false;
    }

    void Map::addEntity(const std::shared_ptr<Entity> &entity, const sf::Vector2i &where, size_t lower_by) {
        // potentially assert tile existence?
        auto tileIter = m_tiles.find(where);
        if (tileIter != m_tiles.end()) { // Check entry at pos
            if (lower_by >= tileIter->second.second.size()) {
                tileIter->second.second.insert(tileIter->second.second.begin(), entity);
            } else {
                tileIter->second.second.insert(
                        tileIter->second.second.begin() + (tileIter->second.second.size() - lower_by), entity);
            }
            entity->m_position = where;
            //entity->setVerticalOffset(tileIter->second.first->getVerticalOffset());
            m_entities.insert(entity);
        }
    }

    size_t Map::numQuads() const {
        size_t total;
        total = m_ui.size();
        // TODO: This could be streamlined by keeping track of Entity removals/additions..?
        for (const auto &loc: m_tiles) {
            if (loc.second.first) {
                total += loc.second.first->numQuads();
            }
        }
        for (const auto &entity: m_entities) {
            total += entity->numQuads();
        }
        return total;
    }

    size_t Map::populate(sf::VertexArray &array, size_t vertexOffset, uint8_t frame, sf::View const &viewport) const {
        size_t quads = numQuads();

        if (array.getVertexCount() <= vertexOffset + quads * 4) {
            std::cout << "VBO RESIZE: " << array.getVertexCount() << " > " << vertexOffset + quads * 6 << std::endl;
            array.resize(vertexOffset + quads * 6); // double to prevent minor increases causing trouble..?
        }

        size_t idx = 0;
        auto tileIter = m_tiles.begin();
        while (tileIter != m_tiles.end()) {
            float vOff = 0;
            if (tileIter->second.first) {
                idx += tileIter->second.first->populate(this, array, vertexOffset + idx, frame, 0);
                vOff = tileIter->second.first->getVerticalOffset();
            }
            for (auto &entity: tileIter->second.second) {
                idx += entity->populate(this, array, vertexOffset + idx, frame, vOff);
            }
            ++tileIter;
        }

        for (auto const &[loc, obj]: m_ui) {
            idx += obj->populate(this, array, vertexOffset + idx, frame, 0); // TODO vertical offset
        }

        return idx;
    }

    void Map::for_each(const std::function<void(std::shared_ptr<padi::Tile>)> &func) {
        for (const auto &[key, value]: m_tiles) {
            func(value.first);
        }
    }

    bool Map::addTile(const std::shared_ptr<padi::Tile> &t) {
        if (m_tiles.find(t->getPosition()) == m_tiles.end()) {
            m_tiles[t->getPosition()] = {t, {}};
            return true;
        }
        return false;
    }

    void Map::addEntity(const std::shared_ptr<Entity> &e, size_t lower_by) {
        addEntity(e, e->getPosition(), lower_by);
    }

    void Map::removeTile(const sf::Vector2i &p) {
        auto iter = m_tiles.find(p);
        if (iter != m_tiles.end()) {
            m_tiles.erase(iter);
        }
    }

    bool Map::addUIObject(const std::shared_ptr<padi::GridObject> &t) {
        m_ui[t->getPosition()] = t;
        return true;
    }

    bool Map::removeUIObject(const std::shared_ptr<padi::GridObject> &obj, const sf::Vector2i &pos) {
        auto iter = m_ui.find(pos);
        if (iter != m_ui.end()) { // Check entry at pos
            if (iter->second == obj) {
                m_ui.erase(iter);
                return true;
            }
        }
        return false;
    }

    bool Map::removeUIObject(const std::shared_ptr<padi::GridObject> &obj) {
        return removeUIObject(obj, obj->getPosition());
    }

    bool Map::moveUIObject(const std::shared_ptr<padi::GridObject> &obj, const sf::Vector2i &pos) {
        removeUIObject(obj);
        obj->m_position = pos;
        addUIObject(obj);
        return true; // TODO
    }

    bool Map::hasEntities(const sf::Vector2i &pos) const {
        auto iter = m_tiles.find(pos);
        if (iter != m_tiles.end()) {
            return !iter->second.second.empty();
        }
        return false;
    }

    const std::unordered_set<std::shared_ptr<Entity>> &Map::allEntities() const {
        return m_entities;
    }

    size_t Map::numTiles() {
        return m_tiles.size();
    }


} // padi