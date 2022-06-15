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
        target.setView(target.getDefaultView());

        target.draw(m_screenQuad, rState);
    }

    void CRTMonitor::handleResize(int width, int height) {
        sf::Vector2f halfSize{float(width), float(height)};
        halfSize /= 2.f;
        sf::Vector2f imgSize{m_vfxBuffer.getSize()};
        m_screenQuad[0].position = {0, 0};
        m_screenQuad[0].texCoords = {0, imgSize.y};

        m_screenQuad[1].position = {0, float(height)};
        m_screenQuad[1].texCoords = {0, 0};

        m_screenQuad[2].position = {float(width), float(height)};
        m_screenQuad[2].texCoords = {imgSize.x, 0};

        m_screenQuad[3].position = {float(width), 0};
        m_screenQuad[3].texCoords = imgSize;
    }

    void CRTMonitor::setShader(std::shared_ptr<sf::Shader> crtShader) {
        m_crtShader = std::move(crtShader);
    }

    sf::RenderTarget *CRTMonitor::asTarget() {
        return &m_vfxBuffer;
    }

}