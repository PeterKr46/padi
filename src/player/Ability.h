//
// Created by Peter on 01/05/2022.
//

#pragma once


#include <memory>
#include <vector>
#include "SFML/System/Vector2.hpp"
#include "../entity/StaticEntity.h"

namespace padi {
    class Level;

    class LivingEntity;

    class Ability {
    public:
        explicit Ability(std::shared_ptr<padi::LivingEntity> user);

        virtual bool cast(padi::Level *level, sf::Vector2i const &pos) = 0;

        virtual void castIndicator(padi::Level *level) = 0;

        virtual void castCancel(padi::Level *level) = 0;

        virtual bool isCastComplete() = 0;

        [[nodiscard]] std::shared_ptr<padi::LivingEntity> getUser() const;

    protected:
        std::shared_ptr<padi::LivingEntity> m_user;
    };

    class LimitedRangeAbility : public Ability {
    public:
        LimitedRangeAbility(std::shared_ptr<padi::LivingEntity> user, size_t range);

        void castIndicator(padi::Level *level) override;

        bool cast(padi::Level *level, const sf::Vector2i &pos) override;

        void castCancel(padi::Level *level) override;

        [[nodiscard]] size_t getRange() const;

        void setRange(size_t range);

        bool isCastComplete() override = 0;

    protected:
        virtual void recalculateRange(Level* level);
        std::vector<sf::Vector2i> m_inRange;
        bool m_rangeChanged{true};
    private:
        std::vector<std::shared_ptr<padi::StaticEntity>> m_rangeIndicators;
        int m_range{2};
    };

} // padi
