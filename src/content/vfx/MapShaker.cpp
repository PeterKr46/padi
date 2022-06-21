//
// Created by Peter on 13/06/2022.
//

#include "MapShaker.h"

#include "../../level/Level.h"
#include "../../map/Tile.h"

namespace padi::content {
    bool MapShaker::onFrameBegin(std::weak_ptr<padi::Level> const &lvl, uint8_t frame) {
        ++m_numFrames;
        float scale = 1.f - (float(m_numFrames) / float(m_maxFrames));
        lvl.lock()->getMap()->for_each([&](const std::shared_ptr<padi::Tile> &tile) {
            int s = abs(tile->getPosition().x) + abs(tile->getPosition().y);
            tile->setVerticalOffset(
                    m_magnitude *
                    std::sin(m_offset + float(s) * m_scale + m_frequency * float(m_numFrames)) *
                    scale);
        });

        return m_numFrames < m_maxFrames;
    }
} // content