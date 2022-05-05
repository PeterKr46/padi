//
// Created by Peter on 01/05/2022.
//

#pragma once

namespace padi {
    //const unsigned int FrameTime_uS{83333};
    const unsigned int CyclesPerMinute{60};
    const unsigned int FrameTime_uS{60000000 / (CyclesPerMinute * 12)};
    const unsigned int CycleLength_F{12};

    const sf::Vector2i Left{-1, 0};
    const sf::Vector2i Right{1, 0};
    const sf::Vector2i Up{0, -1};
    const sf::Vector2i Down{0, 1};

    const size_t UIPadding_px{8};
}