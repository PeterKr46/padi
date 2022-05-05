//
// Created by Peter on 28/04/2022.
//
#pragma once

#include "Animation.h"
#include "Entity.h"
#include "Map.h"

namespace padi {

    class SlaveEntity
            : public padi::Entity {
    public:
        explicit SlaveEntity(const sf::Vector2i &pos);

        void populate(const Map &map, sf::Vertex *pVertex) const override;

        sf::Vector2i getSize() const override;

        sf::Color m_color{255, 255, 255};
        padi::Animation *m_animation{nullptr};
    };

    class LivingEntity
            : public padi::Entity {

    public:
        explicit LivingEntity(const sf::Vector2i &pos);

        // TODO : These are debug functions
        void setAnimation(Animation *anim);

        void setSlaveAnimation(Animation *anim);

        sf::Vector2i getSize() const override;

        void populate(const Map &map, sf::Vertex *pVertex) const override;

        bool move(Map &map, sf::Vector2i const &dir);

        void setColor(sf::Color const& color);

    private:
        sf::Color m_color{255, 255, 255};
        padi::Animation *m_animation{nullptr};
        padi::Animation *m_slaveAnimation{nullptr};
        std::vector<SlaveEntity> m_slaves;
    };
}
