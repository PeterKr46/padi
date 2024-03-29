//
// Created by Peter on 01/05/2022.
//
#include "Cursor.h"

#include <utility>
#include "../Controls.h"
#include "../Constants.h"
#include "../map/Tile.h"
#include "../media/AudioPlayback.h"

namespace padi {

    sf::Keyboard::Key arrows[4]{
            sf::Keyboard::Left,
            sf::Keyboard::Right,
            sf::Keyboard::Up,
            sf::Keyboard::Down};

    void padi::Cursor::update(padi::Level *level) {
        if (!m_locked) {
            auto map = level->getMap();
            if (padi::Controls::wasKeyPressed(sf::Keyboard::Left)) {
                static const sf::Vector2i lefts[3] {padi::Left + padi::Down, padi::Left, padi::Down};
                auto left = 0;
                if(!map->getTile(getPosition() + lefts[left])) {
                    left = (left + 1) % 3;
                }
                if(!map->getTile(getPosition() + lefts[left])) {
                    left = (left + 1) % 3;
                }
                level->moveCursor(getPosition() + lefts[left]);
            } else if (padi::Controls::wasKeyPressed(sf::Keyboard::Right)) {
                static const sf::Vector2i rights[3] {padi::Right + padi::Up, padi::Right, padi::Up};
                auto right = 0;
                if(!map->getTile(getPosition() + rights[right])) {
                    right = (right + 1) % 3;
                }
                if(!map->getTile(getPosition() + rights[right])) {
                    right = (right + 1) % 3;
                }
                level->moveCursor(getPosition() + rights[right]);
            } else if (padi::Controls::wasKeyPressed(sf::Keyboard::Up)) {
                static const sf::Vector2i ups[2] {padi::Left, padi::Up};
                auto up = (abs(getPosition().x) + abs(getPosition().y)) % 2 == 0 ? 0 : 1;
                if(!map->getTile(getPosition() + ups[up])) {
                    up = (up + 1) % 2;
                }
                level->moveCursor(getPosition() + ups[up]);
            } else if (padi::Controls::wasKeyPressed(sf::Keyboard::Down)) {
                static const sf::Vector2i downs[2] {padi::Right, padi::Down};
                auto down = (abs(getPosition().x) + abs(getPosition().y)) % 2 == 0 ? 0 : 1;
                if(!map->getTile(getPosition() + downs[down])) {
                    down = (down + 1) % 2;
                }
                level->moveCursor(getPosition() + downs[down]);
            }
            if (padi::Controls::wasAnyKeyPressed<sf::Keyboard::Key *>(&arrows[0], &arrows[4])) {
                m_color = sf::Color::Yellow;
                level->centerView(getPosition());
            } else {
                m_color = sf::Color::White;
            }
        }
    }

    Cursor::Cursor(std::shared_ptr<padi::Animation> anim) : padi::StaticEntity({0, 0}, CURSOR) {
        m_animation = std::move(anim);
    }

    void Cursor::lock() {
        m_locked = true;
    }

    void Cursor::unlock() {
        m_locked = false;
    }

    bool Cursor::onFrameBegin(std::weak_ptr<padi::Level> const &level, uint8_t frame) {
        //auto tile = level->getMap()->getTile(getPosition());
        //tile->setVerticalOffset((0.5f + 0.5f * sin(float(frame) * (2 * 3.141f / CycleLength_F)) * 1.2f));
        return false;
    }

    void Cursor::moved(const std::weak_ptr<padi::Level> &level) {
        if(!m_beep) {
            m_beep = std::make_shared<padi::AudioPlayback>(level.lock()->getApollo()->lookupAudio("deep_beep"));
        }
        //m_beep->setPosition(getPosition());
        m_beep->restart(level);
    }
}