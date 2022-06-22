//
// Created by Peter on 17/06/2022.
//

#include "Chat.h"

#include "../../ui/Immediate.h"
#include "../../Constants.h"
#include "../../Controls.h"

namespace padi::content {
    void Chat::draw(padi::UIContext *context) {

        if (!m_hidden) {
            context->pushTransform().translate(m_bounds.left, m_bounds.top);

            Immediate::ScalableSprite(context,
                                      {-4, -4,
                                       m_bounds.width + 8, m_bounds.height + 4},
                                      0,
                                      context->getApollo()->lookupAnim("scalable_textfield"),
                                      sf::Color(0xFFFFFF88));
            if (Immediate::isFocused(context, "chat_input")) {
                /*Immediate::ScalableSprite(context,
                                          {-4, m_bounds.height - 14,
                                           m_bounds.width + 8, 14},
                                          0,
                                          context->getApollo()->lookupAnim("scalable_textfield"));*/
                Immediate::ScalableSprite(context,
                                          {-4, m_bounds.height - 14,
                                           m_bounds.width + 8, 14},
                                          0,
                                          context->getApollo()->lookupAnim("scalable_border"),
                                          sf::Color::Yellow);

                context->updateTextColor("chat_input", sf::Color::Yellow);
            }
            else {
                context->updateTextColor("chat_input", sf::Color::White);
            }
            context->popTransform();

            if (Immediate::TextInput(context, "chat_input", &m_inputBuf, m_lineLength, SimpleCharacterSet)) {
                context->updateTextString("chat_input", m_inputBuf);
            }
            if (Immediate::isFocused(context, "chat_input") && padi::Controls::wasKeyReleased(sf::Keyboard::Enter)) {
                if (submit) {
                    submit(m_inputBuf);
                }
                m_inputBuf = "";
                context->updateTextString("chat_input", "");
            }
        }
    }

    void Chat::setHidden(padi::UIContext *context, bool hidden) {
        if (m_hidden != hidden) {
            m_hidden = hidden;
            if (hidden) {
                context->removeText("chat_title");
                context->removeText("chat_input");
                context->removeText("chat_log");
            } else {
                init(context);
            }
        }
    }

    // Used to approximate the number of characters that fit into the Chat log
    #define CHAR_W 7.f
    #define CHAR_H 13.f

    void Chat::init(padi::UIContext *context) {
        if (!m_hidden) {
            m_logBuf.resize(size_t(m_bounds.height / CHAR_H), '\n');
            m_lineLength = size_t(m_bounds.width / CHAR_W);
            context->pushTransform().translate(m_bounds.left, m_bounds.top);
            context->setText("chat_title", "", {0, -12});
            context->setText("chat_input", "", {0, m_bounds.height - 11});
            context->setText("chat_log", m_logBuf, {0, 0});
            context->popTransform();
        }
    }

    Chat::Chat(sf::FloatRect const &bounds)
            : m_bounds(bounds) {

    }

    void Chat::write(padi::UIContext *context, const std::string &msg) {
        m_logBuf = m_logBuf.substr(m_logBuf.find_first_of('\n') + 1) + '\n' + msg;
        if (!m_hidden) {
            context->updateTextString("chat_log", m_logBuf);
        }
    }
} // content