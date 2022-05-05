//
// Created by Peter on 28/04/2022.
//

#include "LivingEntity.h"

#include <utility>
#include "Map.h"
#include "Stage.h"

padi::LivingEntity::LivingEntity(padi::AnimationSet const* moveset, const sf::Vector2i &pos) : Entity(pos), m_moveset(moveset) {
    m_animation = m_moveset->at("idle");
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

std::pair<std::shared_ptr<padi::Animation>,std::shared_ptr<padi::Animation>> determineAnims(padi::AnimationSet const* context, sf::Vector2i const& dir) {
    std::pair<std::shared_ptr<padi::Animation>,std::shared_ptr<padi::Animation>> result{nullptr,nullptr};
    if(dir.x > 0) {
        result.first    = context->at("move_x_from");
        result.second   = context->at("move_x_to");
    }
    else if(dir.x < 0) {
        result.second   = std::make_shared<padi::ReverseAnimation>(context->at("move_x_from"));
        result.first    = std::make_shared<padi::ReverseAnimation>(context->at("move_x_to"));
    }
    else if(dir.y > 0) {
        result.first    = context->at("move_y_from");
        result.second   = context->at("move_y_to");
    }
    else if(dir.y < 0) {
        result.second   = std::make_shared<padi::ReverseAnimation>(context->at("move_y_from"));
        result.first    = std::make_shared<padi::ReverseAnimation>(context->at("move_y_to"));
    }
    else {
        result.first = context->at("idle");
        result.second = context->at("idle");
    }
    return result;
}

bool padi::LivingEntity::move(Stage *stage, const sf::Vector2i &dir) {
    if (!m_slaves.empty() || stage->getMap()->getCurrentCycleFrames() != 0) return false;
    m_slaves.push_back(std::make_shared<padi::SlaveEntity>(getPosition() + dir));
    auto anims = determineAnims(m_moveset, dir);
    m_animation = anims.first;
    m_slaves.front()->m_animation = anims.second;
    m_slaves.front()->m_color = m_color;
    stage->getMap()->addEntity(m_slaves.front());
    m_frameListener = [this](padi::Stage *stage) {
        if(stage->getMap()->getCurrentCycleFrames() == 11) {
            stage->getMap()->removeEntity(m_slaves.front());
            stage->getMap()->moveEntity(shared_from_this(), m_slaves.front()->getPosition());
            m_animation = m_moveset->at("idle");
            m_slaves.clear();
            return false;
        }
        return true;
    };
    stage->addFrameListener(&m_frameListener);
    return true;
}

sf::Vector2i padi::LivingEntity::getSize() const {
    return m_animation->getResolution();
}

void padi::LivingEntity::setColor(sf::Color const &c) {
    m_color = c;
}

sf::Color padi::LivingEntity::getColor() const {
    return m_color;
}

padi::SlaveEntity::SlaveEntity(const sf::Vector2i &pos) : Entity(pos) {

}

void padi::SlaveEntity::populate(const Map *map, sf::Vertex *pVertex) const {
    sf::Vector2f size{getSize()};

    sf::Vector2f anchor = map->mapTilePosToWorld(getPosition());
    float verticalOffset = std::min(float(map->getTileSize().y), size.y) / 2;
    pVertex[0].position = anchor + sf::Vector2f(-size.x / 2, -verticalOffset);
    pVertex[1].position = anchor + sf::Vector2f(size.x / 2,  -verticalOffset);
    pVertex[2].position = anchor + sf::Vector2f(size.x / 2,   size.y - verticalOffset);
    pVertex[3].position = anchor + sf::Vector2f(-size.x / 2,  size.y - verticalOffset);

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
