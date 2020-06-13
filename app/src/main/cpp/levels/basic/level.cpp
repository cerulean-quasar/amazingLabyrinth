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

#include "level.hpp"

namespace basic {
    bool Level::checkBallBorders(glm::vec3 &position, glm::vec3 &velocity) {
        bool wallHit = false;
        if (position.x < -m_width / 2.0f + m_scaleBall * m_originalBallDiameter / 2.0f) {
            // left wall
            position.x = -m_width / 2.0f +
                         m_scaleBall * m_originalBallDiameter * (0.5f /*+ 1/5.0f*/);
            if (velocity.x < 0.0f) {
                if (m_bounce) {
                    velocity.x = -velocity.x;
                } else {
                    velocity.x = 0.0f;
                }
            }
            wallHit = true;
        }

        if (position.x > m_width / 2.0f - m_scaleBall * m_originalBallDiameter / 2.0f) {
            // right wall
            position.x =
                    m_width / 2.0f - m_scaleBall * m_originalBallDiameter * (0.5f /*+ 1/5.0f*/);
            if (velocity.x > 0.0f) {
                if (m_bounce) {
                    velocity.x = -velocity.x;
                } else {
                    velocity.x = 0.0f;
                }
            }
            wallHit = true;
        }

        if (position.y < -m_height / 2.0f + m_scaleBall * m_originalBallDiameter / 2.0f) {
            // bottom wall
            position.y = -m_height / 2.0f +
                         m_scaleBall * m_originalBallDiameter * (0.5f /*+ 1/5.0f*/);
            if (velocity.y < 0.0f) {
                if (m_bounce) {
                    velocity.y = -velocity.y;
                } else {
                    velocity.y = 0.0f;
                }
            }
            wallHit = true;
        }

        if (position.y > m_height / 2.0f - m_scaleBall * m_originalBallDiameter / 2.0f) {
            // top wall
            position.y = m_height / 2.0f -
                         m_scaleBall * m_originalBallDiameter * (0.5f /*+ 1/5.0f*/);
            if (velocity.y > 0.0f) {
                if (m_bounce) {
                    velocity.y = -velocity.y;
                } else {
                    velocity.y = 0.0f;
                }
            }
            wallHit = true;
        }

        return wallHit;
    }

}