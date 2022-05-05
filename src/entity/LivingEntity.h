//
// Created by Peter on 28/04/2022.
//
#pragma once

#include "../animation/Animation.h"
#include "Entity.h"
#include "../animation/Apollo.h"
#include "../level/Level.h"

namespace padi {

    class Map;

    class SlaveEntity
            : public padi::Entity {
    public:
        explicit SlaveEntity(const sf::Vector2i &pos);

        size_t populate(padi::Map const* map, sf::VertexArray & array, size_t vertexOffset, uint8_t frame) const override;

        [[nodiscard]] sf::Vector2i getSize() const override;

        sf::Color m_color{255, 255, 255};
        std::shared_ptr<padi::Animation> m_animation;
    };

    class LivingEntity
            : public padi::Entity
            , public std::enable_shared_from_this<LivingEntity>
            , public CycleListener {

    public:
        explicit LivingEntity(padi::AnimationSet const* moveset, const sf::Vector2i &pos);

        [[nodiscard]] sf::Vector2i getSize() const override;

        size_t populate(padi::Map const* map, sf::VertexArray & array, size_t vertexOffset, uint8_t frame) const override;

        bool intentMove(sf::Vector2i const &dir);
        void intentStay();

        bool onCycleBegin(Level *) override;
        bool onCycleEnd(Level *) override;

        void setColor(sf::Color const &color);
        sf::Color getColor() const;

    private:

        padi::AnimationSet const*  m_apolloCtx;
        std::vector<std::shared_ptr<padi::SlaveEntity>> m_slaves;

        struct {
            bool move{false};
        } m_inAction;

        struct {
            bool            move{false};
            sf::Vector2i    move_dir{0, 0};
        } m_intent;

        sf::Color m_color{168, 255, 168};
        std::shared_ptr<padi::Animation> m_animation;
    };
}
