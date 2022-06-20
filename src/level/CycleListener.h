//
// Created by peter on 03/06/22.
//
#pragma once


#include <cstdint>

namespace padi {
    class Level;

    class CycleListener {
    public:
        virtual bool onCycleBegin(std::weak_ptr<padi::Level> const &lvl) { return false; }

        virtual bool onCycleEnd(std::weak_ptr<padi::Level> const &lvl) { return false; }

        virtual bool onFrameBegin(std::weak_ptr<padi::Level> const &lvl, uint8_t frame) { return false; }

        virtual bool onFrameEnd(std::weak_ptr<padi::Level> const &lvl, uint8_t frame) { return false; }
    };

}