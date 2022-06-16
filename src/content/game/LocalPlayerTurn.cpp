//
// Created by Peter on 15/06/2022.
//

#include "LocalPlayerTurn.h"

#include "../../Controls.h"
#include "../../ui/Immediate.h"
#include "../../level/Level.h"
#include "../../entity/Ability.h"

// TODO
#include "Game.h"
#include "OnlineGame.h"
#include "Packets.h"

namespace padi::content {
    enum LocalTurnState : int {
        IDLE = 0,
        SELECTING = 1,
        CASTING = 2,
        DONE = 3
    };

    LocalPlayerTurn::LocalPlayerTurn(UIContext *uiContext, std::vector<std::shared_ptr<sf::TcpSocket>> &sockets)
            : m_uiContext(uiContext), m_sockets(sockets) {

    }

    bool LocalPlayerTurn::operator()(const std::shared_ptr<Level> &level, const std::shared_ptr<Character> &character) {
        m_uiContext->setText("local_turn", "Your Turn!", {226,20}, true);
        m_uiContext->updateTextOutline("local_turn", sf::Color::Black, 1);
        m_uiContext->updateTextSize("local_turn", 2);
        if (padi::Controls::isKeyDown(sf::Keyboard::Home)) {
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
            if (padi::Controls::wasKeyPressed(sf::Keyboard::Space)) {
                level->pause();
                m_activeAbility = 0;
                m_uiContext->setText("ability", character->abilities[m_activeAbility]->getDescription(), {8, 8});
            }
        } else if (state == SELECTING) {
            if (!level->isPaused()) {
                character->abilities[m_activeAbility]->castIndicator(level.get());
                if (padi::Controls::wasKeyPressed(sf::Keyboard::Enter)) {
                    character->entity->intentCast(character->abilities[m_activeAbility], level->getCursorLocation());
                    {
                        sf::Packet packet;
                        PlayerCastPayload payload;
                        payload.ability = uint8_t(m_activeAbility);
                        payload.pos = level->getCursorLocation();
                        printf("[LocalPlayerTurn] Casting %u at (%i, %i)\n", m_activeAbility, payload.pos.x, payload.pos.y);
                        packet.append(&payload, sizeof(payload));
                        for (auto &socket: m_sockets) {
                            socket->send(packet);
                        }
                    }

                    m_hasCast = true;
                    level->hideCursor();
                } else if (padi::Controls::wasKeyPressed(sf::Keyboard::Escape)) {
                    character->abilities[m_activeAbility]->castCancel(level.get());
                    level->pause();
                    m_uiContext->setText("ability", character->abilities[m_activeAbility]->getDescription(), {8, 8});
                }
            } else {
                if (padi::Controls::wasKeyPressed(sf::Keyboard::Enter)) {
                    level->play();
                    m_uiContext->removeText("ability");
                } else {
                    if (padi::Controls::wasKeyPressed(sf::Keyboard::Escape)) {
                        character->abilities[m_activeAbility]->castCancel(level.get());
                        level->play();
                        m_uiContext->removeText("ability");
                        m_activeAbility = -1;
                    } else if (padi::Controls::wasKeyReleased(sf::Keyboard::Q)) {
                        character->abilities[m_activeAbility]->castCancel(&(*level));
                        m_activeAbility = std::max(0, m_activeAbility - 1);
                        m_uiContext->setText("ability", character->abilities[m_activeAbility]->getDescription(),
                                             {8, 8});
                    } else if (padi::Controls::wasKeyReleased(sf::Keyboard::E)) {
                        character->abilities[m_activeAbility]->castCancel(&(*level));
                        m_activeAbility = std::min(int(character->abilities.size()) - 1, m_activeAbility + 1);
                        m_uiContext->setText("ability", character->abilities[m_activeAbility]->getDescription(),
                                             {8, 8});
                    }
                    m_uiContext->pushTransform().translate(228 - 64, 256 - 72);
                    padi::Immediate::ScalableSprite(m_uiContext, sf::FloatRect{-4, -4, 160, 40}, 0,
                                                    m_uiContext->getApollo()->lookupAnim("scalable_window"),
                                                    sf::Color(168, 168, 168, 255));
                    padi::Immediate::Sprite(m_uiContext, sf::FloatRect{0, 0, 32, 32}, 0,
                                            m_uiContext->getApollo()->lookupAnim("walk"));
                    padi::Immediate::Sprite(m_uiContext, sf::FloatRect{40, 0, 32, 32}, 0,
                                            m_uiContext->getApollo()->lookupAnim("teleport"));
                    padi::Immediate::Sprite(m_uiContext, sf::FloatRect{80, 0, 32, 32}, 0,
                                            m_uiContext->getApollo()->lookupAnim("strike"));
                    padi::Immediate::Sprite(m_uiContext, sf::FloatRect{120, 0, 32, 32}, 0,
                                            m_uiContext->getApollo()->lookupAnim("dash"));
                    padi::Immediate::ScalableSprite(m_uiContext,
                                                    sf::FloatRect{-4 + float(m_activeAbility * 40), -4, 40, 40},
                                                    0,
                                                    m_uiContext->getApollo()->lookupAnim("scalable_border"),
                                                    character->entity->getColor());
                    m_uiContext->popTransform();
                }
            }
        } else if (state == CASTING) {
            if (!character->entity->hasCastIntent() && character->entity->hasFailedCast()) {
                printf("[LocalPlayerTurn] Miscast - retrying.\n");
                m_hasCast = false;
            }
        }

        if (state == DONE) {
            m_hasCast = false;
            m_activeAbility = -1;
            m_uiContext->removeText("local_turn");
        }
        return state == DONE;
    }
} // content