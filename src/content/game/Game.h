//
// Created by peter on 21/05/22.
//
#pragma once

#include "../Activity.h"

#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/Shader.hpp"

#include "../../entity/LivingEntity.h"
#include "../../ui/UIContext.h"
#include "Character.h"
#include "CRTMonitor.h"

namespace padi::content {

    class Game : public padi::Activity {

    public:
        explicit Game(sf::RenderTarget *target);

        void draw(sf::RenderTarget* target) override;

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

        void handleResize(int width, int height) override;

        std::shared_ptr<Level> getLevel();

        std::shared_ptr<padi::Activity> handoff() override;

        void addCharacter(const std::shared_ptr<Character> &character);

    private:
        CRTMonitor m_crt;
        padi::UIContext m_uiContext;

        std::shared_ptr<Level> m_level;

        std::queue<std::shared_ptr<Character>> m_characters;
        std::shared_ptr<Character> m_activeChar;

    };

} // content
