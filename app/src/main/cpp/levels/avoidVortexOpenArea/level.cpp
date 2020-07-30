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
#include <glm/gtc/matrix_transform.hpp>

#include "level.hpp"

namespace avoidVortexOpenArea {
    char constexpr const *Level::m_name;

    void Level::preGenerate() {
        m_ball.position.z = m_mazeFloorZ + ballRadius();
        holePosition.z = m_mazeFloorZ;
        startPosition.z = m_ball.position.z;
    }

    void Level::postGenerate() {
        scale = ballScaleMatrix();

        m_ball.totalRotated = glm::quat();
        m_ball.acceleration = {0.0f, 0.0f, 0.0f};
        m_ball.velocity = {0.0f, 0.0f, 0.0f};

        // set to very large previous position (in vector length) so that it will get drawn
        // the first time through.
        m_ball.prevPosition = {-10.0f, 0.0f, m_mazeFloorZ + ballRadius()};

        startPositionQuad = startPosition;
        startPositionQuad.z = m_mazeFloorZ;
    }

    void Level::generate() {
        /* ensure that the ball and hole are not near each other. They need to be farther apart than
         * the vortexes.  To make the maze fun and harder. */
        float smallestDistance = ballDiameter();
        do {
            holePosition.x = random.getFloat(-maxX, maxX);
            holePosition.y = random.getFloat(-maxY, maxY);

            m_ball.position.x = random.getFloat(-maxX, maxX);
            m_ball.position.y = random.getFloat(-maxY, maxY);
        } while (glm::length(m_ball.position - holePosition) < smallestDistance * 4.0f);
        startPosition = m_ball.position;

        /* ensure that the vortexes are not near each other or the hole or the ball. */
        do {
            glm::vec3 vortexPositionCandidate =
                    {random.getFloat(-maxX, maxX), random.getFloat(-maxY, maxY),
                     m_mazeFloorZ};

            float distance = glm::length(vortexPositionCandidate - m_ball.position);
            if (distance < smallestDistance) {
                continue;
            }

            distance = glm::length(vortexPositionCandidate - holePosition);
            if (distance < smallestDistance) {
                continue;
            }

            for (auto const &vortexPosition : vortexPositions) {
                distance = glm::length(vortexPosition - vortexPositionCandidate);
                if (distance < smallestDistance) {
                    break;
                }
            }

            if (distance >= smallestDistance) {
                vortexPositions.push_back(vortexPositionCandidate);
            }
        } while (vortexPositions.size() < numberOfVortexes);
    }

    bool Level::ballProximity(glm::vec3 const &objPosition) {
        float errDistance = ballDiameter();
        if (glm::length(m_ball.position - objPosition) < errDistance) {
            m_ball.position.x = objPosition.x;
            m_ball.position.y = objPosition.y;
            m_ball.velocity = {0.0f, 0.0f, 0.0f};
            return true;
        }
        return false;
    }

    bool Level::updateData() {
        if (m_finished) {
            // the maze is finished, do nothing and return false (drawing is not necessary).
            return false;
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        float timeDiff = std::chrono::duration<float, std::chrono::seconds::period>(
                currentTime - prevTime).count();
        prevTime = currentTime;

        m_ball.velocity = getUpdatedVelocity(m_ball.acceleration, timeDiff);
        m_ball.position += m_ball.velocity * timeDiff;

        if (ballProximity(holePosition)) {
            m_finished = true;
            return true;
        }

        for (auto &&vortexPosition : vortexPositions) {
            if (ballProximity(vortexPosition)) {
                m_ball.position = startPosition;
                return true;
            }
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

        // the starting vortex (the ball shows up here if it enters a vortex).
        trans = glm::translate(glm::mat4(1.0f), startPositionQuad);
        modelMatrixStartVortex = trans * scale;

        // the vortexes
        for (auto &&vortexPosition : vortexPositions) {
            trans = glm::translate(glm::mat4(1.0f), vortexPosition);
            modelMatrixVortexes.push_back(trans * scale);
        }
    }

    bool Level::updateDrawObjects() {
        m_levelDrawer.updateModelMatrixForObject(m_objRefBall, m_objDataRefBall, modelMatrixBall);
        return true;
    }

} // namespace avoidVortexOpenArea