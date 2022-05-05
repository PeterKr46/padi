//
// Created by Peter on 02/05/2022.
//

#include "Button.h"
#include "../Controls.h"
#include <utility>
#include <iostream>

namespace padi {

    Button::Button(const sf::Vector2i &pos, std::shared_ptr<padi::Animation> sprite)
            : UIObject(pos),
              m_sprites(std::move(sprite)) {}

    size_t Button::populate(const padi::Map *map, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) {
        sf::Vector2f size{m_sprites->getResolution()};
        auto pVertex = &array[vertexOffset];

        sf::Vector2f anchor = map->mapTilePosToWorld(getPosition()) + offset;
        float verticalOffset = std::min(float(map->getTileSize().y), size.y) / 2;
        pVertex[0].position = anchor + sf::Vector2f(-size.x / 2, verticalOffset - size.y);
        pVertex[1].position = anchor + sf::Vector2f(size.x / 2, verticalOffset - size.y);
        pVertex[2].position = anchor + sf::Vector2f(size.x / 2, verticalOffset);
        pVertex[3].position = anchor + sf::Vector2f(-size.x / 2, verticalOffset);

        sf::Vector2f texCoordAnchor = (*m_sprites)[m_state];
        pVertex[0].texCoords = texCoordAnchor;
        pVertex[1].texCoords = texCoordAnchor + sf::Vector2f(size.x, 0);
        pVertex[2].texCoords = texCoordAnchor + sf::Vector2f(size);
        pVertex[3].texCoords = texCoordAnchor + sf::Vector2f(0, size.y);

        for (int i = 0; i < 4; ++i) pVertex[i].color = (m_state == 0 ? color.inactive : (m_state > 1 ? color.pressed : color.active)); // TODO clean up

        return 4;
    }

    void Button::update() {
        if(!active) m_state = 0;
        else {
            m_state = std::min(3,padi::Controls::pollKeyState(sf::Keyboard::Space) + 1);
        }

        if(m_state == 2) {
            auto iter = m_listeners.begin();
            while(iter != m_listeners.end()) {
                if(!(*iter)->onRelease(shared_from_this())) {
                    iter = m_listeners.erase(iter);
                } else {
                    ++iter;
                }
            }
        }
    }

} // padi