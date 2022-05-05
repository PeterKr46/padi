//
// Created by Peter on 01/05/2022.
//

#include <iostream>
#include "Level.h"
#include "../Constants.h"
#include "../Controls.h"

namespace padi {

    void handleFrameBegin(std::vector<std::shared_ptr<CycleListener>> &listeners, Level* lvl, uint8_t frame) {
        auto i = listeners.begin();
        while (i != listeners.end()) {
            if(!(*i)->onFrameBegin(lvl, frame)) i = listeners.erase(i);
            else ++i;
        }
    }

    void handleFrameEnd(std::vector<std::shared_ptr<CycleListener>> &listeners, Level* lvl, uint8_t frame) {
        auto i = listeners.begin();
        while (i != listeners.end()) {
            if(!(*i)->onFrameEnd(lvl, frame)) i = listeners.erase(i);
            else ++i;
        }
    }

    void handleCycleBegin(std::vector<std::shared_ptr<CycleListener>> &listeners, Level* lvl) {
        auto i = listeners.begin();
        while (i != listeners.end()) {
            if(!(*i)->onCycleBegin(lvl)) i = listeners.erase(i);
            else ++i;
        }
    }

    void handleCycleEnd(std::vector<std::shared_ptr<CycleListener>> &listeners, Level* lvl) {
        auto i = listeners.begin();
        while (i != listeners.end()) {
            if(!(*i)->onCycleEnd(lvl))
                i = listeners.erase(i);
            else
                ++i;
        }
    }

    Level::Level(const sf::Vector2i &area, const sf::Vector2i &tile_size) {
        // resize the vertex array to fit the level size
        m_vbo.setPrimitiveType(sf::PrimitiveType::Quads);
        m_vbo.resize(area.x * area.y * 2);
        m_viewTarget.setSize(0,0);
    }

    void Level::update(sf::RenderWindow *window) {
        m_cycle.carried_uS += m_cycle.clock.restart().asMicroseconds();
        while (m_cycle.carried_uS > padi::FrameTime_uS) {
            handleFrameEnd(m_cycleListeners.frameEnd, this, m_cycle.frame);

            m_view.setSize(m_viewTarget.getSize());//(m_viewTarget.getSize() * 0.3f + 0.7f * m_view.getSize()));
            m_view.setCenter(m_viewTarget.getCenter());//(m_viewTarget.getCenter()  * 0.3f + 0.7f * m_view.getCenter()));

            m_cycle.carried_uS -= padi::FrameTime_uS;

            if(m_cycle.frame + 1 == padi::CycleLength_F) {
                handleCycleEnd(m_cycleListeners.cycleEnd, this);
                m_cycle.frame = 0;
                handleCycleBegin(m_cycleListeners.cycleBegin, this);
            } else {
                ++m_cycle.frame;
            }
            handleFrameBegin(m_cycleListeners.frameBegin, this, m_cycle.frame);
        }
        if(m_viewTarget.getSize().x == 0) {
            m_viewTarget.setSize(window->getSize().x, window->getSize().y);
        }

        // Zoom Hotkeys
        if (padi::Controls::isKeyDown(sf::Keyboard::Comma)) {
            m_viewTarget.setSize(m_viewTarget.getSize().x / 2, m_viewTarget.getSize().y / 2);
        } else if (padi::Controls::isKeyDown(sf::Keyboard::Period)) {
            m_viewTarget.setSize(m_viewTarget.getSize().x * 2, m_viewTarget.getSize().y * 2);
        }

        window->setView(m_view);
    }

    void Level::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        //states.transform *= getTransform();

        // apply the tileset texture
        states.texture = &m_sprites;

        // draw the vertex array
        target.draw(&m_vbo[0], m_numVerts, sf::PrimitiveType::Quads, states);
    }

    bool Level::centerView(const sf::Vector2i &position) {
        if (!m_viewLocked) {
            m_viewTarget.setCenter(m_map.mapTilePosToWorld(position));
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
        sf::Clock drawing;
        m_numVerts = m_map.populate(m_vbo, 0, m_cycle.frame);
        std::cout << "DRAW " << 1.f / drawing.restart().asSeconds() << std::endl;
    }

    bool Level::addCycleBeginListener(const std::shared_ptr<CycleListener> &listener) {
        auto found = std::find(m_cycleListeners.cycleBegin.begin(), m_cycleListeners.cycleBegin.end(), listener);
        if(found == m_cycleListeners.cycleBegin.end()) {
            m_cycleListeners.cycleBegin.push_back(listener);
            return true;
        }
        return false;
    }

    bool Level::addFrameBeginListener(const std::shared_ptr<CycleListener> &listener) {
        auto found = std::find(m_cycleListeners.frameBegin.begin(), m_cycleListeners.frameBegin.end(), listener);
        if(found == m_cycleListeners.frameBegin.end()) {
            m_cycleListeners.frameBegin.push_back(listener);
            return true;
        }
        return false;
    }

    bool Level::addFrameEndListener(const std::shared_ptr<CycleListener> &listener) {
        auto found = std::find(m_cycleListeners.frameEnd.begin(), m_cycleListeners.frameEnd.end(), listener);
        if(found == m_cycleListeners.frameEnd.end()) {
            m_cycleListeners.frameEnd.push_back(listener);
            return true;
        }
        return false;
    }

    bool Level::addCycleEndListener(const std::shared_ptr<CycleListener> &listener) {
        auto found = std::find(m_cycleListeners.cycleEnd.begin(), m_cycleListeners.cycleEnd.end(), listener);
        if(found == m_cycleListeners.cycleEnd.end()) {
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

} // padi