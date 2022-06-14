//
// Created by Peter on 13/06/2022.
//

#pragma once

#include "../../level/CycleListener.h"

namespace padi::content {

    class MapShaker : public CycleListener{
    public:
        bool onFrameBegin(padi::Level *, uint8_t frame) override;
    private:
        int m_numFrames = 0;
        int m_maxFrames = 14;
        float m_frequency = 0.45;
        float m_scale = 0.1;
        float m_magnitude = 128;
    };

} // content
