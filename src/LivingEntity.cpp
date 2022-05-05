//
// Created by Peter on 28/04/2022.
//

#include "LivingEntity.h"

padi::LivingEntity::LivingEntity(const sf::Vector2i &pos) : Entity(pos) {

}

void padi::LivingEntity::populate(const padi::Map &context, sf::Vertex *quad) const {
    sf::Vector2f size{getSize()};

    sf::Vector2f anchor = context.mapTilePosToWorld(getPosition());
    quad[0].position = anchor+sf::Vector2f(-size.x / 2,-size.y / 2);
    quad[1].position = anchor+sf::Vector2f(size.x / 2,-size.y / 2);
    quad[2].position = anchor+sf::Vector2f(size.x / 2,size.y / 2);
    quad[3].position = anchor+sf::Vector2f(-size.x / 2,size.y / 2);

    sf::Vector2f texCoordAnchor = m_animation->operator[](context.getCurrentCycleFrames());
    quad[0].texCoords = texCoordAnchor;
    quad[1].texCoords = texCoordAnchor+sf::Vector2f (size.x, 0);
    quad[2].texCoords = texCoordAnchor+sf::Vector2f (size);
    quad[3].texCoords = texCoordAnchor+sf::Vector2f(0, size.y);
    for(int i = 0; i < 4; ++i) quad[i].color = m_color;
}

bool padi::LivingEntity::move(Map &map, const sf::Vector2i &dir) {
    if(!m_slaves.empty()) return false;
    m_slaves.emplace_back(getPosition()+dir);
    m_slaves.front().m_animation = m_slaveAnimation;
    m_slaves.front().m_color = m_color;
    map.addEntity(&m_slaves.front());
    return true;
}

void padi::LivingEntity::setAnimation(padi::Animation *anim) {
    m_animation = anim;
}

void padi::LivingEntity::setSlaveAnimation(padi::Animation *anim) {
    m_slaveAnimation = anim;
}

sf::Vector2i padi::LivingEntity::getSize() const {
    return m_animation->getResolution();
}

void padi::LivingEntity::setColor(sf::Color const& c) {
    m_color = c;
}

padi::SlaveEntity::SlaveEntity(const sf::Vector2i &pos) : Entity(pos) {

}

void padi::SlaveEntity::populate(const padi::Map &context, sf::Vertex *quad) const {
    sf::Vector2f size{getSize()};

    sf::Vector2f anchor = context.mapTilePosToWorld(getPosition());
    quad[0].position = anchor+sf::Vector2f(-size.x / 2,-size.y / 2);
    quad[1].position = anchor+sf::Vector2f(size.x / 2,-size.y / 2);
    quad[2].position = anchor+sf::Vector2f(size.x / 2,size.y / 2);
    quad[3].position = anchor+sf::Vector2f(-size.x / 2,size.y / 2);

    sf::Vector2f texCoordAnchor = m_animation->operator[](context.getCurrentCycleFrames());
    quad[0].texCoords = texCoordAnchor;
    quad[1].texCoords = texCoordAnchor+sf::Vector2f (size.x, 0);
    quad[2].texCoords = texCoordAnchor+sf::Vector2f (size);
    quad[3].texCoords = texCoordAnchor+sf::Vector2f(0, size.y);

    for(int i = 0; i < 4; ++i) quad[i].color = m_color;
}

sf::Vector2i padi::SlaveEntity::getSize() const {
    return m_animation->getResolution();
}
