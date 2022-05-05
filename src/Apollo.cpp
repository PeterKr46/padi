//
// Created by Peter on 29/04/2022.
//

#include "Apollo.h"

namespace padi {
    std::shared_ptr<padi::Animation> Apollo::lookupAnim(const std::string &animName) const {
        auto iter = m_generalSet.find(animName);
        if (iter == m_generalSet.end()) return nullptr;
        return iter->second;
    }

    const padi::CharacterSet* Apollo::lookupChar(const std::string &charName) const {
        auto iter = m_characterSets.find(charName);
        if (iter == m_characterSets.end()) return nullptr;
        return &iter->second;
    }

} // padi