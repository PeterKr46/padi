//
// Created by Peter on 15/06/2022.
//

#pragma once

#include <memory>
#include <utility>

#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/VertexArray.hpp"
#include "SFML/Graphics/Shader.hpp"
#include "SFML/System/Clock.hpp"

namespace padi::content {
    class CRTMonitor : public sf::Drawable {
    public:
        explicit CRTMonitor(std::shared_ptr<sf::Shader> crtShader = nullptr);

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

        void setShader(std::shared_ptr<sf::Shader> crtShader);

        void handleResize(int width, int height);
        void handleResize(sf::Vector2u const& dims);

        sf::RenderTarget* asTarget();

        static sf::Vector2f mapWindowToCRTPosition(sf::Vector2f const& window);

        const sf::Clock & getGraphicsClock() const;

    public:
        bool paused{false};
    private:
        sf::RenderTexture m_vfxBuffer;
        sf::VertexArray m_screenQuad{sf::Quads, 4};
        sf::View m_view;
        sf::Clock m_graphicsClock;
        std::shared_ptr<sf::Shader> m_crtShader;
    };
}