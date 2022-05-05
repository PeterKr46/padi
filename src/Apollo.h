//
// Created by Peter on 29/04/2022.
//

#pragma once

#include <map>
#include <string>
#include <memory>
#include "Animation.h"

namespace padi {

    typedef std::map<std::string, std::shared_ptr<padi::Animation>> CharacterSet;

    class Apollo {
    public:

        [[nodiscard]] std::shared_ptr<padi::Animation> lookupAnim(std::string const& animName) const;

        [[nodiscard]] const padi::CharacterSet* lookupChar(std::string const& charName) const;
    private:
        std::map<std::string, std::shared_ptr<padi::Animation>> m_generalSet;
        std::map<std::string, CharacterSet> m_characterSets;

    };


} // padi
