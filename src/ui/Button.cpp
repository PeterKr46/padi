//
// Created by Peter on 04/05/2022.
//

#include "Button.h"
#include "UIContext.h"
#include "../media/Apollo.h"

namespace padi {
    Button::Button(padi::UIContext const* ctx) {
        m_animation = ctx->getApollo()->lookupAnim("scalable_button");
    }
} // padi