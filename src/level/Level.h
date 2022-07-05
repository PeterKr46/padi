//
// Created by Peter on 01/05/2022.
//
#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include "../map/Map.h"
#include "../media/Apollo.h"
#include "CycleListener.h"

namespace padi {

    class LevelGenerator;

    class Cursor;

    class Level
            : public sf::Drawable, public std::enable_shared_from_this<Level> {
        friend class LevelGenerator;

    public:
        explicit Level(sf::Vector2i const &area, sf::Vector2i const &tile_size = {32, 32});

        void update(sf::RenderTarget *renderTarget);

        void populateVBO();

        void lockView(bool locked);

        bool isViewLocked() const;

        bool centerView(sf::Vector2i const &position, bool force = false);

        padi::Map *getMap();

        uint8_t getCurrentCycleFrame() const;

        bool addCycleBeginListener(std::shared_ptr<CycleListener> const &listener);

        bool addFrameBeginListener(std::shared_ptr<CycleListener> const &listener);

        bool addFrameEndListener(std::shared_ptr<CycleListener> const &listener);

        bool addCycleEndListener(std::shared_ptr<CycleListener> const &listener);

        const Apollo *getApollo() const;

        size_t getVBOCapacity() const;

        void initCursor(std::string const &anim);

        sf::Vector2i getCursorLocation() const;

        void moveCursor(sf::Vector2i const &pos);

        void hideCursor();

        void showCursor();

        void pause();

        void play();

        bool togglePause();

        bool isPaused() const;

        std::shared_ptr<padi::Cursor> getCursor() const;

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

        bool popMobSpawnPosition(sf::Vector2i &pos, uint8_t &seed);

    private:
        sf::View m_view;
        sf::View m_viewTarget;
        bool m_viewLocked{false};

        // Sprite master - anything drawn in the context of this level should be on here.
        sf::Texture m_sprites;
        padi::Apollo m_apollo;
        sf::VertexArray m_vbo;
        size_t m_numVerts{0}; // TODO hm

        struct {
            sf::Clock clock;
            uint64_t carried_uS{0};
            uint8_t frame{0};
        } m_cycle;
        bool m_paused{false};

        struct {
            std::vector<std::shared_ptr<CycleListener>> cycleBegin;
            std::vector<std::shared_ptr<CycleListener>> cycleEnd;
            std::vector<std::shared_ptr<CycleListener>> frameBegin;
            std::vector<std::shared_ptr<CycleListener>> frameEnd;
        } m_cycleListeners;

        padi::Map m_map;
        std::shared_ptr<padi::Cursor> m_cursor;
        std::queue<std::pair<sf::Vector2i, uint8_t>> m_mobSpawns;
    };

} // padi
