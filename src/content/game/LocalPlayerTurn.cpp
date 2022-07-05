//
// Created by Peter on 15/06/2022.
//

#include "LocalPlayerTurn.h"

#include "../../net/Packets.h"
#include "Character.h"
#include "OnlineGame.h"
#include "../../Controls.h"
#include "../../entity/LivingEntity.h"
#include "../../entity/Ability.h"
#include "../../ui/Immediate.h"
#include "../../media/AudioPlayback.h"
#include "../../level/Cursor.h"


namespace padi::content {

    LocalPlayerTurn::LocalPlayerTurn(UIContext *uiContext)
            : m_uiContext(uiContext) {

    }

    void LocalPlayerTurn::playSelectSound(std::shared_ptr<Level> const& level) {
        if(!m_selectSound) {
            m_selectSound = std::make_shared<AudioPlayback>(m_uiContext->getApollo()->lookupAudio("select"));
        }
        m_selectSound->restart(level);
    }

    bool
    LocalPlayerTurn::operator()(const std::shared_ptr<OnlineGame> &game, const std::shared_ptr<Character> &character) {
        auto level = game->getLevel().lock();
        auto emphColor = character->entity->getColor();
        auto ui = game->getUIContext();

        if (padi::Controls::wasKeyPressed(sf::Keyboard::Home)) {
            level->moveCursor(character->entity->getPosition());
            level->centerView(character->entity->getPosition());
        }
        LocalTurnState state = IDLE;
        if (m_activeAbility != -1) {
            state = SELECTING;
            if (!level->isPaused()) {
                state = TARGETING;
                if (m_hasCast) {
                    state = CASTING;
                    if (!character->entity->hasCastIntent()) {
                        if (!character->entity->hasFailedCast() &&
                            character->abilities[m_activeAbility]->isCastComplete()) {
                            state = DONE;
                        }
                    }
                }
            }
        }
        if (state == IDLE) {
            transitionUI(START, IDLE, character);
            padi::Immediate::ScalableSprite(m_uiContext,
                                            sf::FloatRect{0, 0, 453, 255},
                                            0,
                                            m_uiContext->getApollo()->lookupAnim("scalable_border"),
                                            character->entity->getColor());
            if (!m_uiContext->isFocusActive() && padi::Controls::wasKeyPressed(sf::Keyboard::Space)) {
                level->pause();
                m_activeAbility = 0;
                transitionUI(IDLE, SELECTING, character);
                m_uiContext->updateTextString("ability",
                                              character->abilities[m_activeAbility]->getDescription());
                playSelectSound(level);
            }
        } else if (state == SELECTING) {
            level->getCursor()->lock();
            if (!m_uiContext->isFocusActive() && padi::Controls::wasKeyPressed(sf::Keyboard::Space)) {
                level->play();

                transitionUI(SELECTING, TARGETING, character);
                playSelectSound(level);
            } else if (!m_uiContext->isFocusActive() && padi::Controls::wasKeyPressed(sf::Keyboard::Escape)) {
                m_activeAbility = -1;
                level->play();

                transitionUI(SELECTING, IDLE, character);
                playSelectSound(level);
            } else {
                static sf::Keyboard::Key updown[2] = {sf::Keyboard::Up, sf::Keyboard::Down};
                if (!m_uiContext->isFocusActive() && padi::Controls::wasKeyReleased(sf::Keyboard::Up)) {
                    m_activeAbility = (int64_t(character->abilities.size()) + m_activeAbility - 1) %
                                      int64_t(character->abilities.size());
                } else if (!m_uiContext->isFocusActive() && padi::Controls::wasKeyReleased(sf::Keyboard::Down)) {
                    m_activeAbility = (m_activeAbility + 1) % int64_t(character->abilities.size());
                }
                if (!m_uiContext->isFocusActive() && padi::Controls::wasAnyKeyReleased(updown, updown + 2)) {
                    m_uiContext->updateTextString("ability",
                                                  character->abilities[m_activeAbility]->getDescription());
                    playSelectSound(level);
                }

                auto numAbilities = character->abilities.size();
                sf::FloatRect bounds{4, 128 - 20 * float(numAbilities), 32, 40 * float(numAbilities) - 8};
                padi::Immediate::ScalableSprite(m_uiContext,
                                                sf::FloatRect{
                                                        bounds.left - 4, bounds.top - 4,
                                                        bounds.width + 8, bounds.height + 8
                                                },
                                                0,
                                                m_uiContext->getApollo()->lookupAnim("scalable_window"),
                                                sf::Color(168, 168, 168, 255));
                for (size_t i = 0; i < numAbilities; ++i) {
                    auto &ability = character->abilities[i];
                    padi::Immediate::Sprite(m_uiContext, sf::FloatRect{bounds.left, bounds.top + 40.f * i, 32, 32}, 0,
                                            m_uiContext->getApollo()->lookupAnim(
                                                    ability->getIconId()));
                    if (ability->numUses > 0) {
                        auto id = "uses_left_" + std::to_string(i);
                        m_uiContext->setText(id.c_str(), std::to_string(ability->numUses), {bounds.left + 3, bounds.top + 40.f * i});
                    }
                }
                padi::Immediate::ScalableSprite(m_uiContext,
                                                sf::FloatRect{bounds.left - 4,
                                                              bounds.top - 4 + float(m_activeAbility * 40), 40, 40},
                                                0,
                                                m_uiContext->getApollo()->lookupAnim("scalable_border"),
                                                emphColor);
                Immediate::Sprite(m_uiContext,
                                  sf::FloatRect{bounds.left, bounds.top - 24, 32, 32},
                                  0,
                                  m_uiContext->getApollo()->lookupAnim("arrow_up"),
                                  padi::Controls::isKeyDown(sf::Keyboard::Up) ? emphColor : sf::Color::White);
                Immediate::Sprite(m_uiContext,
                                  sf::FloatRect{bounds.left, bounds.top + bounds.height - 8, 32, 32},
                                  0,
                                  m_uiContext->getApollo()->lookupAnim("arrow_down"),
                                  padi::Controls::isKeyDown(sf::Keyboard::Down) ? emphColor : sf::Color::White);
            }
        } else if (state == TARGETING) {
            if (!m_uiContext->isFocusActive() && padi::Controls::wasKeyPressed(sf::Keyboard::Space)) {
                character->entity->intentCast(character->abilities[m_activeAbility], level->getCursorLocation());
                {
                    sf::Packet packet;
                    CharacterCastPayload payload;
                    payload.ability = uint8_t(m_activeAbility);
                    payload.pos = level->getCursorLocation();
                    printf("[LocalPlayerTurn] Casting %lld at (%i, %i)\n", m_activeAbility, payload.pos.x,
                           payload.pos.y);
                    packet.append(&payload, sizeof(payload));
                    game->broadcast(packet);
                }

                m_hasCast = true;
                level->hideCursor();
                transitionUI(TARGETING, CASTING, character);
            } else if (!m_uiContext->isFocusActive() && padi::Controls::wasKeyPressed(sf::Keyboard::Escape)) {
                transitionUI(TARGETING, SELECTING, character);
                character->abilities[m_activeAbility]->castCancel(level);
                level->pause();
                level->moveCursor(character->entity->getPosition());
                m_uiContext->updateTextString("ability", character->abilities[m_activeAbility]->getDescription());
            } else {
                character->abilities[m_activeAbility]->castIndicator(level);
            }
        } else if (state == CASTING) {
            if (!character->entity->hasCastIntent()) {
                if (character->entity->hasFailedCast()) {
                    transitionUI(CASTING, TARGETING, character);
                    printf("[LocalPlayerTurn] Miscast - retrying.\n");
                    m_hasCast = false;
                } else {
                    transitionUI(CASTING, DONE, character);
                }
            }
        }

        if (state == DONE) {
            m_hasCast = false;
            {
                auto ability = character->abilities[m_activeAbility];
                if (ability->numUses > 0) {
                    ability->numUses--;
                }
                if (ability->numUses == 0) {
                    character->abilities.erase(std::remove(character->abilities.begin(), character->abilities.end(), ability));
                    printf("[LocalPlayerTurn] Used up ability %i.\n", m_activeAbility);
                }
            }
            m_activeAbility = -1;
        }
        return state == DONE;
    }

    void LocalPlayerTurn::transitionUI(LocalPlayerTurn::LocalTurnState from, LocalPlayerTurn::LocalTurnState to,
                                       const std::shared_ptr<Character> &character) {
        switch (from) {
            case START:
            case CASTING:
            case DONE:
                break;
            case SELECTING:
                m_uiContext->removeText("ability");
                for (size_t i = 0; i < character->abilities.size(); ++i) {
                    if(character->abilities[i]->numUses > 0) {
                        auto id = "uses_left_" + std::to_string(i);
                        m_uiContext->removeText(id.c_str());
                    }
                }
                break;
            case IDLE:
                m_uiContext->removeText("press_space");
                break;
            case TARGETING:
                m_uiContext->removeText("cast_or_cancel");
                break;
        }
        switch (to) {
            case START:
            case DONE:
                break;
            case IDLE:
                m_uiContext->setText("press_space", "PRESS SPACE", {8, 8}, false);
                m_uiContext->updateTextOutline("press_space", sf::Color::Black, 1);
                m_uiContext->updateTextColor("press_space", character->entity->getColor());
                break;
            case SELECTING:
                m_uiContext->setText("ability", "",{8, 230});
                for (size_t i = 0; i < character->abilities.size(); ++i) {
                    if(character->abilities[i]->numUses > 0) {
                        auto id = "uses_left_" + std::to_string(i);
                        m_uiContext->setText(id.c_str(),"X", {0,0});
                        m_uiContext->updateTextColor(id.c_str(), character->entity->getColor());
                    }
                }
                break;
            case CASTING:
                break;
            case TARGETING:
                m_uiContext->setText("cast_or_cancel", "SPACE TO CONFIRM\nESCAPE TO ABORT", {8, 8}, false);
                m_uiContext->updateTextOutline("cast_or_cancel", sf::Color::Black, 1);
                m_uiContext->updateTextColor("cast_or_cancel", character->entity->getColor());
                break;
        }

    }
} // content