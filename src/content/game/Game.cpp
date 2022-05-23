//
// Created by peter on 21/05/22.
//

#include "Game.h"
#include <SFML/Window/Keyboard.hpp>
#include "../../Controls.h"
#include "../../level/LevelGenerator.h"
#include "../../level/SpawnEvent.h"
#include "../abilities/Abilities.h"

namespace padi::content {


    Game::Game(sf::RenderTarget *target)
            : m_renderTarget(target) {
        if (m_vfxBuffer.create(float(target->getSize().x) / target->getSize().y * 256, 256 )) {
            auto levelGen = padi::LevelGenerator();
            time_t seed;
            time(&seed);
            m_level = levelGen
                    .withSpritesheet("../media/level_sheet.png")    // TODO
                    .withApollo("../media/level.apollo")            // TODO
                    .withSeed(seed)
                    .withArea({100, 100})
                    .generate();
            m_level->initCursor("cursor"); // TODO

            auto apollo = m_level->getApollo();
            m_player = std::make_shared<padi::LivingEntity>(apollo->lookupAnimContext("cube"), sf::Vector2i{0, 0});
            m_player->setColor({255, 255, 255});
            auto leSpawn = std::make_shared<padi::SpawnEvent>(m_player);
            leSpawn->dispatch(m_level);

            auto walk = std::make_shared<padi::content::Walk>();
            walk->user = m_player;
            m_playerAbilities.push_back(walk);
            auto tp = std::make_shared<padi::content::Teleport>();
            tp->user = m_player;
            m_playerAbilities.push_back(tp);
            m_playerAbilities.push_back(std::make_shared<padi::content::Lighten>());

            m_playerAbilities.push_back(std::make_shared<padi::content::Darken>());

            printf("[padi::content::Game] VfxBuffer at size %u, %u!\n", m_vfxBuffer.getSize().x, m_vfxBuffer.getSize().y);
        } else {
            printf("[padi::content::Game] Could not create vfxBuffer Texture!\n");
        }
    }

    void Game::draw() {
        m_level->update(&m_vfxBuffer);
        if (padi::Controls::isKeyDown(sf::Keyboard::Home)) {
            m_level->moveCursor(m_player->getPosition());
        }
        if (active != -1) {
            m_playerAbilities[active]->castIndicator(&(*m_level));
            if (padi::Controls::isKeyDown(sf::Keyboard::Enter)) {
                m_player->intentCast(m_playerAbilities[active], m_level->getCursorLocation());
                active = -1;
            } else if (padi::Controls::isKeyDown(sf::Keyboard::Escape)) {
                active = -1;
                m_level->hideCursor();
            }
        } else if (padi::Controls::isKeyDown(sf::Keyboard::Q)) {
            active = 0;
        } else if (padi::Controls::isKeyDown(sf::Keyboard::E)) {
            active = 1;
        } else if (padi::Controls::wasKeyPressed(sf::Keyboard::W)) {
            active = 2;
        } else if (padi::Controls::wasKeyPressed(sf::Keyboard::R)) {
            active = 3;
        }
        if (padi::Controls::wasKeyPressed(sf::Keyboard::Space)) {
            m_level->togglePause();
        }

        m_level->centerView(m_player->getPosition());

        m_level->populateVBO();
        m_vfxBuffer.clear();
        auto states = sf::RenderStates::Default;
        states.transform.scale(
                sf::Vector2f(256.f / m_vfxBuffer.getView().getSize().y, 256.f / m_vfxBuffer.getView().getSize().y));
        m_vfxBuffer.draw(*m_level, states);

        auto rState = sf::RenderStates::Default;
        rState.shader = m_level->isPaused() ? m_level->getApollo()->lookupShader("fpa_pause") : m_level->getApollo()->lookupShader("fpa");
        rState.texture = &m_vfxBuffer.getTexture();
        m_renderTarget->setView(m_renderTarget->getDefaultView());
        m_renderTarget->draw(m_screenQuad, rState);

    }

    void Game::handleResize(int width, int height) {
        sf::Vector2f halfSize{float(width), float(height)};
        halfSize /= 2.f;
        sf::Vector2f imgSize{m_vfxBuffer.getSize()};
        m_screenQuad[0].position = {0,0};
        m_screenQuad[0].texCoords = {0, imgSize.y};

        m_screenQuad[1].position = {0, float(height)};
        m_screenQuad[1].texCoords = {0, 0};

        m_screenQuad[2].position = {float(width), float(height)};
        m_screenQuad[2].texCoords = {imgSize.x, 0};

        m_screenQuad[3].position = {float(width), 0};
        m_screenQuad[3].texCoords = imgSize;
    }

    std::shared_ptr<padi::Activity> Game::handoff() {
        return shared_from_this();
    }

    void Game::draw(sf::RenderTarget &target, sf::RenderStates states) const {

    }
} // content