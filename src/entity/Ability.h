//
// Created by Peter on 01/05/2022.
//

#pragma once


#include <string>
#include <memory>
#include <vector>
#include "SFML/System/Vector2.hpp"
#include "StaticEntity.h"

namespace padi {
    class Level;

    class LivingEntity;

    class Ability {
    public:
        explicit Ability(std::shared_ptr<padi::LivingEntity> user);

        virtual bool cast(const std::weak_ptr<Level> &level, sf::Vector2i const &pos) = 0;

        virtual void castIndicator(const std::weak_ptr<Level> &level) = 0;

        virtual void castCancel(const std::weak_ptr<Level> &level) = 0;

        virtual bool isCastComplete() = 0;

        [[nodiscard]] virtual std::string const& getDescription() const;
        void setDescription(std::string const& description);

        [[nodiscard]] virtual uint32_t getAbilityType() const = 0;

        virtual void writeProperties(uint8_t* data, uint32_t maxSize) { };

        [[nodiscard]] std::string const& getIconId() const;

        [[nodiscard]] std::weak_ptr<padi::LivingEntity> getUser() const;

        /*
         * Use sparingly - this is very volatile if the ability is in use!
         */
        void reassign(std::weak_ptr<padi::LivingEntity> const& user);

    protected:
        std::shared_ptr<padi::LivingEntity> m_user; // TODO weak_ptr instead
        std::string m_description;
        std::string m_iconId;
    };

    class LimitedRangeAbility : public Ability {
    public:
        LimitedRangeAbility(std::shared_ptr<padi::LivingEntity> user, size_t range);

        void castIndicator(const std::weak_ptr<Level> &level) override;

        bool cast(const std::weak_ptr<Level> &level, const sf::Vector2i &pos) override;

        void castCancel(const std::weak_ptr<Level> &level) override;

        [[nodiscard]] size_t getRange() const;

        void setRange(size_t range);

        bool isCastComplete() override = 0;

        [[nodiscard]] uint32_t getAbilityType() const override = 0;

        void writeProperties(uint8_t* data, uint32_t maxSize) override { };

    protected:
        virtual void recalculateRange(const std::weak_ptr<Level> &level);
        std::vector<sf::Vector2i> m_inRange;
        bool m_rangeChanged{true};
    private:
        std::vector<std::shared_ptr<padi::StaticEntity>> m_rangeIndicators;
        int m_range{2};
    };

} // padi
