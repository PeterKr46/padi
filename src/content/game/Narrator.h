//
// Created by Peter on 21/06/2022.
//

#pragma once

#include <memory>
#include <queue>
#include <stack>
#include <string>
#include "SFML/Graphics/Rect.hpp"
#include "SFML/System/Clock.hpp"

namespace padi {
    class UIContext;
    namespace content {
        class OnlineGame;

        struct Character;
    }
}

namespace padi::content {

    struct NarratorEvent {
    public:
        enum : unsigned int {
            ShowText    = 1,
            ShowFrame   = 2,
            CenterView  = 4,
            Confirm     = 8,
            Sleep       = 16,
            ShowSprite  = 32,

        } type{};

        struct ShowText {
            bool center{true};
            char text[128]{};
        };

        struct Sleep {
            float duration{2};
        };

        struct ShowFrame {
            sf::FloatRect rect;
        };

        struct ShowSprite {
            char id[32]{};
            sf::Vector2f pos;
        };

        struct CenterView {
            sf::Vector2i center;
        };

        union {
            struct ShowText showText;
            struct ShowSprite showSprite;
            struct ShowFrame showFrame;
            struct CenterView centerView;
            struct Sleep sleep;
        } data;
    };

    class Narrator {
    public:
        virtual bool operator()(const std::shared_ptr<OnlineGame> &, const std::shared_ptr<Character> &);

        void queue(NarratorEvent const& event);

        float speed = 2.0f;

        void queueText(const char* msg, bool center = true);
        void queueFrame(sf::FloatRect const& rect);
        void queueCenter(sf::Vector2i const& p);
        void queueSprite(const char* id, sf::Vector2f const& center);
        void queueConfirm();
        void queueSleep(float duration);

    protected:
        static void displayText(std::string const &txt, UIContext *ctx, bool center = true);

        void clear(UIContext *ui);

        std::vector<NarratorEvent> m_active;
        std::queue<NarratorEvent> m_promptQueue;
        sf::Clock m_timer;
    };

    class Tutorial : public Narrator {
    public:
        Tutorial();
    };

} // content

