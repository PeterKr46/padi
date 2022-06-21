//
// Created by Peter on 28/04/2022.
//
#pragma once

#include "Entity.h"
#include "../level/CycleListener.h"
#include "../media/Apollo.h"

namespace padi {

    class Map;
    class Ability;
    class StaticEntity;

    class HPBar {
    public:
        explicit HPBar(padi::AnimationSet const* sprites, int maxHP);
        size_t populate(sf::VertexArray &array, size_t vertexOffset, float verticalOffset, const std::shared_ptr<const Entity> &entity,
                        sf::Color color = sf::Color::Green) const;

        void setHP(int hp);

        void setMaxHP(int hp);

        [[nodiscard]] int getHP() const;

        [[nodiscard]] int getMaxHP() const;

        float m_verticalOffset = -32;
    private:
        int m_HP, m_maxHP;
        padi::AnimationSet const*  m_apolloCtx;
    };

    class LivingEntity
            : public padi::Entity
            , public std::enable_shared_from_this<LivingEntity>
            , public CycleListener {

    public:
        static const uint32_t EntityType = 2;

        explicit LivingEntity(std::string  name, padi::AnimationSet const* moveset, const sf::Vector2i &pos);

        [[nodiscard]] sf::Vector2i getSize() const override;

        size_t populate(padi::Map const* map, sf::VertexArray & array, size_t vertexOffset, uint8_t frame, float tileVerticalOffset) const override;
        size_t numQuads() const override;

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

        bool onCycleBegin(std::weak_ptr<padi::Level> const &lvl) override;
        bool onCycleEnd(std::weak_ptr<padi::Level> const &lvl) override;

        void setColor(sf::Color const &color);
        sf::Color getColor() const;

        std::string const& getName();

        std::weak_ptr<HPBar> getHPBar();
        bool hasHPBar() const;
        void initHPBar(int maxHP, padi::AnimationSet const *sprites);

    private:

        padi::AnimationSet const*  m_apolloCtx;
        std::vector<std::shared_ptr<padi::StaticEntity>> m_slaves;
        std::shared_ptr<HPBar> m_hp;
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
