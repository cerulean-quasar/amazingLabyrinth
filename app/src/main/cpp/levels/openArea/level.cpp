/**
 * Copyright 2024 Cerulean Quasar. All Rights Reserved.
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

namespace openArea {
    bool Level::updateData() {
        if (m_finished) {
            // the maze is finished, do nothing and return false (drawing is not necessary).
            return false;
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        float timeDiff = std::chrono::duration<float, std::chrono::seconds::period>(
                currentTime - prevTime).count();
        prevTime = currentTime;

        m_ball.velocity = getUpdatedVelocity(timeDiff);
        m_ball.position = getUpdatedPosition(timeDiff);

        float errDistance = ballDiameter();
        if (glm::length(m_ball.position - holePosition) < errDistance) {
            m_finished = true;
            m_ball.position.x = holePosition.x;
            m_ball.position.y = holePosition.y;
            m_ball.velocity = {0.0f, 0.0f, 0.0f};
            return true;
        }

        checkBallBorders(m_ball.position, m_ball.velocity);
        updateRotation(timeDiff);
        modelMatrixBall = glm::translate(glm::mat4(1.0f), m_ball.position) *
                          glm::mat4_cast(m_ball.totalRotated) * scale;

        return drawingNecessary();
    }

    void Level::generateModelMatrices() {
        // the ball
        glm::mat4 trans = glm::translate(glm::mat4(1.0f), m_ball.position);
        modelMatrixBall = trans * scale;

        // the hole
        trans = glm::translate(glm::mat4(1.0f), holePosition);
        modelMatrixHole = trans * scale;
    }

    bool Level::updateDrawObjects() {
        m_levelDrawer.updateModelMatrixForObject(m_objRefBall, m_objDataRefBall, modelMatrixBall);
        return true;
    }
} // namespace openArea