//
// Created by Peter on 15/06/2022.
//

#pragma once

#include "../Activity.h"

namespace padi::content {

    class OnlineGame : public padi::Activity {
    public:
        void handleResize(int width, int height) override;
        void draw(sf::RenderTarget* target) override;
        std::shared_ptr<padi::Activity> handoff() override;
    };

} // content
