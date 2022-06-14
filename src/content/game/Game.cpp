//
// Created by peter on 21/05/22.
//

#include "Game.h"
#include <SFML/Window/Keyboard.hpp>
#include <utility>
#include "../../Controls.h"
#include "../../level/LevelGenerator.h"
#include "../../level/SpawnEvent.h"
#include "../../map/Tile.h"
#include "../abilities/Abilities.h"
#include "../../ui/Immediate.h"
#include "../npc/Mob.h"
#include "MapShaker.h"

namespace padi::content {


    Game::Game(sf::RenderTarget *target)
            : m_renderTarget(target), m_graphicsClock() {
        if (m_vfxBuffer.create(int(float(target->getSize().x) / float(target->getSize().y) * 256), 256)) {
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
            m_uiContext.init("../media/ui.apollo", "../media/ui_sheet.png");
            auto shake = std::make_shared<MapShaker>();
            m_level->addFrameBeginListener(shake);

            auto apollo = m_level->getApollo();
            // TODO name
            sf::Color colors[3]{{255, 64,  64},
                                {64,  255, 64},
                                {64,  64,  255}};
            for (auto &color: colors) {
                auto player = std::make_shared<Character>();
                player->entity = std::make_shared<padi::LivingEntity>("player", apollo->lookupAnimContext("cube"),
                                                                      sf::Vector2i{0, 0});
                player->controller = [=](const std::shared_ptr<Level> &l, const std::shared_ptr<Character> &c) {
                    return defaultControls(l, c);
                };

                player->entity->setColor(color);
                auto leSpawn = std::make_shared<padi::SpawnEvent>(player->entity);
                leSpawn->dispatch(m_level);
                m_level->centerView(player->entity->getPosition());

                player->abilities.push_back(std::make_shared<padi::content::Walk>(player->entity, 8));
                player->abilities.push_back(std::make_shared<padi::content::Teleport>(player->entity));
                player->abilities.push_back(std::make_shared<padi::content::Lighten>(player->entity));
                player->abilities.push_back(std::make_shared<padi::content::Dash>(player->entity, 8));
                player->abilities.push_back(std::make_shared<padi::content::Darken>(player->entity));
                m_characters.push(player);
            }

            m_activeChar = m_characters.front();
            m_characters.pop();

            auto mob = std::make_shared<padi::content::Mob>("mob", apollo->lookupAnimContext("bubbleboi"),
                                                            sf::Vector2i{2, 2});
            auto mobSpawn = std::make_shared<padi::SpawnEvent>(mob);
            mobSpawn->dispatch(m_level);
            m_characters.push(std::make_shared<Character>(mob->asCharacter()));

            printf("[padi::content::Game] VfxBuffer at size %u, %u!\n", m_vfxBuffer.getSize().x,
                   m_vfxBuffer.getSize().y);
            auto light = std::make_shared<padi::StaticEntity>(sf::Vector2i{0, 0});
            light->m_animation = m_level->getApollo()->lookupAnim("lightshaft");
            m_level->getMap()->addEntity(light);
            light->m_color.a = 128;
            light->setVerticalOffset(-8);
        } else {
            printf("[padi::content::Game] Could not create vfxBuffer Texture!\n");
        }
    }

    void Game::draw() {
        m_level->update(&m_vfxBuffer);

        m_level->populateVBO();
        m_vfxBuffer.clear();
        auto states = sf::RenderStates::Default;
        states.transform.scale(256.f / m_vfxBuffer.getView().getSize().y, 256.f / m_vfxBuffer.getView().getSize().y);
        m_vfxBuffer.draw(*m_level, states);
        m_uiContext.nextFrame();

        if (m_activeChar->controller(m_level, m_activeChar)) {
            m_characters.push(m_activeChar);
            m_activeChar = m_characters.front();
            m_characters.pop();
        }
        m_vfxBuffer.draw(m_uiContext);

        auto rState = sf::RenderStates::Default;
        auto shader = m_level->getApollo()->lookupShader("fpa");
        shader->setUniform("time", m_graphicsClock.getElapsedTime().asSeconds());
        shader->setUniform("paused", m_level->isPaused());
        rState.shader = shader.get();
        rState.texture = &m_vfxBuffer.getTexture();
        m_renderTarget->setView(m_renderTarget->getDefaultView());

        m_renderTarget->draw(m_screenQuad, rState);
    }

    void Game::handleResize(int width, int height) {
        sf::Vector2f halfSize{float(width), float(height)};
        halfSize /= 2.f;
        sf::Vector2f imgSize{m_vfxBuffer.getSize()};
        m_screenQuad[0].position = {0, 0};
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
        // Immediate requires non-const draw :c
    }

    std::shared_ptr<Level> Game::getLevel() {
        return m_level;
    }

    enum TurnState : int {
        IDLE = 0,
        SELECTING = 1,
        CASTING = 2,
        DONE = 3
    };

    bool Game::defaultControls(const std::shared_ptr<Level> &level, const std::shared_ptr<Character> &character) {
        if (padi::Controls::isKeyDown(sf::Keyboard::Home)) {
            m_level->moveCursor(character->entity->getPosition());
            m_level->centerView(character->entity->getPosition());
        }
        TurnState state = IDLE;
        if (activeAbility != -1) {
            state = SELECTING;
            if (hasCast) {
                state = CASTING;
                if (!character->entity->hasCastIntent() && character->abilities[activeAbility]->isCastComplete()) {
                    state = DONE;
                }
            }
        }

        if (state == IDLE) {
            if (padi::Controls::wasKeyPressed(sf::Keyboard::Space)) {
                m_level->pause();
                activeAbility = 0;
            }
        } else if (state == SELECTING) {
            if (padi::Controls::wasKeyPressed(sf::Keyboard::Escape)) {
                activeAbility = -1;
                m_level->play();
            } else if (!m_level->isPaused()) {
                character->abilities[activeAbility]->castIndicator(m_level.get());
                if (padi::Controls::wasKeyPressed(sf::Keyboard::Enter)) {
                    character->entity->intentCast(character->abilities[activeAbility], m_level->getCursorLocation());
                    hasCast = true;
                } else if (padi::Controls::wasKeyPressed(sf::Keyboard::Escape)) {
                    character->abilities[activeAbility]->castCancel(m_level.get());
                    m_level->pause();
                }
            } else {
                if (padi::Controls::wasKeyPressed(sf::Keyboard::Enter)) {
                    m_level->play();
                } else {
                    if (padi::Controls::wasKeyReleased(sf::Keyboard::Q)) {
                        character->abilities[activeAbility]->castCancel(&(*m_level));
                        activeAbility = std::max(0, activeAbility - 1);
                    }
                    if (padi::Controls::wasKeyReleased(sf::Keyboard::E)) {
                        character->abilities[activeAbility]->castCancel(&(*m_level));
                        activeAbility = std::min(int(character->abilities.size()) - 1, activeAbility + 1);
                    }
                    m_uiContext.pushTransform().translate(228 - 64, 256 - 72);
                    padi::Immediate::ScalableSprite(&m_uiContext, sf::FloatRect{-4, -4, 160, 40}, 0,
                                                    m_uiContext.getApollo()->lookupAnim("scalable_window"),
                                                    sf::Color(128, 128, 128, 255));
                    padi::Immediate::Sprite(&m_uiContext, sf::FloatRect{0, 0, 32, 32}, 0,
                                            m_uiContext.getApollo()->lookupAnim("walk"));
                    padi::Immediate::Sprite(&m_uiContext, sf::FloatRect{40, 0, 32, 32}, 0,
                                            m_uiContext.getApollo()->lookupAnim("teleport"));
                    padi::Immediate::Sprite(&m_uiContext, sf::FloatRect{80, 0, 32, 32}, 0,
                                            m_uiContext.getApollo()->lookupAnim("strike"));
                    padi::Immediate::Sprite(&m_uiContext, sf::FloatRect{120, 0, 32, 32}, 0,
                                            m_uiContext.getApollo()->lookupAnim("dash"));
                    padi::Immediate::ScalableSprite(&m_uiContext,
                                                    sf::FloatRect{-4 + float(activeAbility * 40), -4, 40, 40},
                                                    0,
                                                    m_uiContext.getApollo()->lookupAnim("scalable_border"),
                                                    character->entity->getColor());
                    m_uiContext.popTransform();
                }
            }
        }
        if(state == DONE) {
            hasCast = false;
            activeAbility = -1;
        }
        return state == DONE;
    }
} // content