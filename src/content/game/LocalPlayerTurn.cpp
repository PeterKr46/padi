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

    bool
    LocalPlayerTurn::operator()(const std::shared_ptr<OnlineGame> &game, const std::shared_ptr<Character> &character) {
        auto level = game->getLevel().lock();
        auto emphColor = character->entity->getColor();

        if (padi::Controls::wasKeyPressed(sf::Keyboard::Home)) {
            level->moveCursor(character->entity->getPosition());
            level->centerView(character->entity->getPosition());
        }
        LocalTurnState state = IDLE;
        if (m_activeAbility != -1) {
            state = SELECTING;
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
        if (state == IDLE) {
            transitionUI(START, IDLE, emphColor);
            padi::Immediate::ScalableSprite(m_uiContext,
                                            sf::FloatRect{0,0, 453, 255},
                                            0,
                                            m_uiContext->getApollo()->lookupAnim("scalable_border"),
                                            character->entity->getColor());
            if (!m_uiContext->isFocusActive() && padi::Controls::wasKeyPressed(sf::Keyboard::Space)) {
                level->pause();
                m_activeAbility = 0;
                transitionUI(IDLE, SELECTING);
                m_uiContext->setText("ability", character->abilities[m_activeAbility]->getDescription(), {8, 24 + 128 + float(character->abilities.size()) * 20});
                auto ap = std::make_shared<AudioPlayback>(m_uiContext->getApollo()->lookupAudio("select"));
                ap->start(level);
            }
        } else if (state == SELECTING) {
            if (!level->isPaused()) {
                character->abilities[m_activeAbility]->castIndicator(level);
                if (padi::Controls::wasKeyPressed(sf::Keyboard::Space)) {
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
                    transitionUI(SELECTING, CASTING);
                } else if (padi::Controls::wasKeyPressed(sf::Keyboard::Escape)) {
                    character->abilities[m_activeAbility]->castCancel(level);
                    level->pause();
                    m_uiContext->setText("ability", character->abilities[m_activeAbility]->getDescription(),{8, 24+128 + float(character->abilities.size()) * 20});
                }
            } else {
                level->getCursor()->lock();
                if (padi::Controls::wasKeyPressed(sf::Keyboard::Space)) {
                    level->play();
                    transitionUI(SELECTING, CASTING);
                    auto ap = std::make_shared<AudioPlayback>(m_uiContext->getApollo()->lookupAudio("select"));
                    ap->start(level);
                } else {
                    if (padi::Controls::wasKeyPressed(sf::Keyboard::Escape)) {
                        transitionUI(SELECTING, IDLE, emphColor);
                        level->play();
                        m_activeAbility = -1;
                    } else {
                        int dir = 0;
                        if (padi::Controls::wasKeyReleased(sf::Keyboard::Up)) {
                            dir = -1;
                            m_activeAbility = (int64_t(character->abilities.size()) + m_activeAbility - 1) % int64_t(character->abilities.size());
                        } else if (padi::Controls::wasKeyReleased(sf::Keyboard::Down)) {
                            dir = 1;
                            m_activeAbility = (m_activeAbility + 1) % int64_t(character->abilities.size());
                        }
                        if(dir != 0) {
                            m_uiContext->updateTextString("ability", character->abilities[m_activeAbility]->getDescription());
                            auto ap = std::make_shared<AudioPlayback>(m_uiContext->getApollo()->lookupAudio("select"));
                            ap->start(level);
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
                        for(size_t i = 0; i < numAbilities; ++i) {
                            padi::Immediate::Sprite(m_uiContext, sf::FloatRect{bounds.left, bounds.top + 40 * i, 32, 32}, 0,
                                                    m_uiContext->getApollo()->lookupAnim(character->abilities[i]->getIconId()));
                        }
                        padi::Immediate::ScalableSprite(m_uiContext,
                                                        sf::FloatRect{bounds.left - 4, bounds.top-4 + float(m_activeAbility * 40), 40, 40},
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
                }
            }
        } else if (state == CASTING) {
            if (!character->entity->hasCastIntent()) {
                if (character->entity->hasFailedCast()) {
                    transitionUI(CASTING, SELECTING);
                    printf("[LocalPlayerTurn] Miscast - retrying.\n");
                    m_hasCast = false;
                } else {
                    transitionUI(CASTING, DONE);
                }
            }
        }

        if (state == DONE) {
            m_hasCast = false;
            m_activeAbility = -1;
        }
        return state == DONE;
    }

    void LocalPlayerTurn::transitionUI(LocalPlayerTurn::LocalTurnState from, LocalPlayerTurn::LocalTurnState to, sf::Color emphColor) {
        switch (from) {
            case START:
            case CASTING:
            case DONE:
                break;
            case SELECTING:
                m_uiContext->removeText("ability");
                break;
            case IDLE:
                m_uiContext->removeText("press_space");
                break;
        }
        switch (to) {
            case START:
            case DONE:
                break;
            case IDLE:
                m_uiContext->setText("press_space", "PRESS SPACE", {8,8}, false);
                m_uiContext->updateTextOutline("press_space", sf::Color::Black, 1);
                m_uiContext->updateTextColor("press_space", emphColor);
                break;
            case SELECTING:
            case CASTING:
                break;
        }

    }
} // content