//
// Created by Peter on 28/04/2022.
//

#include "LivingEntity.h"
#include "Map.h"
#include "Stage.h"

padi::LivingEntity::LivingEntity(const sf::Vector2i &pos) : Entity(pos) {

}

void padi::LivingEntity::populate(const Map *map, sf::Vertex *pVertex) const {
    sf::Vector2f size{getSize()};

    sf::Vector2f anchor = map->mapTilePosToWorld(getPosition());
    pVertex[0].position = anchor + sf::Vector2f(-size.x / 2, -size.y / 2);
    pVertex[1].position = anchor + sf::Vector2f(size.x / 2, -size.y / 2);
    pVertex[2].position = anchor + sf::Vector2f(size.x / 2, size.y / 2);
    pVertex[3].position = anchor + sf::Vector2f(-size.x / 2, size.y / 2);

    sf::Vector2f texCoordAnchor = m_animation->operator[](map->getCurrentCycleFrames());
    pVertex[0].texCoords = texCoordAnchor;
    pVertex[1].texCoords = texCoordAnchor + sf::Vector2f(size.x, 0);
    pVertex[2].texCoords = texCoordAnchor + sf::Vector2f(size);
    pVertex[3].texCoords = texCoordAnchor + sf::Vector2f(0, size.y);
    for (int i = 0; i < 4; ++i) pVertex[i].color = m_color;
}


bool padi::LivingEntity::move(Stage *stage, const sf::Vector2i &dir) {
    if (!m_slaves.empty()) return false;
    m_slaves.emplace_back(getPosition() + dir);
    m_slaves.front().m_animation = m_slaveAnimation;
    m_slaves.front().m_color = m_color;
    stage->getMap()->addEntity(&m_slaves.front());
    auto col = m_color;
    padi::FrameListener f = [col](padi::Stage *stage) { std::cout << stage->getMap()->getCurrentCycleFrames() << ": " << int(col.r) << "," << int(col.g) << "," << int(col.b) << std::endl; return false; };
    stage->addFrameListener(f);
    return true;
}

void padi::LivingEntity::setAnimation(padi::Animation const *anim) {
    m_animation = anim;
}

void padi::LivingEntity::setSlaveAnimation(padi::Animation const *anim) {
    m_slaveAnimation = anim;
}

sf::Vector2i padi::LivingEntity::getSize() const {
    return m_animation->getResolution();
}

void padi::LivingEntity::setColor(sf::Color const &c) {
    m_color = c;
}

padi::SlaveEntity::SlaveEntity(const sf::Vector2i &pos) : Entity(pos) {

}

void padi::SlaveEntity::populate(const Map *map, sf::Vertex *pVertex) const {
    sf::Vector2f size{getSize()};

    sf::Vector2f anchor = map->mapTilePosToWorld(getPosition());
    pVertex[0].position = anchor + sf::Vector2f(-size.x / 2, -size.y / 2);
    pVertex[1].position = anchor + sf::Vector2f(size.x / 2, -size.y / 2);
    pVertex[2].position = anchor + sf::Vector2f(size.x / 2, size.y / 2);
    pVertex[3].position = anchor + sf::Vector2f(-size.x / 2, size.y / 2);

    sf::Vector2f texCoordAnchor = m_animation->operator[](map->getCurrentCycleFrames());
    pVertex[0].texCoords = texCoordAnchor;
    pVertex[1].texCoords = texCoordAnchor + sf::Vector2f(size.x, 0);
    pVertex[2].texCoords = texCoordAnchor + sf::Vector2f(size);
    pVertex[3].texCoords = texCoordAnchor + sf::Vector2f(0, size.y);

    for (int i = 0; i < 4; ++i) pVertex[i].color = m_color;
}

sf::Vector2i padi::SlaveEntity::getSize() const {
    return m_animation->getResolution();
}
