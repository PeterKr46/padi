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
        sound.setAttenuation(0.0f);
        sound.setRelativeToListener(true);
        sound.setPosition(0,0,0);
    }

    void AudioPlayback::start(const std::weak_ptr<Level> &lvl) {
        if (m_hasTerminated && sound.getStatus() != sf::SoundSource::Playing) {
            sound.play();
            lvl.lock()->addFrameEndListener(shared_from_this());
            m_hasTerminated = false;
            m_frames = 1 + (m_buffer->getDuration().asMicroseconds() / (padi::FrameTime_uS));
        }
    }

    void AudioPlayback::restart(const std::weak_ptr<Level> &lvl) {
        sound.play();
        if (m_hasTerminated) {
            lvl.lock()->addFrameEndListener(shared_from_this());
            m_hasTerminated = false;
        }
        m_frames = 1 + (m_buffer->getDuration().asMicroseconds() / (padi::FrameTime_uS));
    }

    bool AudioPlayback::onFrameEnd(const std::weak_ptr<padi::Level> &lvl, uint8_t frame) {
        if (--m_frames == 0) {
            m_hasTerminated = true;
            return false;
        }
        return true;
    }


    void AudioPlayback::setPosition(sf::Vector2i pos) {
        sf::Vector2f fpos = Map::mapTilePosToWorld(pos);
        sound.setAttenuation(0.01f);
        sound.setRelativeToListener(false);
        sound.setPosition((fpos.x + 0.5f) / float(TileSize.x), (fpos.y + 0.5f) / float(TileSize.y), 0);
    }

    void AudioPlayback::disableAttenuation() {
        sound.setRelativeToListener(true);
        sound.setPosition(0,0,0);
        sound.setAttenuation(0.0f);
    }
}