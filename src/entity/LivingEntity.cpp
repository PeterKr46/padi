//
// Created by Peter on 28/04/2022.
//

#include "LivingEntity.h"

#include <utility>
#include <iostream>
#include "../level/Level.h"
#include "../Constants.h"
#include "Ability.h"
#include "StaticEntity.h"

padi::LivingEntity::LivingEntity(std::string name, padi::AnimationSet const *moveset, const sf::Vector2i &pos)
        : Entity(pos, 1),
          m_name(std::move(name)),
          m_apolloCtx(moveset) {
    m_animation = m_apolloCtx->at("idle");
    m_slaves.push_back(std::make_shared<padi::StaticEntity>(getPosition()));
}

std::pair<std::shared_ptr<padi::Animation>, std::shared_ptr<padi::Animation>>
determineAnims(padi::AnimationSet const *context, sf::Vector2i const &dir) {
    std::pair<std::shared_ptr<padi::Animation>, std::shared_ptr<padi::Animation>> result{nullptr, nullptr};
    if (dir.x > 0) {
        result.first = context->at("move_x_from");
        result.second = context->at("move_x_to");
    } else if (dir.x < 0) {
        result.second = std::make_shared<padi::ReverseAnimation>(context->at("move_x_from"));
        result.first = std::make_shared<padi::ReverseAnimation>(context->at("move_x_to"));
    } else if (dir.y > 0) {
        result.first = context->at("move_y_from");
        result.second = context->at("move_y_to");
    } else if (dir.y < 0) {
        result.second = std::make_shared<padi::ReverseAnimation>(context->at("move_y_from"));
        result.first = std::make_shared<padi::ReverseAnimation>(context->at("move_y_to"));
    } else {
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

size_t
padi::LivingEntity::populate(const padi::Map *map, sf::VertexArray &array, size_t vertexOffset, uint8_t frame, float tileVerticalOffset) const {
    sf::Vector2f size{getSize()};
    auto pVertex = &array[vertexOffset];

    sf::Vector2f anchor = padi::Map::mapTilePosToWorld(getPosition());
    float verticalOffset = tileVerticalOffset + getVerticalOffset() + std::min(float(padi::TileSize.y), size.y) / 2;
    pVertex[0].position = anchor + sf::Vector2f(-size.x / 2, verticalOffset - size.y);
    pVertex[1].position = anchor + sf::Vector2f(size.x / 2, verticalOffset - size.y);
    pVertex[2].position = anchor + sf::Vector2f(size.x / 2, verticalOffset);
    pVertex[3].position = anchor + sf::Vector2f(-size.x / 2, verticalOffset);

    sf::Vector2f texCoordAnchor = (*m_animation)[frame];
    pVertex[0].texCoords = texCoordAnchor;
    pVertex[1].texCoords = texCoordAnchor + sf::Vector2f(size.x, 0);
    pVertex[2].texCoords = texCoordAnchor + sf::Vector2f(size);
    pVertex[3].texCoords = texCoordAnchor + sf::Vector2f(0, size.y);

    for (int i = 0; i < 4; ++i) pVertex[i].color = m_color;

    return 4;
}

bool padi::LivingEntity::onCycleBegin(std::weak_ptr<padi::Level> const &lvl) {
    if (m_intent.move) {
        //std::cout << "[padi::LivingEntity(" << m_name << ")] Moving." << std::endl;
        m_intent.move = false;
        m_inAction.move = true;
        auto anims = determineAnims(m_apolloCtx, m_intent.move_dir);
        m_animation = anims.first;
        m_slaves.front()->m_animation = anims.second;
        m_slaves.front()->m_color = m_color;
        lvl.lock()->getMap()->addEntity(m_slaves.front(), getPosition() + m_intent.move_dir);
    } else if (m_intent.cast) {
        //std::cout << "[padi::LivingEntity(" << m_name << ")] Casting." << std::endl;
        m_intent.cast = false;
        m_animation = m_apolloCtx->at("idle");
        m_inAction.cast = true;
        m_inAction.cast_failed = ! m_intent.cast_ability->cast(lvl, m_intent.cast_pos);
    } else {
        //std::cout << "[padi::LivingEntity(" << m_name << ")] Idle." << std::endl;
        m_animation = m_apolloCtx->at("idle");
    }
    return true;
}

bool padi::LivingEntity::onCycleEnd(std::weak_ptr<padi::Level> const &lvl) {
    if (m_inAction.move) {
        auto level = lvl.lock();
        m_inAction.move = false;
        level->getMap()->removeEntity(m_slaves.front());
        level->getMap()->moveEntity(shared_from_this(), m_slaves.front()->getPosition());
        m_animation = m_apolloCtx->at("idle");
    }
    if (m_inAction.cast || m_inAction.cast_failed) {
        m_inAction.cast = false; // hm
        m_inAction.cast_failed = false; // hm
    }
    return true;
}

bool padi::LivingEntity::intentMove(const sf::Vector2i &dir) {
    if (abs(dir.x) + abs(dir.y) > 1) {
        return false;
    } else {
        m_intent.move = true;
        m_intent.move_dir = dir;
        return true;
    }
}

void padi::LivingEntity::intentStay() {
    m_intent.move = false;
}

void padi::LivingEntity::intentCast(std::shared_ptr<padi::Ability> const &ability, sf::Vector2i const &position) {
    m_intent.cast = true;
    m_intent.cast_ability = ability;
    m_intent.cast_pos = position;
}

bool padi::LivingEntity::isCasting() const {
    return m_inAction.cast;
}

bool padi::LivingEntity::isMoving() const {
    return m_inAction.move;
}

bool padi::LivingEntity::hasMoveIntent() const {
    return m_intent.move;
}

bool padi::LivingEntity::hasCastIntent() const {
    return m_intent.cast;
}

sf::Vector2i padi::LivingEntity::currentMoveDirection() const {
    if (!m_inAction.move) return {0, 0};
    return m_slaves.front()->getPosition() - getPosition();
}

void padi::LivingEntity::trySetAnimation(std::string const &anim) {
    m_animation = m_apolloCtx->at(anim);
}

padi::AnimationSet const *padi::LivingEntity::getAnimationSet() const {
    return m_apolloCtx;
}

bool padi::LivingEntity::hasFailedCast() const {
    return m_inAction.cast_failed;
}

std::string const &padi::LivingEntity::getName() {
    return m_name;
}

