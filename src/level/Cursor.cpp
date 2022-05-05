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
            if (padi::Controls::isKeyDown(sf::Keyboard::Left)) {
                level->getMap()->moveEntity(m_entity, m_entity->getPosition() + padi::Left + padi::Down);
            } else if (padi::Controls::isKeyDown(sf::Keyboard::Right)) {
                level->getMap()->moveEntity(m_entity, m_entity->getPosition() + padi::Right + padi::Up);
            } else if (padi::Controls::isKeyDown(sf::Keyboard::Up)) {
                auto up = (abs(m_entity->getPosition().x) + abs(m_entity->getPosition().y)) % 2 == 0 ? padi::Left : padi::Up;
                level->getMap()->moveEntity(m_entity, m_entity->getPosition() + up);
            } else if (padi::Controls::isKeyDown(sf::Keyboard::Down)) {
                auto down = (abs(m_entity->getPosition().x) + abs(m_entity->getPosition().y)) % 2 == 1 ? padi::Right : padi::Down;
                level->getMap()->moveEntity(m_entity, m_entity->getPosition() + down);
            }
            if (padi::Controls::isAnyKeyPressed<sf::Keyboard::Key *>(&arrows[0], &arrows[4])) {
                m_entity->m_color = sf::Color::Yellow;
            } else {
                m_entity->m_color = sf::Color::White;
            }
        }
    }

    Cursor::Cursor(std::shared_ptr<padi::Animation> anim) {
        m_entity = std::make_shared<padi::StaticEntity>(sf::Vector2i{0, 0});
        m_entity->m_animation = std::move(anim);
    }

    void Cursor::moveTo(sf::Vector2i const &pos) {

    }

    sf::Vector2i Cursor::getPosition() const {
        return m_entity->getPosition();
    }
}