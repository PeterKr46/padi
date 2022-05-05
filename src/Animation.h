//
// Created by Peter on 28/04/2022.
//

#pragma once

#include <vector>
#include <memory>
#include "SFML/System/Vector2.hpp"

namespace padi {

    class Animation {
    public:

        [[nodiscard]] virtual size_t length() const = 0;

        [[nodiscard]] virtual sf::Vector2i getResolution() const = 0;

        virtual sf::Vector2f operator[](size_t frame) const = 0;
    };

    class ReverseAnimation : public Animation {
    public:
        explicit ReverseAnimation(std::shared_ptr<padi::Animation> animation);

        [[nodiscard]] sf::Vector2i getResolution() const override;

        [[nodiscard]] size_t length() const override;

        sf::Vector2f operator[](size_t frame) const override;

    private:
        std::shared_ptr<padi::Animation> m_original;
    };

    class SimpleAnimation : public Animation {
    public:
        SimpleAnimation(sf::Vector2i resolution, std::vector<sf::Vector2f> anchors);

        [[nodiscard]] size_t length() const override;

        [[nodiscard]] sf::Vector2i getResolution() const override;

        sf::Vector2f operator[](size_t frame) const override;

    private:
        sf::Vector2i m_resolution;
        std::vector<sf::Vector2f> m_anchors;
    };

    class StaticAnimation : public Animation {
    public:
        StaticAnimation(sf::Vector2i resolution, sf::Vector2f anchor);

        [[nodiscard]] size_t length() const override;

        [[nodiscard]] sf::Vector2i getResolution() const override;

        sf::Vector2f operator[](size_t frame) const override;

    private:
        sf::Vector2i m_resolution;
        sf::Vector2f m_anchor;
    };

    SimpleAnimation
    StripAnimation(sf::Vector2i const &resolution, sf::Vector2i const &anchor0, sf::Vector2i const &step,
                   size_t num_frames, size_t repeat = 0);

}
