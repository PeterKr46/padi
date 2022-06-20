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
#include "../../level/Level.h"


namespace padi::content {
    enum LocalTurnState : int {
        IDLE = 0,
        SELECTING = 1,
        CASTING = 2,
        DONE = 3
    };

    LocalPlayerTurn::LocalPlayerTurn(UIContext *uiContext)
            : m_uiContext(uiContext) {

    }

    bool
    LocalPlayerTurn::operator()(const std::shared_ptr<OnlineGame> &game, const std::shared_ptr<Character> &character) {
        auto level = game->getLevel().lock();

        m_uiContext->setText("local_turn", "Your Turn!", {226, 20}, true);
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
            if (!m_uiContext->isFocusActive() && padi::Controls::wasKeyPressed(sf::Keyboard::Space)) {
                level->pause();
                m_activeAbility = 0;
                m_uiContext->setText("ability", character->abilities[m_activeAbility]->getDescription(), {8, 8});
                auto ap = std::make_shared<AudioPlayback>(m_uiContext->getApollo()->lookupAudio("select"));
                ap->start(level);
            }
        } else if (state == SELECTING) {
            if (!level->isPaused()) {
                character->abilities[m_activeAbility]->castIndicator(level);
                if (padi::Controls::wasKeyPressed(sf::Keyboard::Enter)) {
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
                } else if (padi::Controls::wasKeyPressed(sf::Keyboard::Escape)) {
                    character->abilities[m_activeAbility]->castCancel(level);
                    level->pause();
                    m_uiContext->setText("ability", character->abilities[m_activeAbility]->getDescription(), {8, 8});
                }
            } else {
                if (padi::Controls::wasKeyPressed(sf::Keyboard::Enter)) {
                    level->play();
                    m_uiContext->removeText("ability");
                } else {
                    if (padi::Controls::wasKeyPressed(sf::Keyboard::Escape)) {
                        character->abilities[m_activeAbility]->castCancel(level);
                        level->play();
                        m_uiContext->removeText("ability");
                        m_activeAbility = -1;
                    } else {
                        int dir = 0;
                        if (padi::Controls::wasKeyReleased(sf::Keyboard::Q)) {
                            dir = -1;
                            m_activeAbility = (int64_t(character->abilities.size()) + m_activeAbility - 1) % int64_t(character->abilities.size());
                        } else if (padi::Controls::wasKeyReleased(sf::Keyboard::E)) {
                            dir = 1;
                            m_activeAbility = (m_activeAbility + 1) % int64_t(character->abilities.size());
                        }
                        if(dir != 0) {
                            character->abilities[m_activeAbility]->castCancel(level);
                            m_uiContext->setText("ability", character->abilities[m_activeAbility]->getDescription(),{8, 8});
                            auto ap = std::make_shared<AudioPlayback>(m_uiContext->getApollo()->lookupAudio("select"));
                            ap->start(level);
                        }
                        auto numAbilities = character->abilities.size();
                        sf::FloatRect bounds{223 - 20 * float(numAbilities), 256 - 72, 40 * float(numAbilities) - 8, 32};
                        padi::Immediate::ScalableSprite(m_uiContext,
                                                        sf::FloatRect{
                                                                bounds.left - 4, bounds.top - 4,
                                                                bounds.width + 8, bounds.height + 8
                                                        },
                                                        0,
                                                        m_uiContext->getApollo()->lookupAnim("scalable_window"),
                                                        sf::Color(168, 168, 168, 255));
                        for(size_t i = 0; i < numAbilities; ++i) {
                            padi::Immediate::Sprite(m_uiContext, sf::FloatRect{bounds.left + 40 * i, bounds.top, 32, 32}, 0,
                                                    m_uiContext->getApollo()->lookupAnim(character->abilities[i]->getIconId()));
                        }
                        padi::Immediate::ScalableSprite(m_uiContext,
                                                        sf::FloatRect{bounds.left - 4 + float(m_activeAbility * 40), bounds.top-4, 40, 40},
                                                        0,
                                                        m_uiContext->getApollo()->lookupAnim("scalable_border"),
                                                        character->entity->getColor());
                    }
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