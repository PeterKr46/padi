//
// Created by Peter on 13/06/2022.
//

#include "MapShaker.h"

#include "../../level/Level.h"
#include "../../map/Tile.h"

namespace padi::content {
    bool MapShaker::onFrameBegin(padi::Level * lvl, uint8_t frame) {
        ++m_numFrames;
        lvl->getMap()->for_each([&](const std::shared_ptr<padi::Tile>& tile) {
            int s = tile->getPosition().x + tile->getPosition().y;
            tile->setVerticalOffset( m_magnitude * std::sin(m_offset + float(s) * m_scale + m_frequency * float(m_numFrames)) * (1.f - (float(m_numFrames) / float(m_maxFrames))));
        });

        return m_numFrames < m_maxFrames;
    }
} // content