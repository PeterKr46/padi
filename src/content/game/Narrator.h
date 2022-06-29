//
// Created by Peter on 21/06/2022.
//

#pragma once

#include <memory>
#include <queue>
#include <string>

namespace padi {
    class UIContext;
    namespace content {
        class OnlineGame;
        struct Character;
    }
}

namespace padi::content {

    class Narrator {
    public:

        virtual bool operator()(const std::shared_ptr<OnlineGame> &, const std::shared_ptr<Character> &) = 0;

    protected:
        void displayText(std::string const& txt, UIContext* ctx);
        void clear(UIContext* ui);

    };

    class LocalNarrator : public Narrator {
    public:
        LocalNarrator();
        bool operator()(const std::shared_ptr<OnlineGame> &, const std::shared_ptr<Character> &) override;

        std::queue<std::string> m_promptQueue;
    };

    class RemoteNarrator : public Narrator {
    public:
        bool operator()(const std::shared_ptr<OnlineGame> &, const std::shared_ptr<Character> &) override;
    };

} // content

