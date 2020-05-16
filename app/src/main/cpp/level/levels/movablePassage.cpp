/**
 * Copyright 2020 Cerulean Quasar. All Rights Reserved.
 *
 *  This file is part of AmazingLabyrinth.
 *
 *  AmazingLabyrinth is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  AmazingLabyrinth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with AmazingLabyrinth.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "movablePassage.hpp"

void MovablePassage::initDone() {
    if (m_nbrComponents == 0 || m_nbrTilesX == 0 || m_nbrTilesY == 0) {
        throw std::runtime_error("MovablePassage maze not properly initialized");
    }

    float tileSizeX = m_width/m_nbrTilesX;
    float tileSizeY = m_height/(m_nbrTilesY + m_nbrTileRowsForStart + m_nbrTileRowsForEnd);
    float tileSize = tileSizeX < tileSizeY ? tileSizeX : tileSizeY;
    uint32_t nbrExtraTileRowsX = static_cast<uint32_t>(std::floor(
            (m_width - tileSize * m_nbrTilesX) / tileSize));
    uint32_t nbrExtraTilesX = nbrExtraTileRowsX * m_nbrTilesY;

    // minus 2 * nbrExtraTileRowsY for up tunnel components
    uint32_t nbrExtraTileRowsY = static_cast<uint32_t>(std::floor(
            (m_height - tileSize * m_nbrTilesY) / tileSize));
    uint32_t nbrExtraTilesY = nbrExtraTileRowsY * m_nbrTilesX - 2 * nbrExtraTileRowsY;

    // we have a device with a screen of a similar size to our tunnel building area,
    // get some more space by making the movable space less.
    if (nbrExtraTilesX + nbrExtraTilesY < m_nbrComponents) {
        uint32_t moreExtraComponentsRequired = m_nbrComponents - (nbrExtraTilesX + nbrExtraTilesY);
        uint32_t nbrExtraPerimeters = moreExtraComponentsRequired/(2*m_nbrTilesX + 2*m_nbrTilesY) + 1;
        tileSizeX = m_width/(m_nbrTilesX + 2 * nbrExtraPerimeters);
        tileSizeY = m_height/(m_nbrTilesY + m_nbrTileRowsForStart + m_nbrTileRowsForEnd + 2 * nbrExtraPerimeters);
        tileSize = tileSizeX < tileSizeY ? tileSizeX : tileSizeY;

        nbrExtraTileRowsX = static_cast<uint32_t>(std::floor(
                (m_width - tileSize * m_nbrTilesX) / tileSize));
        nbrExtraTilesX = nbrExtraTileRowsX * m_nbrTilesX;

        // minus 2 * nbrExtraTileRowsY for up tunnel components
        nbrExtraTileRowsY = static_cast<uint32_t>(std::floor(
                (m_height - tileSize * m_nbrTilesY) / tileSize));
        nbrExtraTilesY = nbrExtraTileRowsY * m_nbrTilesX - 2 * nbrExtraTileRowsY;
    }

    for (uint32_t k = 0; k < nbrExtraTileRowsY; k++) {
        if (k % 2 == 0) {
            glm::vec3 pos{-m_width / 2 + (m_nbrTilesX + nbrExtraTileRowsY) / 2 * tileSize,
                          m_height / 2 - tileSize/2 - m_nbrTileRowsForEnd*tileSize - tileSize * k/2,
                          m_mazeFloorZ};
            m_fixedComponents.add(pos);
        } else {
            glm::vec3 pos{-m_width / 2 + (m_nbrTilesX + nbrExtraTileRowsY) / 2 * tileSize,
                          -m_height / 2 + tileSize/2 + m_nbrTileRowsForStart*tileSize + tileSize * k/2,
                          m_mazeFloorZ};
            m_fixedComponents.add(pos);
        }
    }

    uint32_t i = 0;
    uint32_t j = 0;
    bool addingTopBottom = true;
    for (auto &component : m_components) {
        for (auto &pos : component.m_placements) {
            pos.m_position.z = m_mazeFloorZ;
            if (addingTopBottom) {
                if (i == (m_nbrTilesX + nbrExtraTileRowsY)/2) {
                    // skip the mid row position because it is used for the fixed tunnels.
                    i++;
                }
                pos.m_position.y = (j % 2 == 0) ?
                       m_height/2 - tileSize/2 - m_nbrTileRowsForEnd * tileSize - tileSize * j/2 :
                       -m_height/2 + tileSize/2 - m_nbrTileRowsForStart * tileSize - tileSize * j/2;
                pos.m_position.x = -m_width / 2 + tileSize / 2 + tileSize * i;
                i++;
                if (i > m_nbrTilesX + nbrExtraTileRowsY) {
                    j++;
                    i = 0;
                    if (j > nbrExtraTileRowsY) {
                        addingTopBottom = false;
                        j = 0;
                        i = 0;
                    }
                }
            } else {
                pos.m_position.x = (i % 2 == 0) ?
                                   m_width / 2 - tileSize / 2 - tileSize * i / 2 :
                                   -m_width / 2 + tileSize / 2 + tileSize * i / 2;
                pos.m_position.y = -m_height/2 +
                        (1/2.0f + m_nbrTileRowsForStart + nbrExtraTileRowsY + j)*tileSize;
                j++;
                if (j > m_nbrTilesY) {
                    i++;
                    j = 0;
                }
            }
        }
    }

    m_tileSize = tileSize;
}
