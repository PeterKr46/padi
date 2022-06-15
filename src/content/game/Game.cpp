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
#include "LocalPlayerTurn.h"
#include "CRTMonitor.h"

namespace padi::content {


    Game::Game(sf::RenderTarget *target) : m_crt(nullptr) {
        auto levelGen = padi::LevelGenerator();
        time_t seed = 1655244295;
        //time(&seed);
        m_level = levelGen
                .withSpritesheet("../media/level_sheet.png")    // TODO
                .withApollo("../media/level.apollo")            // TODO
                .withSeed(seed)
                .withArea({100, 100})
                .generate();
        m_level->initCursor("cursor"); // TODO
        m_crt.handleResize(int(target->getSize().x), int(target->getSize().y));
        m_crt.setShader(m_level->getApollo()->lookupShader("fpa"));
        m_uiContext.init("../media/ui.apollo", "../media/ui_sheet.png");
        auto shake = std::make_shared<MapShaker>();
        m_level->addFrameBeginListener(shake);

        auto apollo = m_level->getApollo();
        // TODO name
        sf::Color colors[1]{{255, 64,  64}};
                            /*{64,  255, 64},
                            {64,  64,  255}};*/
        for (auto &color: colors) {
            auto player = std::make_shared<Character>();
            player->entity = std::make_shared<padi::LivingEntity>("player", apollo->lookupAnimContext("cube"),
                                                                  sf::Vector2i{0, 0});
            LocalPlayerTurn local(&m_uiContext);
            player->controller = [=](const std::shared_ptr<Level> &l, const std::shared_ptr<Character> &c) mutable {
                return local(l, c);
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
    }

    void Game::draw(sf::RenderTarget* target) {
        m_level->update(m_crt.asTarget());

        m_level->populateVBO();
        m_crt.asTarget()->clear();
        auto states = sf::RenderStates::Default;
        states.transform.scale(256.f / m_crt.asTarget()->getView().getSize().y, 256.f / m_crt.asTarget()->getView().getSize().y);
        m_crt.asTarget()->draw(*m_level, states);
        m_uiContext.nextFrame();

        if (m_activeChar->controller(m_level, m_activeChar)) {
            if(m_activeChar->alive) {
                m_characters.push(m_activeChar);
            }
            m_activeChar = m_characters.front();
            m_characters.pop();
            if(m_activeChar->entity) {
                m_level->centerView(m_activeChar->entity->getPosition());
                m_level->moveCursor(m_activeChar->entity->getPosition());
            }
        }
        m_crt.asTarget()->draw(m_uiContext);
        target->draw(m_crt);
    }

    void Game::handleResize(int width, int height) {
       m_crt.handleResize(width,height);
    }

    std::shared_ptr<padi::Activity> Game::handoff() {
        return shared_from_this();
    }

    void Game::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        // Immediate requires non-const draw :c
    }

    void Game::addCharacter(const std::shared_ptr<Character> &character) {
        m_characters.push(character);
    }
}