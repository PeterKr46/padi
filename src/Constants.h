//
// Created by Peter on 01/05/2022.
//

#pragma once

namespace std {
    template<typename S>
    struct std::hash<sf::Vector2<S>> {
        std::hash<S> hash;
        std::size_t operator()(sf::Vector2<S> const &s) const noexcept {
            std::size_t h1 = hash(s.x);
            std::size_t h2 = hash(s.y);
            return h1 ^ (h2 << 1);
        }
    };
}

namespace padi {
    //const unsigned int FrameTime_uS{83333};
    const unsigned int CyclesPerMinute{60};
    const unsigned int CycleLength_F{12};
    const unsigned int FrameTime_uS{60000000 / (CyclesPerMinute * CycleLength_F)};

    const sf::Vector2i Left{-1, 0};
    const sf::Vector2i Right{1, 0};
    const sf::Vector2i Up{0, -1};
    const sf::Vector2i Down{0, 1};

    const sf::Vector2i AllDirections[4]{Left, Right, Up, Down};
    const sf::Vector2i Neighborhood[5]{sf::Vector2i{0,0}, Left, Right, Up, Down};

    const sf::Vector2i TileSize{32,32};
    const size_t UIPadding_px{8};

    const char AddressCharacterSet[] = "0123456789.";
    const char AlphabeticCharacterSet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char AlphabeticNumericCharacterSet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const char SimpleCharacterSet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.!?+-/()%$ ";
}
