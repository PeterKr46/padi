//
// Created by Peter on 01/05/2022.
//
#include "Cursor.h"

#include <utility>
#include "../Controls.h"
#include "../Constants.h"

namespace padi {

    sf::Keyboard::Key arrows[4]{
            sf::Keyboard::Left,
            sf::Keyboard::Right,
            sf::Keyboard::Up,
            sf::Keyboard::Down};

    void padi::Cursor::update(padi::Level *level) {
        if (!m_locked) {
            if (padi::Controls::wasKeyPressed(sf::Keyboard::Left)) {
                level->getMap()->moveEntity(shared_from_this(), getPosition() + padi::Left + padi::Down);
            } else if (padi::Controls::wasKeyPressed(sf::Keyboard::Right)) {
                level->getMap()->moveEntity(shared_from_this(), getPosition() + padi::Right + padi::Up);
            } else if (padi::Controls::wasKeyPressed(sf::Keyboard::Up)) {
                auto up = (abs(getPosition().x) + abs(getPosition().y)) % 2 == 0 ? padi::Left : padi::Up;
                level->getMap()->moveEntity(shared_from_this(), getPosition() + up);
            } else if (padi::Controls::wasKeyPressed(sf::Keyboard::Down)) {
                auto down = (abs(getPosition().x) + abs(getPosition().y)) % 2 == 1 ? padi::Right : padi::Down;
                level->getMap()->moveEntity(shared_from_this(), getPosition() + down);
            }
            if (padi::Controls::isAnyKeyPressed<sf::Keyboard::Key *>(&arrows[0], &arrows[4])) {
                m_color = sf::Color::Yellow;
            } else {
                m_color = sf::Color::White;
            }
        }
    }

    Cursor::Cursor(std::shared_ptr<padi::Animation> anim) : padi::StaticEntity({0,0}) {
        m_animation = std::move(anim);
    }

    void Cursor::lock() {
        m_locked = true;
    }

    void Cursor::unlock() {
        m_locked = false;
    }
}