//
// Created by Peter on 21/06/2022.
//

#include "Narrator.h"

#include <cstring>
#include "../../ui/Immediate.h"
#include "OnlineGame.h"

namespace padi::content {

    void Narrator::displayText(const std::string &txt, UIContext *ui, bool center) {
        if(center) {
            ui->setText("narrator_text", txt, sf::Vector2f{226.5, 80.f}, true);
        } else {
            ui->setText("narrator_text", txt, sf::Vector2f{32, 80.f}, false);
        }
        ui->updateTextOutline("narrator_text", sf::Color::Black, 1);
    }

    void Narrator::clear(UIContext *ui) {
        m_active.clear();
        ui->removeText("narrator_text");
        ui->removeText("narrator_continue");
        ui->setFocusActive(false);
    }

    void Narrator::queueText(const char* msg, bool center) {
        NarratorEvent event{NarratorEvent::ShowText};
        event.data.showText.center = center;
        std::strncpy(event.data.showText.text, msg, std::min(strlen(msg), sizeof(event.data.showText.text) / sizeof(char) - 1));
        m_promptQueue.emplace(event);
    }

    void Narrator::queueFrame(const sf::FloatRect &rect) {
        NarratorEvent event{NarratorEvent::ShowFrame};
        event.data.showFrame = {rect};
        m_promptQueue.emplace(event);
    }

    void Narrator::queueConfirm() {
        m_promptQueue.emplace(NarratorEvent{NarratorEvent::Confirm});
    }

    void Narrator::queueSleep(float duration) {
        NarratorEvent event{NarratorEvent::Sleep};
        event.data.sleep = {duration};
        m_promptQueue.emplace(event);
    }

    void Narrator::queueCenter(sf::Vector2i const& p) {
        NarratorEvent event{NarratorEvent::CenterView};
        event.data.centerView = {p};
        m_promptQueue.emplace(event);
    }

    void Narrator::queueSprite(const char* id, sf::Vector2f const& center) {
        NarratorEvent event{NarratorEvent::ShowSprite};
        event.data.showSprite.pos = center;
        std::strncpy(event.data.showSprite.id, id, std::min(strlen(id), sizeof(event.data.showSprite.id) / sizeof(char) - 1));

        m_promptQueue.emplace(event);
    }

    bool Narrator::operator()(const std::shared_ptr<OnlineGame> &game, const std::shared_ptr<Character> &) {
        auto ui = game->getUIContext();
        if (m_active.empty()) {
            if (m_promptQueue.empty()) {
                return true;
            }
            unsigned int terminator;
            do {
                auto &event = m_promptQueue.front();
                m_active.emplace_back(event);
                m_promptQueue.pop();
                terminator = (m_active.back().type & (NarratorEvent::Confirm | NarratorEvent::Sleep));
                switch (event.type) {
                    case NarratorEvent::ShowText:
                        displayText(event.data.showText.text, ui, event.data.showText.center);
                        break;
                    case NarratorEvent::CenterView:
                        game->getLevel().lock()->centerView(event.data.centerView.center, true);
                        break;
                    case NarratorEvent::Confirm:
                        ui->setText("narrator_continue", "Continue", sf::Vector2f{226.5, 180}, true);
                        break;
                    case NarratorEvent::Sleep:
                        m_timer.restart();
                        break;
                    case NarratorEvent::ShowFrame:
                    case NarratorEvent::ShowSprite:
                        break;
                }
            } while (!m_promptQueue.empty() && terminator == 0);
        }
        for (auto &event: m_active) {
            if (event.type == NarratorEvent::ShowFrame) {
                Immediate::ScalableSprite(ui, event.data.showFrame.rect, 0,
                                          ui->getApollo()->lookupAnim("scalable_marker"), sf::Color::Red);
            } else if(event.type == NarratorEvent::ShowSprite) {
                auto sprite = ui->getApollo()->lookupAnim(event.data.showSprite.id);
                auto size = sf::Vector2f(sprite->getResolution());
                Immediate::Sprite(ui, {event.data.showSprite.pos - size / 2.f, size}, 0, sprite);
            }
        }
        unsigned int terminator = m_active.back().type;
        if (terminator == NarratorEvent::Confirm) {
            Immediate::setFocus(ui, "confirm");
            ui->setFocusActive(true);
            if (Immediate::Button(ui, "confirm", {176.5, 170, 100, 24})) {
                clear(ui);
                ui->setFocusActive(false);
            }
        } else if (terminator == NarratorEvent::Sleep) {
            ui->setFocusActive(false);
            if (m_timer.getElapsedTime().asSeconds() * speed >= m_active.back().data.sleep.duration) {
                clear(ui);
            }
        }
        return false;
    }

    void Narrator::queue(const NarratorEvent &event) {
        m_promptQueue.emplace(event);
    }

    Tutorial::Tutorial() {
        // Event 1
        queueText("Welcome to the Tutorial!");
        queueSleep(5);
        // Event 2
        queueText("This is you!", false);
        //queueCenter({-4, 4});
        queueCenter({0,0});
        queueFrame({sf::Vector2f {226-19, 127-19-8} + Map::mapTilePosToWorld({-4,4}), {38,38}});
        queueSleep(2);

        queueText("This is you! Yup, a cube...", false);
        queueSleep(5);
        queueText(""
                  "This is you! Yup, a cube...\n\n"
                  "                        ... that would rather not be here.", false);
        queueSleep(4);
        // Event 3
        queueText("So, how do you leave?");
        queueSleep(4);
        queueText("Each level, you must activate a beacon.");
        queueFrame({sf::Vector2f {226-19, 127-19-8} + Map::mapTilePosToWorld({0,0}), {38,38}});
        queueConfirm();

        queueText("First, you must defeat a certain number of enemies.");
        queueSleep(3);
        queueText("Enemies come in different flavors.");
        queueConfirm();

        queueText("Some  will chase you\n to go out with a bang!");
        queueFrame({sf::Vector2f {226-19, 127-19-8} + Map::mapTilePosToWorld({-3,2}), {38,38}});
        queueConfirm();

        queueText("Others will pursue your light,\n cursing every place they step.");
        queueFrame({sf::Vector2f {226-19, 127-19-8} + Map::mapTilePosToWorld({2,-3}), {38,38}});
        queueConfirm();

        queueText("Darkness is creeping all around you.");
        queueConfirm();

        queueText("Using a limited set of abilities,\n plan your every move carefully!");
        queueSleep(3.0f);
        queueText("Let's give it a go, start your turn!");
        queueConfirm();
    }

} // content