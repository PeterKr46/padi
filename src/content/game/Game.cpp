//
// Created by peter on 21/05/22.
//

#include "Game.h"
#include "../../level/LevelGenerator.h"
#include "../../Controls.h"
#include "SFML/Window/Keyboard.hpp"
#include "../../level/SpawnEvent.h"

namespace padi::content {

    Game::Game(sf::RenderTarget *target)
            : m_renderTarget(target) {
        if (m_vfxBuffer.create(target->getSize().x, target->getSize().y)) {
            {
                sf::Vector2f halfSize{target->getSize()};
                halfSize /= 2.f;
                sf::Vector2f imgSize{m_vfxBuffer.getSize()};
                m_screenQuad[0].position = -halfSize;
                m_screenQuad[0].texCoords = {0, imgSize.y};

                m_screenQuad[1].position = {-halfSize.x, halfSize.y};
                m_screenQuad[1].texCoords = {0, 0};

                m_screenQuad[2].position = halfSize;
                m_screenQuad[2].texCoords = {imgSize.x, 0};

                m_screenQuad[3].position = {halfSize.x, -halfSize.y};
                m_screenQuad[3].texCoords = imgSize;
            }
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
        } else {
            printf("[padi::content::Game] Could not create vfxBuffer Texture!\n");
        }
    }

    void Game::draw() {
        m_level->update(&m_vfxBuffer);
        if (padi::Controls::isKeyDown(sf::Keyboard::Home)) {
            m_level->moveCursor(m_player->getPosition());
        }
        /*if (activeAbility) {
            activeAbility->castIndicator(&(*m_level));
            if (padi::Controls::isKeyDown(sf::Keyboard::Space)) {
                m_player->intentCast(activeAbility, m_level->getCursorLocation());
                activeAbility = nullptr;
            } else if (padi::Controls::isKeyDown(sf::Keyboard::Escape)) {
                activeAbility = nullptr;
                m_level->hideCursor();
            }
        } else if (padi::Controls::isKeyDown(sf::Keyboard::Q)) {
            activeAbility = lightenAbility;
        } else if (padi::Controls::isKeyDown(sf::Keyboard::E)) {
            activeAbility = tpAbility;
        } else if (padi::Controls::wasKeyPressed(sf::Keyboard::W)) {
            activeAbility = walkAbility;
        } else if (padi::Controls::wasKeyPressed(sf::Keyboard::R)) {
            activeAbility = darkenAbility;
        }*/

        m_level->centerView(m_player->getPosition());

        //quadCounter.setString(std::to_string(m_level->getMap()->numQuads()));
        //quadCounter.setPosition(m_level->getMap()->mapTilePosToWorld(m_level->getCursorLocation()));

        m_level->populateVBO();
        m_vfxBuffer.clear();
        auto states = sf::RenderStates::Default;
        states.transform.scale(
                sf::Vector2f(256.f / m_vfxBuffer.getView().getSize().y, 256.f / m_vfxBuffer.getView().getSize().y));
        m_vfxBuffer.draw(*m_level, states);
        //m_vfxBuffer.draw(quadCounter, states);

        auto rState = sf::RenderStates::Default;
        //rState.shader = &crtShader;
        rState.texture = &m_vfxBuffer.getTexture();
        m_renderTarget->draw(m_screenQuad, rState);
    }

    void Game::handleResize(int width, int height) {
        {
            sf::Vector2f halfSize{float(width), float(height)};
            halfSize /= 2.f;
            sf::Vector2f imgSize{m_vfxBuffer.getSize()};
            m_screenQuad[0].position = -halfSize;
            m_screenQuad[0].texCoords = {0, imgSize.y};

            m_screenQuad[1].position = {-halfSize.x, halfSize.y};
            m_screenQuad[1].texCoords = {0, 0};

            m_screenQuad[2].position = halfSize;
            m_screenQuad[2].texCoords = {imgSize.x, 0};

            m_screenQuad[3].position = {halfSize.x, -halfSize.y};
            m_screenQuad[3].texCoords = imgSize;
        }
    }

    std::shared_ptr<padi::Activity> Game::handoff() {
        return shared_from_this();
    }

    void Game::draw(sf::RenderTarget &target, sf::RenderStates states) const {

    }
} // content