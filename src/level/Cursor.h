//
// Created by Peter on 01/05/2022.
//
#pragma once

#include <SFML/Window/Keyboard.hpp>
#include "../entity/StaticEntity.h"
#include "Level.h"
#include "CycleListener.h"

namespace padi {

    class AudioPlayback;

    class Cursor
            : public CycleListener, public padi::StaticEntity, public std::enable_shared_from_this<Cursor> {
    public:

        static const uint32_t EntityType = 7;

        explicit Cursor(std::shared_ptr<padi::Animation> anim);

        void update(padi::Level *level);

        bool onFrameBegin(std::weak_ptr<padi::Level> const &lvl, uint8_t frame) override;

        void lock();
        void unlock();

        void moved(std::weak_ptr<padi::Level> const& level);

    private:
        bool m_locked{false};
        std::shared_ptr<AudioPlayback> m_beep;
    };

} // padi
