//
// Created by Peter on 27/04/2022.
//

#include "Map.h"

namespace padi {

    sf::Vector2i Map::mapWorldPosToTile(const sf::Vector2f &world) const {
        sf::Vector2f local = getTransform().getInverse().transformPoint(world);
        return {int(2 * local.y + (local.x - 16)) / 32, int(2 * local.y - (local.x - 16)) / 32};
    }

    sf::Vector2f Map::mapTilePosToWorld(const sf::Vector2i &tile, int z) const {
        return {float(tile.x - tile.y) * 0.5f * m_tileSize.x, float(tile.x + tile.y - z) * 0.25f * m_tileSize.y};
    }

    Tile *Map::getTile(sf::Vector2i const &pos) {
        auto iter = m_tiles.find(pos);
        if (iter != m_tiles.end()) {
            return iter->second;
        }
        return nullptr;
    }

    const Tile *Map::getTile(const sf::Vector2i &pos) const {
        auto iter = m_tiles.find(pos);
        if (iter != m_tiles.end()) {
            return iter->second;
        }
        return nullptr;
    }

    Tile *Map::getTile(int x, int y) {
        return getTile({x, y});
    }

    const Tile *Map::getTile(int x, int y) const {
        return getTile({x, y});
    }

    size_t Map::getEntities(const sf::Vector2i &pos, std::vector<Entity *> &out) {
        auto iter = m_entities.find(pos);
        if (iter != m_entities.end()) {
            std::copy(iter->second.begin(), iter->second.end(), out.begin());
            return iter->second.size();
        }
        return 0;
    }

    size_t Map::getEntities(const sf::Vector2i &pos, std::vector<Entity *> &out) const {
        auto iter = m_entities.find(pos);
        if (iter != m_entities.end()) {
            std::copy(iter->second.begin(), iter->second.end(), out.begin());
            return iter->second.size();
        }
        return 0;
    }

    bool Map::moveEntity(Entity *entity, const sf::Vector2i &pos1, const sf::Vector2i &pos2) {
        removeEntity(entity);
        addEntity(entity, pos2);
        return true; // TODO ?
    }

    bool Map::removeEntity(const Entity *entity) {
        return removeEntity(entity, entity->getPosition());
    }

    bool Map::removeEntity(const Entity *entity, const sf::Vector2i &pos) {
        auto iter = m_entities.find(pos);
        if (iter != m_entities.end()) { // Check entry at pos
            auto rmAt = std::find(iter->second.begin(), iter->second.end(), entity);
            if (rmAt != iter->second.end()) { // Check entry in vector
                iter->second.erase(rmAt);
                if(iter->second.empty()) { // Remove empty vectors
                    m_entities.erase(iter);
                }
                return true;
            }
        }
        return false;
    }

    void Map::addEntity(Entity *entity, const sf::Vector2i &where) {
        // potentially assert tile existence?
        m_entities[where].push_back(entity);
    }

    void Map::addEntity(Entity *e) {
        addEntity(e, e->getPosition());
    }

    size_t Map::numQuads() const {
        size_t total;
        total = m_tiles.size();
        // TODO: This could be streamlined by keeping track of Entity removals/additions..?
        for(const auto& loc : m_entities) {
            total += loc.second.size();
        }
        return total;
    }

    size_t Map::populate(sf::VertexArray &array, size_t vertexOffset, const sf::Vector2i &tile_size) {
        size_t quads = numQuads();
        if(array.getVertexCount() <= vertexOffset + quads * 4) {
            array.resize(vertexOffset + quads * 6); // double to prevent minor increases causing trouble..?
        }
        auto tileIter = m_tiles.begin();
        auto entityIter = m_entities.begin();

        size_t idx = 0;
        depth_order dTest;

        while(tileIter != m_tiles.end() && entityIter != m_entities.end()) {
            if(dTest.equal(tileIter->first, entityIter->first)) {
                tileIter->second->populate(*this, &array[vertexOffset + idx * 4]);
                idx++;
                tileIter = std::next(tileIter);
            } else {
                if (dTest(tileIter->first, entityIter->first)) {
                    tileIter->second->populate(*this, &array[vertexOffset + idx * 4]);
                    idx++;
                    tileIter = std::next(tileIter);
                } else {
                    for (auto &entity: entityIter->second) {
                        entity->populate(*this, &array[vertexOffset + idx * 4]);
                        idx++;
                    }
                    entityIter = std::next(entityIter);
                }
            }
        }
        while(tileIter != m_tiles.end()) {
            tileIter->second->populate(*this, &array[vertexOffset + idx * 4]);
            idx++;
            tileIter = std::next(tileIter);
        }
        return quads * 4;
    }

    const sf::Vector2i &Map::getTileSize() const {
        return m_tileSize;
    }

    bool Map::addTile(Tile* t) {
        if(m_tiles.find(t->getPosition()) == m_tiles.end()) {
            m_tiles[t->getPosition()] = t;
            return true;
        }
        return false;
    }

    void Tile::populate(const Map &context, sf::Vertex *quad) const {
        sf::Vector2f anchor = context.mapTilePosToWorld(getPosition());
        sf::Vector2f tileSize = {static_cast<float>(context.getTileSize().x), static_cast<float>(context.getTileSize().y)};
        quad[0].position = anchor;
        quad[1].position = anchor+sf::Vector2f(tileSize.x,0);
        quad[2].position = anchor+sf::Vector2f(tileSize.x,tileSize.y);
        quad[3].position = anchor+sf::Vector2f(0,tileSize.y);
        quad[0].color = sf::Color(255,0,0)  ;
        quad[1].color = sf::Color(255,255,0);
        quad[2].color = sf::Color(255,0,255);
        quad[3].color = sf::Color(0,0,255)  ;


        quad[0].texCoords = sf::Vector2f(0, 0);
        quad[1].texCoords = sf::Vector2f(32,0);
        quad[2].texCoords = sf::Vector2f(32,32);
        quad[3].texCoords = sf::Vector2f(0, 32);
    }

    Tile::Tile(const sf::Vector2i &pos) : GridPlaceable(pos) {

    }

    void Entity::populate(Map const& context, sf::Vertex *quad) const {
        sf::Vector2f anchor = context.mapTilePosToWorld(getPosition())-sf::Vector2f (0,14);
        sf::Vector2f tileSize = {static_cast<float>(context.getTileSize().x), static_cast<float>(context.getTileSize().y)};
        quad[0].position = anchor;
        quad[1].position = anchor+sf::Vector2f(tileSize.x,0);
        quad[2].position = anchor+sf::Vector2f(tileSize.x,tileSize.y);
        quad[3].position = anchor+sf::Vector2f(0,tileSize.y);
        quad[0].color = sf::Color(0,255,0)  ;
        quad[1].color = sf::Color(0,255,255);
        quad[2].color = sf::Color(0,0,0);
        quad[3].color = sf::Color(0,0,255)  ;


        quad[0].texCoords = sf::Vector2f(0, 0);
        quad[1].texCoords = sf::Vector2f(32,0);
        quad[2].texCoords = sf::Vector2f(32,32);
        quad[3].texCoords = sf::Vector2f(0, 32);
    }

    Entity::Entity(const sf::Vector2i &pos) : GridPlaceable(pos) {

    }
} // padi