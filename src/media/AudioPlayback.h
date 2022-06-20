//
// Created by Peter on 19/06/2022.
//

#pragma once

#include <memory>
#include "../level/CycleListener.h"
#include "SFML/Audio/Sound.hpp"

namespace padi {
    class AudioPlayback : public padi::CycleListener, public std::enable_shared_from_this<AudioPlayback> {
    public:
        explicit AudioPlayback(std::shared_ptr<sf::SoundBuffer> s);

        bool onCycleEnd(std::weak_ptr<padi::Level> const &lvl) override;

        void start(const std::weak_ptr<Level>& lvl);
        void restart(const std::weak_ptr<Level>& lvl);

        sf::Sound sound;
    private:
        std::shared_ptr<sf::SoundBuffer> m_buffer;
        size_t m_cycles;
        bool m_hasTerminated{true};
    };

}