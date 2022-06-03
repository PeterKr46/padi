//
// Created by Peter on 01/05/2022.
//

#include "Ability.h"

#include <utility>
#include "../entity/LivingEntity.h"

namespace padi {

    Ability::Ability(std::shared_ptr<padi::LivingEntity> user) : m_user(std::move(user)) {

    }

    std::shared_ptr<padi::LivingEntity> Ability::getUser() const {
        return m_user;
    }

    LimitedRangeAbility::LimitedRangeAbility(std::shared_ptr<padi::LivingEntity> user, size_t range)
            : Ability(std::move(user)), m_range(int(range)), m_rangeChanged(true) {

    }

    size_t LimitedRangeAbility::getRange() const {
        return m_range;
    }

    void LimitedRangeAbility::setRange(size_t r) {
        m_range = int(r);
        m_rangeChanged = true;
    }

    void LimitedRangeAbility::recalculateRange(Level* level) {
        m_inRange.clear();
        m_inRange.reserve((m_range * 2) * (m_range * 2));
        sf::Vector2i min = m_user->getPosition();
        for (int x = min.x - m_range; x <= min.x + m_range; ++x)
            for (int y = min.y - m_range; y <= min.y + m_range; ++y) {
                m_inRange.emplace_back(x, y);
            }
        m_rangeChanged = false;
    }

    void LimitedRangeAbility::castIndicator(padi::Level *lvl) {
        if (m_rangeChanged) {
            recalculateRange(lvl);
            if(m_inRange.size() < m_rangeIndicators.size()) {
                for (size_t i = m_inRange.size(); i < m_rangeIndicators.size(); ++i) {
                    lvl->getMap()->removeUIObject(m_rangeIndicators[i]);
                }
                m_rangeIndicators.resize(m_inRange.size());
            } else if(m_inRange.size() > m_rangeIndicators.size()) {
                for (size_t i = m_rangeIndicators.size(); i < m_inRange.size(); ++i) {
                    m_rangeIndicators.push_back(std::make_shared<padi::StaticEntity>(m_inRange[i]));
                    lvl->getMap()->addUIObject(m_rangeIndicators[i]);
                }
            }
            for (size_t i = 0; i < m_inRange.size(); ++i) {
                lvl->getMap()->moveUIObject(m_rangeIndicators[i], m_inRange[i]);
                m_rangeIndicators[i]->m_animation = lvl->getApollo()->lookupAnim("indicator");
                m_rangeIndicators[i]->m_color = m_user->getColor();
                m_rangeIndicators[i]->m_color.a = 128;
            }
        }
    }

    bool LimitedRangeAbility::cast(padi::Level *level, const sf::Vector2i &pos) {
        m_rangeChanged = true;
        for(auto const& ind : m_rangeIndicators) {
            level->getMap()->removeUIObject(ind);
        }
        return true;
    }

    void LimitedRangeAbility::castCancel(padi::Level *level) {
        m_rangeChanged = true;
        for(auto const& ind : m_rangeIndicators) {
            level->getMap()->removeUIObject(ind);
        }
    }
}