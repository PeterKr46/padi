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
            if (padi::Controls::wasKeyPressed(sf::Keyboard::Left)) {
                level->moveCursor(getPosition() + padi::Left + padi::Down);
            } else if (padi::Controls::wasKeyPressed(sf::Keyboard::Right)) {
                level->moveCursor(getPosition() + padi::Right + padi::Up);
            } else if (padi::Controls::wasKeyPressed(sf::Keyboard::Up)) {
                auto up = (abs(getPosition().x) + abs(getPosition().y)) % 2 == 0 ? padi::Left : padi::Up;
                level->moveCursor(getPosition() + up);
            } else if (padi::Controls::wasKeyPressed(sf::Keyboard::Down)) {
                auto down = (abs(getPosition().x) + abs(getPosition().y)) % 2 == 1 ? padi::Right : padi::Down;
                level->moveCursor(getPosition() + down);
            }
            if (padi::Controls::isAnyKeyPressed<sf::Keyboard::Key *>(&arrows[0], &arrows[4])) {
                m_color = sf::Color::Yellow;
                level->centerView(getPosition());
            } else {
                m_color = sf::Color::White;
            }
        }
    }

    Cursor::Cursor(std::shared_ptr<padi::Animation> anim) : padi::StaticEntity({0, 0}, EntityType) {
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