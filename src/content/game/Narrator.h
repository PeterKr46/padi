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
            CenterView    = 4,
            Confirm     = 8,
            Sleep       = 16
        } type;

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

        struct CenterView {
            sf::Vector2i center;
        };

        union {
            struct ShowText showText;
            struct ShowFrame showFrame;
            struct CenterView centerView;
            struct Sleep sleep;
        } data;
    };

    class Narrator {
    public:
        virtual bool operator()(const std::shared_ptr<OnlineGame> &, const std::shared_ptr<Character> &);


        float speed = 2.0f;

    protected:
        void displayText(std::string const &txt, UIContext *ctx, bool center = true);

        void clear(UIContext *ui);

        void queueText(const char* msg, bool center = true);
        void queueFrame(sf::FloatRect const& rect);
        void queueCenter(sf::Vector2i const& p);
        void queueConfirm();
        void queueSleep(float duration);

        std::vector<NarratorEvent> m_active;
        std::queue<NarratorEvent> m_promptQueue;
        sf::Clock m_timer;
    };

    class Tutorial : public Narrator {
    public:
        Tutorial();
    };

    class RemoteNarrator : public Narrator {
    public:
        bool operator()(const std::shared_ptr<OnlineGame> &, const std::shared_ptr<Character> &) override;
    };

} // content

