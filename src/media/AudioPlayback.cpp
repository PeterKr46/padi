//
// Created by Peter on 19/06/2022.
//

#include "AudioPlayback.h"
#include <SFML/Audio.hpp>
#include "../Constants.h"
#include "../level/Level.h"

namespace padi {

    AudioPlayback::AudioPlayback(std::shared_ptr<sf::SoundBuffer> s) : m_buffer(std::move(s)) {
        sound.setBuffer(*m_buffer);
        sound.play();
        m_cycles = 1 + (m_buffer->getDuration().asMicroseconds() / (padi::CycleLength_F * padi::FrameTime_uS));
    }

    void AudioPlayback::restart(padi::Level * lvl) {
        sound.play();
        if(m_hasTerminated) {
            lvl->addCycleEndListener(shared_from_this());
            m_hasTerminated = false;
        }
        m_cycles = 1 + (m_buffer->getDuration().asMicroseconds() / (padi::CycleLength_F * padi::FrameTime_uS));
    }

    bool AudioPlayback::onCycleEnd(padi::Level *) {
        if(m_cycles-- == 0) {
            m_hasTerminated = true;
            return false;
        }
        return true;
    }
}