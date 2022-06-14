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

    struct Character {
        std::shared_ptr<padi::LivingEntity> entity; // OPTIONAL
        std::vector<std::shared_ptr<padi::Ability>> abilities;
        std::function<bool(const std::shared_ptr<Level> &, const std::shared_ptr<Character> &)> controller;
        bool alive{true};
    };

    class Game : public padi::Activity {

    public:
        explicit Game(sf::RenderTarget *target);

        void draw() override;

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

        void handleResize(int width, int height) override;

        std::shared_ptr<Level> getLevel();

        std::shared_ptr<padi::Activity> handoff() override;

        bool defaultControls(const std::shared_ptr<Level> &level, const std::shared_ptr<Character> &character);

        void addCharacter(const std::shared_ptr<Character> &character);

    private:
        sf::RenderTarget *m_renderTarget;
        sf::RenderTexture m_vfxBuffer;
        sf::VertexArray m_screenQuad{sf::Quads, 4};
        std::shared_ptr<Level> m_level;

        padi::UIContext m_uiContext;

        std::queue<std::shared_ptr<Character>> m_characters;
        std::shared_ptr<Character> m_activeChar;


        int activeAbility = -1;
        bool hasCast = false;


        sf::Clock m_graphicsClock;
    };

} // content
