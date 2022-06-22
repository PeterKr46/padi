//
// Created by Peter on 21/06/2022.
//

#include "Narrator.h"

namespace padi::content {
    Narrator::Narrator(UIContext *uiContext)
            : m_uiContext(uiContext) {

    }

    void Narrator::displayText(const std::string &txt) {
        m_uiContext->setText("narrator_text", txt, sf::Vector2f{426.5f, 128.f}, true);
        m_uiContext->updateTextSize("narrator_text", 4);
        m_uiContext->updateTextOutline("narrator_text", sf::Color::Black, 1);
    }

    bool Narrator::operator()(const std::shared_ptr<OnlineGame> &, const std::shared_ptr<Character> &) {
        return false;
    }

} // content