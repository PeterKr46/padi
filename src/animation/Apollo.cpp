//
// Created by Peter on 29/04/2022.
//

#include "Apollo.h"

#include <utility>

namespace padi {
    std::shared_ptr<padi::Animation> Apollo::lookupAnim(const std::string &animName) const {
        auto iter = m_generalSet.find(animName);
        if (iter == m_generalSet.end()) return nullptr;
        return iter->second;
    }

    const padi::AnimationSet *Apollo::lookupContext(const std::string &charName) const {
        auto iter = m_contextMap.find(charName);
        if (iter == m_contextMap.end()) return nullptr;
        return &iter->second;
    }

    void Apollo::addAnimation(std::string const &name, std::shared_ptr<padi::Animation> anim) {
        m_generalSet[name] = std::move(anim);
    }

    bool Apollo::initializeContext(std::string const &name) {
        if (m_contextMap.find(name) != m_contextMap.end()) {
            return false;
        } else {
            m_contextMap[name] = {};
            return true;
        }
    }

    bool Apollo::addAnimation(std::string const &name, std::string const &animName,
                              const std::shared_ptr<padi::Animation> &anim) {
        auto ctx = m_contextMap.find(name);
        if (ctx != m_contextMap.end()) {
            ctx->second[animName] = anim;
            return true;
        } else {
            return false;
        }
    }

    std::shared_ptr<padi::Animation> Apollo::lookupAnim(const std::string &ctxName, const std::string &animName) const {
        auto ctx = m_contextMap.find(ctxName);
        if (ctx != m_contextMap.end()) {
            auto anim = ctx->second.find(animName);
            if (anim != ctx->second.end()) {
                return anim->second;
            }
        }
        return {nullptr};
    }

} // padi