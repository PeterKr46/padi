//
// Created by Peter on 21/06/2022.
//

#include "Narrator.h"

#include "../../ui/Immediate.h"
#include "OnlineGame.h"

namespace padi::content {

    void Narrator::displayText(const std::string &txt, UIContext* ui) {
        ui->setText("narrator_text", txt, sf::Vector2f{226.5, 50.f}, true);
        ui->updateTextSize("narrator_text", 1);
        ui->updateTextOutline("narrator_text", sf::Color::Black, 1);
        ui->setText("narrator_continue", "Continue", sf::Vector2f{226.5, 180}, true);
    }

    void Narrator::clear(UIContext* ui) {
        ui->removeText("narrator_text");
        ui->removeText("narrator_continue");
        ui->setFocusActive(false);
    }

    bool LocalNarrator::operator()(const std::shared_ptr<OnlineGame> & game, const std::shared_ptr<Character> &) {
        if(m_promptQueue.empty()) return true;
        auto ui = game->getUIContext();
        ui->setFocusActive(true);
        displayText(m_promptQueue.front(), ui);
        if(Immediate::Button(ui, "confirm", {176.5,170,100, 25})) {
            m_promptQueue.pop();
            clear(ui);
            return true;
        }
        return false;
    }

    LocalNarrator::LocalNarrator() {
        m_promptQueue.push("Welcome to the Tutorial!\n\n"
                           "This Level will demonstrate the basic gameplay loop.\n"
                           "Your task is simple - clear the level before the da-\n"
                           "rkness takes it all.\n"
                           "  You (and your allies) must fight your path to an\n"
                           "exit, which shows after you've cleared a large enough");
    }

    bool RemoteNarrator::operator()(const std::shared_ptr<OnlineGame> &, const std::shared_ptr<Character> &) {
        return false;
    }

} // content