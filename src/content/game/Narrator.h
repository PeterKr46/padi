//
// Created by Peter on 21/06/2022.
//

#pragma once

#include <memory>
#include "OnlineGame.h"

namespace padi::content {

    class Narrator {
    public:
        explicit Narrator(UIContext *uiContext);

        void displayText(std::string const& txt);
    private:
        padi::UIContext *m_uiContext;
    };

} // content

