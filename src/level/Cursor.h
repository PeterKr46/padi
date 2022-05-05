//
// Created by Peter on 01/05/2022.
//
#pragma once

#include "Level.h"
#include "SFML/Window/Keyboard.hpp"
#include "../entity/LivingEntity.h"

namespace padi {

    class Cursor
            : public padi::CycleListener {
    public:
        explicit Cursor(std::shared_ptr<padi::Animation> anim);

        void update(padi::Level* level);

        void moveTo(sf::Vector2i const& pos);
        [[nodiscard]] sf::Vector2i getPosition() const;

    private:
        bool m_locked{false};
        std::shared_ptr<padi::StaticEntity> m_entity;
    };

} // padi
