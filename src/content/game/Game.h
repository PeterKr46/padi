//
// Created by peter on 21/05/22.
//
#pragma once

#include "../Activity.h"
#include "SFML/Graphics/RenderTexture.hpp"
#include "../../entity/LivingEntity.h"

namespace padi::content {
    class Game : public padi::Activity {

    public:
        explicit Game(sf::RenderTarget* target);

        void draw() override;

        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

        void handleResize(int width, int height) override;

        std::shared_ptr<padi::Activity> handoff() override;

    private:
        sf::RenderTarget* m_renderTarget;
        std::shared_ptr<Level> m_level;
        std::shared_ptr<padi::LivingEntity> m_player;
        sf::RenderTexture m_vfxBuffer;
        sf::VertexArray m_screenQuad{sf::Quads, 4};

    };

} // content
