//
// Created by Peter on 01/05/2022.
//
#pragma once

#include "../map/Map.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include "../media/Apollo.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>

namespace padi {

    class CycleListener;

    class LevelGenerator;

    class Level
            : public sf::Drawable {
        friend class LevelGenerator;

    public:
        explicit Level(sf::Vector2i const &area, sf::Vector2i const &tile_size = {32, 32});

        void update(sf::RenderWindow *window);

        void populateVBO();

        void lockView(bool locked);

        bool isViewLocked() const;

        bool centerView(sf::Vector2i const &position);

        padi::Map *getMap();

        bool addCycleBeginListener(std::shared_ptr<CycleListener> const &listener);

        bool addFrameBeginListener(std::shared_ptr<CycleListener> const &listener);

        bool addFrameEndListener(std::shared_ptr<CycleListener> const &listener);

        bool addCycleEndListener(std::shared_ptr<CycleListener> const &listener);

        const Apollo* getApollo() const;

        size_t getVBOCapacity() const;

    private:

    protected:
        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

    private:
        sf::View m_view;
        sf::View m_viewTarget;
        bool m_viewLocked{false};

        // Sprite master - anything drawn in the context of this level should be on here.
        sf::Texture m_sprites;
        padi::Apollo m_apollo;
        sf::VertexArray m_vbo;

        struct {
            sf::Clock clock;
            uint64_t carried_uS{0};
            uint8_t frame{0};
        } m_cycle;

        struct {
            std::vector<std::shared_ptr<CycleListener>> cycleBegin;
            std::vector<std::shared_ptr<CycleListener>> cycleEnd;
            std::vector<std::shared_ptr<CycleListener>> frameBegin;
            std::vector<std::shared_ptr<CycleListener>> frameEnd;
        } m_cycleListeners;

        padi::Map m_map;
        size_t m_numVerts{0}; // TODO hm
    };

    class CycleListener {
    public:
        virtual bool onCycleBegin(Level *) { return false; }

        virtual bool onCycleEnd(Level *) { return false; }

        virtual bool onFrameBegin(Level *, uint8_t frame) { return false; }

        virtual bool onFrameEnd(Level *, uint8_t frame) { return false; }
    };

} // padi
