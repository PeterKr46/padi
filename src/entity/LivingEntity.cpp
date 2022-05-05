//
// Created by Peter on 28/04/2022.
//

#include "LivingEntity.h"

#include <utility>
#include <iostream>

padi::LivingEntity::LivingEntity(padi::AnimationSet const* moveset, const sf::Vector2i &pos) : Entity(pos), m_apolloCtx(moveset) {
    m_animation = m_apolloCtx->at("idle");
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

sf::Vector2i padi::LivingEntity::getSize() const {
    return m_animation->getResolution();
}

void padi::LivingEntity::setColor(sf::Color const &c) {
    m_color = c;
}

sf::Color padi::LivingEntity::getColor() const {
    return m_color;
}

size_t padi::LivingEntity::populate(const padi::Map *map, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) const {
    sf::Vector2f size{getSize()};
    auto pVertex = &array[vertexOffset];

    sf::Vector2f anchor = map->mapTilePosToWorld(getPosition());
    float verticalOffset = std::min(float(map->getTileSize().y), size.y) / 2;
    pVertex[0].position = anchor + sf::Vector2f(-size.x / 2, verticalOffset - size.y);
    pVertex[1].position = anchor + sf::Vector2f(size.x / 2,  verticalOffset - size.y);
    pVertex[2].position = anchor + sf::Vector2f(size.x / 2,   verticalOffset);
    pVertex[3].position = anchor + sf::Vector2f(-size.x / 2,  verticalOffset);

    sf::Vector2f texCoordAnchor = (*m_animation)[frame];
    pVertex[0].texCoords = texCoordAnchor;
    pVertex[1].texCoords = texCoordAnchor + sf::Vector2f(size.x, 0);
    pVertex[2].texCoords = texCoordAnchor + sf::Vector2f(size);
    pVertex[3].texCoords = texCoordAnchor + sf::Vector2f(0, size.y);

    for (int i = 0; i < 4; ++i) pVertex[i].color = m_color;

    return 4;
}

bool padi::LivingEntity::onCycleBegin(padi::Level * lvl) {
    if(m_intent.move) {
        std::cout << "Start" << std::endl;
        m_intent.move = false;
        m_inAction.move = true;
        m_slaves.push_back(std::make_shared<padi::SlaveEntity>(getPosition() + m_intent.move_dir));
        auto anims = determineAnims(m_apolloCtx, m_intent.move_dir);
        m_animation = anims.first;
        m_slaves.front()->m_animation = anims.second;
        m_slaves.front()->m_color = m_color;
        lvl->getMap()->addEntity(m_slaves.front());
    }
    return true;
}

bool padi::LivingEntity::onCycleEnd(padi::Level * lvl) {
    if(m_inAction.move) {
        std::cout << "Stop (intent: " << (m_intent.move ? "Go" : "Stay") << ")" << std::endl;
        m_inAction.move = false;
        lvl->getMap()->removeEntity(m_slaves.front());
        lvl->getMap()->moveEntity(shared_from_this(), m_slaves.front()->getPosition());
        m_animation = m_apolloCtx->at("idle");
        m_slaves.clear();
    }
    return true;
}

bool padi::LivingEntity::moveIntent(const sf::Vector2i &dir) {
    if(m_intent.move || abs(dir.x) + abs(dir.y) > 1) {
        return false;
    }
    else {
        m_intent.move = true;
        m_intent.move_dir = dir;
        return true;
    }
}

padi::SlaveEntity::SlaveEntity(const sf::Vector2i &pos) : Entity(pos) {

}

sf::Vector2i padi::SlaveEntity::getSize() const {
    return m_animation->getResolution();
}

size_t padi::SlaveEntity::populate(const padi::Map *map, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) const {
    sf::Vector2f size{getSize()};
    auto pVertex = &array[vertexOffset];

    sf::Vector2f anchor = map->mapTilePosToWorld(getPosition());
    float verticalOffset = std::min(float(map->getTileSize().y), size.y) / 2;
    pVertex[0].position = anchor + sf::Vector2f(-size.x / 2, verticalOffset - size.y);
    pVertex[1].position = anchor + sf::Vector2f(size.x / 2,  verticalOffset - size.y);
    pVertex[2].position = anchor + sf::Vector2f(size.x / 2,   verticalOffset);
    pVertex[3].position = anchor + sf::Vector2f(-size.x / 2,  verticalOffset);

    sf::Vector2f texCoordAnchor = (*m_animation)[frame];
    pVertex[0].texCoords = texCoordAnchor;
    pVertex[1].texCoords = texCoordAnchor + sf::Vector2f(size.x, 0);
    pVertex[2].texCoords = texCoordAnchor + sf::Vector2f(size);
    pVertex[3].texCoords = texCoordAnchor + sf::Vector2f(0, size.y);

    for (int i = 0; i < 4; ++i) pVertex[i].color = m_color;

    return 4;
}
