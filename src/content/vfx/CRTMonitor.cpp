//
// Created by Peter on 15/06/2022.
//

#include "CRTMonitor.h"
#include <cmath>

namespace padi::content {
#define CRT_WIDTH   int(453)
#define CRT_HEIGHT  int(255)
    
    CRTMonitor::CRTMonitor(std::shared_ptr<sf::Shader> crtShader)
            : m_crtShader(std::move(crtShader)) {
        if (!m_vfxBuffer.create(CRT_WIDTH, CRT_HEIGHT)) {
            printf("[CRTMonitor] ERROR: Failed to create RenderTarget.\r");
        }
        //m_vfxBuffer.setSmooth(true);
    }

    void CRTMonitor::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        auto rState = sf::RenderStates::Default;

        m_crtShader->setUniform("time", m_graphicsClock.getElapsedTime().asSeconds());
        m_crtShader->setUniform("paused", paused);
        rState.shader = m_crtShader.get();
        rState.texture = &m_vfxBuffer.getTexture();

        target.setView(m_view);

        target.draw(m_screenQuad, rState);
    }

    void CRTMonitor::handleResize(int width, int height) {
        sf::Vector2f monitorSize;
        monitorSize.x = std::min<float>(float(width), (16.f / 9.f) * float(height));
        monitorSize.y = std::min<float>(float(height), (9.f / 16.f) * float(width));


        m_screenQuad[0].position = {0, 0};
        m_screenQuad[0].texCoords = {0, CRT_HEIGHT};

        m_screenQuad[1].position = {0, monitorSize.y};
        m_screenQuad[1].texCoords = {0, 1};

        m_screenQuad[2].position = monitorSize;
        m_screenQuad[2].texCoords = {CRT_WIDTH, 0};

        m_screenQuad[3].position = {monitorSize.x, 0};
        m_screenQuad[3].texCoords = {CRT_WIDTH, CRT_HEIGHT};

        m_view.setSize(float(width), float(height));
        m_view.setCenter(monitorSize / 2.f);
    }

    void CRTMonitor::setShader(std::shared_ptr<sf::Shader> crtShader) {
        m_crtShader = std::move(crtShader);
    }

    sf::RenderTarget *CRTMonitor::asTarget() {
        return &m_vfxBuffer;
    }

    void CRTMonitor::handleResize(sf::Vector2u const& d) {
        handleResize(int(d.x), int(d.y));
    }

    sf::Vector2f CRTMonitor::mapWindowToCRTPosition(const sf::Vector2f &window) {
        static const sf::Vector2f curvature{4.8, 3.5};
        sf::Vector2f uv = window * 2.0f - sf::Vector2f(1,1);
        sf::Vector2f offset = sf::Vector2f (
                uv.x * std::pow(std::abs(uv.y) / curvature.x, 2.f),
                uv.y * std::pow(std::abs(uv.x) / curvature.y, 2.f)
                );
        uv += offset;
        uv = uv * 0.5f + sf::Vector2f(0.5f, 0.5f);
        return uv;
    }

    const sf::Clock &CRTMonitor::getGraphicsClock() const {
        return m_graphicsClock;
    }

}