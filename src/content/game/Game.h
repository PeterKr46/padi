//
// Created by peter on 21/05/22.
//
#pragma once

#include "../Activity.h"

#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/Shader.hpp"

#include "../../entity/LivingEntity.h"
#include "../../ui/UIContext.h"

namespace padi::content {
    class Game : public padi::Activity {

    public:
        explicit Game(sf::RenderTarget *target);

        void draw() override;

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

        void handleResize(int width, int height) override;

        std::shared_ptr<Level> getLevel();

        std::shared_ptr<padi::Activity> handoff() override;

    private:
        sf::RenderTarget *m_renderTarget;
        sf::RenderTexture m_vfxBuffer;
        sf::VertexArray m_screenQuad{sf::Quads, 4};
        std::shared_ptr<Level> m_level;

        padi::UIContext m_uiContext;

        std::shared_ptr<padi::LivingEntity> m_player;
        std::vector<std::shared_ptr<padi::Ability>> m_playerAbilities;
        int active = -1;


        sf::Clock m_graphicsClock;
    };

} // content
