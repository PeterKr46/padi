//
// Created by Peter on 02/05/2022.
//

#pragma once

#include "../entity/LivingEntity.h"
#include "SFML/Audio/Sound.hpp"

namespace padi {

    class MapButton;

    class Apollo;

    class MapButtonListener {
    public:
        virtual bool onRelease(std::shared_ptr<padi::MapButton> button) = 0;
    };

    class MapButton : public padi::GridObject, public std::enable_shared_from_this<MapButton> {
    public:
        MapButton(const sf::Vector2i &pos, std::shared_ptr<padi::Animation> sprite, Apollo const *apollo);

        size_t
        populate(const padi::Map *map, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) const override;

        void update();

    public:
        sf::Vector2f offset{0, -8};
        bool active{true};
        struct {
            sf::Color inactive{186, 186, 186};
            sf::Color active{255, 255, 255};
            sf::Color pressed{255, 255, 86};
        } color;
    private:
        sf::Sound m_soundOn;
        sf::Sound m_soundOff;
        std::shared_ptr<padi::Animation> m_sprites;
        uint8_t m_state{0}; // 0 = none, 1 = down, 2 = pressed, 3 = up
        std::vector<std::shared_ptr<MapButtonListener>> m_listeners;
    };

} // padi
