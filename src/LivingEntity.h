//
// Created by Peter on 28/04/2022.
//
#pragma once

#include "Animation.h"
#include "Entity.h"

namespace padi {

    class Map;

    class Stage;

    class SlaveEntity
            : public padi::Entity {
    public:
        explicit SlaveEntity(const sf::Vector2i &pos);

        void populate(const Map *map, sf::Vertex *pVertex) const override;

        sf::Vector2i getSize() const override;

        sf::Color m_color{255, 255, 255};
        std::shared_ptr<padi::Animation> m_animation;
    };

    class LivingEntity
            : public padi::Entity {

    public:
        explicit LivingEntity(const sf::Vector2i &pos);

        // TODO : These are debug functions
        void setAnimation(std::shared_ptr<padi::Animation> anim);

        void setSlaveAnimation(std::shared_ptr<padi::Animation> anim);

        sf::Vector2i getSize() const override;

        void populate(const padi::Map *map, sf::Vertex *pVertex) const override;

        bool move(padi::Stage *map, sf::Vector2i const &dir);

        void setColor(sf::Color const &color);

    private:
        sf::Color m_color{255, 255, 255};
        std::shared_ptr<padi::Animation> m_animation;
        std::shared_ptr<padi::Animation> m_slaveAnimation;
        std::vector<std::shared_ptr<padi::SlaveEntity>> m_slaves;
    };
}
