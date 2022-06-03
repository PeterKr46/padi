//
// Created by peter on 03/06/22.
//
#pragma once


#include <cstdint>

namespace padi {
    class Level;

    class CycleListener {
    public:
        virtual bool onCycleBegin(padi::Level *) { return false; }

        virtual bool onCycleEnd(padi::Level *) { return false; }

        virtual bool onFrameBegin(padi::Level *, uint8_t frame) { return false; }

        virtual bool onFrameEnd(padi::Level *, uint8_t frame) { return false; }
    };

}