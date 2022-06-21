//
// Created by Peter on 19/06/2022.
//

#pragma once

#include <memory>
#include "../level/CycleListener.h"
#include "SFML/Audio/Sound.hpp"
#include "SFML/System/Vector2.hpp"

namespace padi {
    class AudioPlayback : public padi::CycleListener, public std::enable_shared_from_this<AudioPlayback> {
    public:
        explicit AudioPlayback(std::shared_ptr<sf::SoundBuffer> s);

        bool onFrameEnd(const std::weak_ptr<padi::Level> &lvl, uint8_t frame) override;

        void start(const std::weak_ptr<Level>& lvl);
        void restart(const std::weak_ptr<Level>& lvl);

        void setPosition(sf::Vector2i pos);
        void disableAttenuation();

        sf::Sound sound;
    private:
        std::shared_ptr<sf::SoundBuffer> m_buffer;
        size_t m_frames{0};
        bool m_hasTerminated{true};
    };

}