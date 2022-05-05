//
// Created by Peter on 28/04/2022.
//
#pragma once

#include "Animation.h"
#include "Entity.h"
#include "Apollo.h"
#include "Actions.h"

namespace padi {

    class Map;

    class Stage;

    class SlaveEntity
            : public padi::Entity {
    public:
        explicit SlaveEntity(const sf::Vector2i &pos);

        void populate(const Map *map, sf::Vertex *pVertex) const override;

        [[nodiscard]] sf::Vector2i getSize() const override;

        sf::Color m_color{255, 255, 255};
        std::shared_ptr<padi::Animation> m_animation;
    };

    class LivingEntity
            : public padi::Entity
            , public std::enable_shared_from_this<LivingEntity> {

    public:
        explicit LivingEntity(padi::AnimationSet const* moveset, const sf::Vector2i &pos);

        [[nodiscard]] sf::Vector2i getSize() const override;

        void populate(const padi::Map *map, sf::Vertex *pVertex) const override;

        bool move(padi::Stage *map, sf::Vector2i const &dir);

        void setColor(sf::Color const &color);

    private:
        padi::FrameListener m_frameListener;
        padi::AnimationSet const*  m_moveset;
        sf::Color m_color{255, 255, 255};
        std::shared_ptr<padi::Animation> m_animation;
        std::shared_ptr<padi::Animation> m_slaveAnimation;
        std::vector<std::shared_ptr<padi::SlaveEntity>> m_slaves;
    };
}
