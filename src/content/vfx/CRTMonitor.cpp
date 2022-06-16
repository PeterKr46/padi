//
// Created by Peter on 15/06/2022.
//

#include "CRTMonitor.h"


namespace padi::content {
    CRTMonitor::CRTMonitor(std::shared_ptr<sf::Shader> crtShader)
            : m_crtShader(std::move(crtShader)) {
        if (m_vfxBuffer.create(455, 256)) {
            // TODO error msg
        }
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

        sf::Vector2f imgSize{m_vfxBuffer.getSize()};

        m_screenQuad[0].position = {0, 0};
        m_screenQuad[0].texCoords = {0, imgSize.y};

        m_screenQuad[1].position = {0, monitorSize.y};
        m_screenQuad[1].texCoords = {0, 0};

        m_screenQuad[2].position = monitorSize;
        m_screenQuad[2].texCoords = {imgSize.x, 0};

        m_screenQuad[3].position = {monitorSize.x, 0};
        m_screenQuad[3].texCoords = imgSize;

        m_view.setSize(float(width), float(height));
        m_view.setCenter(monitorSize / 2.f);
    }

    void CRTMonitor::setShader(std::shared_ptr<sf::Shader> crtShader) {
        m_crtShader = std::move(crtShader);
    }

    sf::RenderTarget *CRTMonitor::asTarget() {
        return &m_vfxBuffer;
    }

}