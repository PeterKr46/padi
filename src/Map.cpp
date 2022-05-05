//
// Created by Peter on 27/04/2022.
//

#include "Map.h"

#include <cmath>

#include "Entity.h"

namespace padi {

    sf::Vector2i Map::mapWorldPosToTile(const sf::Vector2f &world) const {
        sf::Vector2f local = getTransform().getInverse().transformPoint(world) + sf::Vector2f (m_tileSize.x / 2.f, m_tileSize.y / 4.f);
        return {int(2 * local.y + (local.x - m_tileSize.x / 2)) / m_tileSize.x, int(2 * local.y - (local.x - m_tileSize.x / 2)) / m_tileSize.x};
    }

    sf::Vector2f Map::mapTilePosToWorld(const sf::Vector2i &tile, int z) const {
        return {float(tile.x - tile.y) * 0.5f * m_tileSize.x, float(tile.x + tile.y - z) * 0.25f * m_tileSize.y};
    }

    std::shared_ptr<padi::Tile> Map::getTile(const sf::Vector2i &pos) const {
        auto iter = m_tiles.find(pos);
        if (iter != m_tiles.end()) {
            return iter->second.first;
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
        total = m_tiles.size();
        // TODO: This could be streamlined by keeping track of Entity removals/additions..?
        for(const auto& loc : m_tiles) {
            total += loc.second.second.size();
        }
        return total;
    }

    size_t Map::populate(sf::VertexArray &array, size_t vertexOffset, const sf::Vector2i &tile_size) {
        m_cycleCarry += m_cycle.restart();
        while(m_cycleCarry.asMilliseconds() >= 1000) {
            m_cycleCarry -= sf::seconds(1);
        }

        size_t quads = numQuads();
        if(array.getVertexCount() <= vertexOffset + quads * 4) {
            array.resize(vertexOffset + quads * 6); // double to prevent minor increases causing trouble..?
        }
        auto tileIter = m_tiles.begin();

        size_t idx = 0;

        while(tileIter != m_tiles.end()) {
            if(tileIter->second.first) {
                tileIter->second.first->populate(*this, &array[vertexOffset + idx * 4]);
                idx++;
            }
            for (auto &entity: tileIter->second.second) {
                entity->populate(this, &array[vertexOffset + idx * 4]);
                idx++;
            }
            tileIter = std::next(tileIter);
        }
        return quads * 4;
    }

    sf::Vector2i Map::getTileSize() const {
        return m_tileSize;
    }

    sf::Time Map::getCurrentCycleTime() const {
        return m_cycleCarry;
    }

    int Map::getCurrentCycleFrames() const {
        return std::floor(m_cycleCarry.asSeconds() * 12);
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

    void Tile::populate(const Map &context, sf::Vertex *quad) const {
        sf::Vector2f anchor = context.mapTilePosToWorld(getPosition());
        sf::Vector2f tileSize = {static_cast<float>(context.getTileSize().x), static_cast<float>(context.getTileSize().y)};
        quad[0].position = anchor+sf::Vector2f(-tileSize.x / 2,-tileSize.y / 2);
        quad[1].position = anchor+sf::Vector2f(tileSize.x / 2,-tileSize.y / 2);
        quad[2].position = anchor+sf::Vector2f(tileSize.x / 2,tileSize.y / 2);
        quad[3].position = anchor+sf::Vector2f(-tileSize.x / 2,tileSize.y / 2);
        quad[0].color = m_color;
        quad[1].color = m_color;
        quad[2].color = m_color;
        quad[3].color = m_color;


        quad[0].texCoords = sf::Vector2f(128 + 0, 0 + m_detail * tileSize.y);
        quad[1].texCoords = sf::Vector2f(128 + 32, 0 + m_detail * tileSize.y);
        quad[2].texCoords = sf::Vector2f(128 + 32, 32 + m_detail * tileSize.y);
        quad[3].texCoords = sf::Vector2f(128 + 0, 32 + m_detail * tileSize.y);
    }

    Tile::Tile(const sf::Vector2i &pos) : GridPlaceable(pos) {

    }

} // padi