//
// Created by Peter on 01/07/2022.
//

#include "Utils.h"

namespace padi {


    sf::Vector2f round(sf::Vector2f f) {
        f.x = std::round(f.x);
        f.y = std::round(f.y);
        return f;
    }

    uint32_t rgb(int r, int g, int b) {
        return (r << 24) | (g << 16) | (b << 8) | 0xFF;
    }

    uint32_t hsv(int hue, float sat, float val) {
        hue %= 360;
        while (hue < 0) hue += 360;

        if (sat < 0.f) sat = 0.f;
        if (sat > 1.f) sat = 1.f;

        if (val < 0.f) val = 0.f;
        if (val > 1.f) val = 1.f;

        int h = hue / 60;
        float f = float(hue) / 60 - h;
        float p = val * (1.f - sat);
        float q = val * (1.f - sat * f);
        float t = val * (1.f - sat * (1 - f));

        switch (h) {
            default:
            case 0:
            case 6:
                return rgb(val * 255, t * 255, p * 255);
            case 1:
                return rgb(q * 255, val * 255, p * 255);
            case 2:
                return rgb(p * 255, val * 255, t * 255);
            case 3:
                return rgb(p * 255, q * 255, val * 255);
            case 4:
                return rgb(t * 255, p * 255, val * 255);
            case 5:
                return rgb(val * 255, p * 255, q * 255);
        }
    }

    size_t hash_c_string(const char *p, size_t s) {
        size_t result = 0;
        const size_t prime = 31;
        for (size_t i = 0; i < s; ++i) {
            result = p[i] + (result * prime);
        }
        return result;
    }
}