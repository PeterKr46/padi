//
// Created by Peter on 28/04/2022.
//
#pragma once

#include "../media/Animation.h"
#include "Entity.h"
#include "../media/Apollo.h"
#include "../level/Level.h"
#include "StaticEntity.h"
#include "../level/CycleListener.h"

namespace padi {

    class Map;

    class Ability;

    class LivingEntity
            : public padi::Entity
            , public std::enable_shared_from_this<LivingEntity>
            , public CycleListener {

    public:
        explicit LivingEntity(std::string  name, padi::AnimationSet const* moveset, const sf::Vector2i &pos);

        [[nodiscard]] sf::Vector2i getSize() const override;

        size_t populate(padi::Map const* map, sf::VertexArray & array, size_t vertexOffset, uint8_t frame) const override;

        bool intentMove(sf::Vector2i const &dir);
        void intentStay();
        void intentCast(std::shared_ptr<padi::Ability> const &ability, sf::Vector2i const &position);

        bool isMoving() const;
        sf::Vector2i currentMoveDirection() const;
        bool hasMoveIntent() const;
        bool hasCastIntent() const;
        bool hasFailedCast() const;
        bool isCasting() const;

        padi::AnimationSet const* getAnimationSet() const;
        void trySetAnimation(std::string const& anim);

        bool onCycleBegin(Level *) override;
        bool onCycleEnd(Level *) override;

        void setColor(sf::Color const &color);
        sf::Color getColor() const;

    private:

        padi::AnimationSet const*  m_apolloCtx;
        std::vector<std::shared_ptr<padi::StaticEntity>> m_slaves;

        struct {
            bool move{false};
            bool cast{false};
            bool cast_failed{false};
        } m_inAction;

        struct {
            bool            move{false};
            sf::Vector2i    move_dir{0, 0};
            bool            cast{false};
            sf::Vector2i    cast_pos{0, 0};
            std::shared_ptr<padi::Ability> cast_ability{nullptr};
        } m_intent;

        sf::Color m_color{255, 255, 255};
        std::shared_ptr<padi::Animation> m_animation;
        const std::string m_name;
    };
}
