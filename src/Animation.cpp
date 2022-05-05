//
// Created by Peter on 28/04/2022.
//

#include "Animation.h"

#include <utility>

namespace padi {
    SimpleAnimation::SimpleAnimation(sf::Vector2i resolution, std::vector<sf::Vector2f> anchors)
            : m_resolution(resolution), m_anchors(std::move(anchors)) {

    }

    sf::Vector2i SimpleAnimation::getResolution() const {
        return m_resolution;
    }

    size_t SimpleAnimation::length() const {
        return m_anchors.size();
    }

    sf::Vector2f SimpleAnimation::operator[](size_t frame) const {
        return m_anchors.at(frame);
    }

    ReverseAnimation::ReverseAnimation(Animation *animation) : m_original(animation) {

    }

    sf::Vector2i ReverseAnimation::getResolution() const {
        return m_original->getResolution();
    }

    size_t ReverseAnimation::length() const {
        return m_original->length();
    }

    sf::Vector2f ReverseAnimation::operator[](size_t frame) const {
        return m_original->operator[](length()-1-frame);
    }

    SimpleAnimation
    StripAnimation(const sf::Vector2i &resolution, const sf::Vector2i &anchor0, const sf::Vector2i &step,
                   size_t num_frames, size_t repeat) {
        auto frames = std::vector<sf::Vector2f>();
        for (size_t loop = 0; loop <= repeat; ++loop) {
            for (size_t i = 0; i < num_frames; ++i) {
                frames.emplace_back(float(anchor0.x + (step.x * i)),
                                    float(anchor0.y + (step.y * i)));
            }
        }
        return {resolution, frames};
    }
}
