//
// Created by Peter on 01/05/2022.
//

#include "Level.h"

#include <iostream>
#include <SFML/Audio.hpp>
#include <cmath>
#include "../map/Tile.h"
#include "../Constants.h"
#include "../Controls.h"
#include "Cursor.h"
#include "CycleListener.h"

namespace padi {

    void handleFrameBegin(std::vector<std::shared_ptr<CycleListener>> &listeners, const std::weak_ptr<Level> &lvl,
                          uint8_t frame) {
        auto i = listeners.begin();
        while (i != listeners.end()) {
            if (!(*i)->onFrameBegin(lvl, frame)) i = listeners.erase(i);
            else ++i;
        }
    }

    void handleFrameEnd(std::vector<std::shared_ptr<CycleListener>> &listeners, const std::weak_ptr<Level> &lvl,
                        uint8_t frame) {
        auto i = listeners.begin();
        while (i != listeners.end()) {
            if (!(*i)->onFrameEnd(lvl, frame)) i = listeners.erase(i);
            else ++i;
        }
    }

    void handleCycleBegin(std::vector<std::shared_ptr<CycleListener>> &listeners, const std::weak_ptr<Level> &lvl) {
        auto i = listeners.begin();
        while (i != listeners.end()) {
            if (!(*i)->onCycleBegin(lvl)) i = listeners.erase(i);
            else ++i;
        }
    }

    void handleCycleEnd(std::vector<std::shared_ptr<CycleListener>> &listeners, const std::weak_ptr<Level> &lvl) {
        auto i = listeners.begin();
        while (i != listeners.end()) {
            if (!(*i)->onCycleEnd(lvl))
                i = listeners.erase(i);
            else
                ++i;
        }
    }

    Level::Level(const sf::Vector2i &area, const sf::Vector2i &tile_size) {
        // resize the vertex array to fit the level size
        m_vbo.setPrimitiveType(sf::PrimitiveType::Quads);
        m_vbo.resize(area.x * area.y * 2);
        m_viewTarget.setSize(0, 0);
    }

    void Level::update(sf::RenderTarget *renderTarget) {
        if (!m_paused) {
            m_cycle.carried_uS += m_cycle.clock.restart().asMicroseconds();
        } else {
            m_cycle.clock.restart();
        }

        while (m_cycle.carried_uS > padi::FrameTime_uS) {
            handleFrameEnd(m_cycleListeners.frameEnd, shared_from_this(), m_cycle.frame);

            m_view.setSize(m_viewTarget.getSize());
            auto delta = m_viewTarget.getCenter() - m_view.getCenter();
            float deltaMag = abs(delta.x) + abs(delta.y);
            if (deltaMag > float(TileSize.x * 8)) {
                sf::Vector2f target = m_viewTarget.getCenter();
                target.x = floor(target.x) + 0.5f;
                target.y = floor(target.y) + 0.5f;
                m_view.setCenter(target);
            } else if (deltaMag > float(TileSize.x * 3)) {
                sf::Vector2f target = m_view.getCenter() + delta * 0.8f * (1 - float(TileSize.x * 3) / deltaMag);
                target.x = floor(target.x) + 0.5f;
                target.y = floor(target.y) + 0.5f;
                m_view.setCenter(target);
            }

            m_cycle.carried_uS -= padi::FrameTime_uS;

            if (m_cycle.frame + 1 == padi::CycleLength_F) {
                handleCycleEnd(m_cycleListeners.cycleEnd, shared_from_this());
                m_cycle.frame = 0;
                handleCycleBegin(m_cycleListeners.cycleBegin, shared_from_this());
            } else {
                ++m_cycle.frame;
            }
            handleFrameBegin(m_cycleListeners.frameBegin, shared_from_this(), m_cycle.frame);
        }
        if (m_viewTarget.getSize().x == 0) {
            sf::Vector2f size{float(renderTarget->getSize().x) / renderTarget->getSize().y * 255, 255};
            m_viewTarget.setSize(size);
            printf("[padi::Level] view Target size initialized to %.f,%.f\n", size.x, size.y);
        }

        renderTarget->setView(m_view);
        if (m_cursor) m_cursor->update(this);
    }

    void Level::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        states.shader = m_apollo.lookupShader("base_shader").get();

        // apply the tileset texture
        states.texture = &m_sprites;

        // draw the vertex array
        target.draw(&m_vbo[0], m_numVerts, sf::PrimitiveType::Quads, states);
        sf::Listener::setPosition(m_view.getCenter().x / TileSize.x, m_view.getCenter().y / TileSize.y, 10);
        sf::Listener::setUpVector(0, 1, 2);
        sf::Listener::setDirection(0, 2, 1);
    }

    bool Level::centerView(const sf::Vector2i &position, bool force) {
        if (!m_viewLocked) {
            m_viewTarget.setCenter(padi::Map::mapTilePosToWorld(position));
            if (force) {
                m_view.setCenter(m_viewTarget.getCenter());
            }
        }
        return m_viewLocked;
    }

    void Level::lockView(bool locked) {
        m_viewLocked = locked;
    }

    bool Level::isViewLocked() const {
        return m_viewLocked;
    }

    Map *Level::getMap() {
        return &m_map;
    }

    void Level::populateVBO() {
        m_numVerts = m_map.populate(m_vbo, 0, m_cycle.frame, m_viewTarget);
    }

    bool Level::addCycleBeginListener(const std::shared_ptr<CycleListener> &listener) {
        auto found = std::find(m_cycleListeners.cycleBegin.begin(), m_cycleListeners.cycleBegin.end(), listener);
        if (found == m_cycleListeners.cycleBegin.end()) {
            m_cycleListeners.cycleBegin.push_back(listener);
            return true;
        }
        return false;
    }

    bool Level::addFrameBeginListener(const std::shared_ptr<CycleListener> &listener) {
        auto found = std::find(m_cycleListeners.frameBegin.begin(), m_cycleListeners.frameBegin.end(), listener);
        if (found == m_cycleListeners.frameBegin.end()) {
            m_cycleListeners.frameBegin.push_back(listener);
            return true;
        }
        return false;
    }

    bool Level::addFrameEndListener(const std::shared_ptr<CycleListener> &listener) {
        auto found = std::find(m_cycleListeners.frameEnd.begin(), m_cycleListeners.frameEnd.end(), listener);
        if (found == m_cycleListeners.frameEnd.end()) {
            m_cycleListeners.frameEnd.push_back(listener);
            return true;
        }
        return false;
    }

    bool Level::addCycleEndListener(const std::shared_ptr<CycleListener> &listener) {
        auto found = std::find(m_cycleListeners.cycleEnd.begin(), m_cycleListeners.cycleEnd.end(), listener);
        if (found == m_cycleListeners.cycleEnd.end()) {
            m_cycleListeners.cycleEnd.push_back(listener);
            return true;
        }
        return false;
    }

    const Apollo *Level::getApollo() const {
        return &m_apollo;
    }

    size_t Level::getVBOCapacity() const {
        return m_vbo.getVertexCount();
    }

    uint8_t Level::getCurrentCycleFrame() const {
        return m_cycle.frame;
    }

    void Level::moveCursor(sf::Vector2i const &pos) {
        m_map.moveEntity(m_cursor, pos, ~0u);
        m_cursor->moved(shared_from_this());
    }

    sf::Vector2i Level::getCursorLocation() const {
        return m_cursor->getPosition();
    }

    void Level::hideCursor() {
        m_map.removeEntity(m_cursor);
        m_cursor->lock();
    }

    void Level::showCursor() {
        // Move is more robust here because I'm lazy
        m_map.moveEntity(m_cursor, m_cursor->getPosition());
        m_cursor->unlock();
    }

    void Level::initCursor(std::string const &key) {
        m_cursor = std::make_shared<padi::Cursor>(m_apollo.lookupAnim(key));
        m_cursor->lock();
        addFrameBeginListener(m_cursor);
    }

    std::shared_ptr<padi::Cursor> Level::getCursor() const {
        return m_cursor;
    }

    bool Level::isPaused() const {
        return m_paused;
    }

    void Level::play() {
        m_paused = false;
    }

    void Level::pause() {
        m_paused = true;
    }

    bool Level::togglePause() {
        m_paused = !m_paused;
        return m_paused;
    }

    bool Level::popMobSpawnPosition(sf::Vector2i &pos, uint8_t& seed) {
        if(!m_mobSpawns.empty()) {
            pos = m_mobSpawns.front().first;
            seed = m_mobSpawns.front().second;
            m_mobSpawns.pop();
            return true;
        }
        return false;
    }

} // padi