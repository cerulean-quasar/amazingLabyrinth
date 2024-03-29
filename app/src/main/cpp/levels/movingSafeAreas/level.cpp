/**
 * Copyright 2023 Cerulean Quasar. All Rights Reserved.
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

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "level.hpp"
#include "loadData.hpp"

namespace movingSafeAreas {
    void Level::preGenerate() {
        m_quadScaleY = m_height / (numberOfMidQuadRows + 2.0f) / m_quadOriginalSize;
        m_startQuadPosition = {0.0f, -maxY + m_quadScaleY * m_quadOriginalSize / 2.0f,
                               m_mazeFloorZ};
        m_endQuadPosition = {0.0f, maxY - m_quadScaleY * m_quadOriginalSize / 2.0f, m_mazeFloorZ};
        m_startPosition = {0.0f, -maxY + ballRadius(), m_mazeFloorZ + ballRadius()};

        m_ball.totalRotated = glm::quat();
        m_ball.acceleration = {0.0f, 0.0f, 0.0f};
        m_ball.velocity = {0.0f, 0.0f, 0.0f};

        // set to very large previous position (in vector length) so that it will get drawn
        // the first time through.
        m_ball.prevPosition = {-10.0f, 0.0f, m_mazeFloorZ + ballRadius()};
        m_ball.position = m_startPosition;
    }

    void Level::generate() {
        // The moving quads move at a random speed in the x direction.  There is a random number of
        // them in each row.
        Random randomGenerator;
        for (uint32_t i = 0; i < numberOfMidQuadRows; i++) {
            MovingQuadRow row;
            uint32_t numberOfQuadsInRow = randomGenerator.getUInt(minQuadsInRow, maxQuadsInRow);
            uint32_t direction = randomGenerator.getUInt(0, 1) * 2 - 1;
            row.speed = direction *
                        randomGenerator.getFloat(minQuadMovingSpeed(), maxQuadMovingSpeed());

            //float xscale = (m_width/numberOfQuadsInRow - spaceBetweenQuadsX*(numberOfQuadsInRow-1))/m_quadOriginalSize;
            float xscale = m_width / (1.5f * numberOfQuadsInRow + 0.5f) / m_quadOriginalSize;
            row.scale = {xscale, m_quadScaleY, 1.0f};
            float xpos = randomGenerator.getFloat(-maxX, maxX - xscale *
                                                                (1.5f * numberOfQuadsInRow - 0.5f) *
                                                                m_quadOriginalSize);
            for (uint32_t j = 0; j < numberOfQuadsInRow; j++) {
                /*
                glm::vec3 pos{row.scale.x*m_quadOriginalSize*j + spaceBetweenQuadsX * j - xpos,
                              m_quadScaleY*m_quadOriginalSize*(i+1.5f)-maxY,
                              m_mazeFloorZ};
                              */
                glm::vec3 pos{row.scale.x * m_quadOriginalSize * (1.5f * j) + xpos,
                              m_quadScaleY * m_quadOriginalSize * (i + 1.5f) - maxY,
                              m_mazeFloorZ};
                row.positions.push_back(pos);
            }

            m_movingQuads.push_back(row);
        }
    }

    bool Level::ballOnQuad(glm::vec3 const &quadCenterPos, float xSize) {
        return m_ball.position.x < quadCenterPos.x + xSize / 2 &&
               m_ball.position.x > quadCenterPos.x - xSize / 2 &&
               m_ball.position.y <
               quadCenterPos.y + m_quadScaleY * m_quadOriginalSize / 2 + ballRadius() &&
               m_ball.position.y >
               quadCenterPos.y - m_quadScaleY * m_quadOriginalSize / 2 - ballRadius();
    }

    bool Level::updateData() {
        if (m_finished) {
            // the maze is finished, do nothing and return false (drawing is not necessary).
            return false;
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        float timeDiff = std::chrono::duration<float, std::chrono::seconds::period>(
                currentTime - m_prevTime).count();
        m_prevTime = currentTime;

        m_ball.position = getUpdatedPosition(timeDiff);
        m_ball.velocity = getUpdatedVelocity(m_ball.acceleration, timeDiff);

        // if the ball is on the end quad, then the user won.
        if (ballOnQuad(m_endQuadPosition, 2 * maxX)) {
            m_finished = true;
            return true;
        }

        // If the ball is not on any quads, then it goes back to the beginning.
        bool isOnQuads = false;
        if (ballOnQuad(m_startQuadPosition, 2 * maxX)) {
            isOnQuads = true;
        } else {
            for (auto const &movingQuadRow : m_movingQuads) {
                for (auto const &movingQuadPos : movingQuadRow.positions) {
                    if (ballOnQuad(movingQuadPos, movingQuadRow.scale.x * m_quadOriginalSize)) {
                        isOnQuads = true;
                        break;
                    }
                }
            }
        }

        if (!isOnQuads) {
            m_ball.position = m_startPosition;
            m_ball.velocity = {0.0, 0.0, 0.0};
            m_ball.acceleration = {0.0, 0.0, 0.0};
        }

        // Move the moving quads.
        for (auto &movingQuadRow : m_movingQuads) {
            for (auto &movingQuadPos : movingQuadRow.positions) {
                movingQuadPos.x += movingQuadRow.speed * timeDiff;
            }
            if ((movingQuadRow.positions[0].x < -maxX && movingQuadRow.speed < 0) ||
                (movingQuadRow.positions[movingQuadRow.positions.size() - 1].x > maxX &&
                 movingQuadRow.speed > 0)) {
                movingQuadRow.speed = -movingQuadRow.speed;
            }
        }

        checkBallBorders(m_ball.position, m_ball.velocity);
        updateRotation(timeDiff);

        timeDiffSinceLastMove += timeDiff;
        if (timeDiffSinceLastMove > 0.01f || drawingNecessary()) {
            timeDiffSinceLastMove = 0.0f;
            return true;
        }

        return false;
    }

    bool Level::updateDrawObjects() {
        // the ball
        m_levelDrawer.updateModelMatrixForObject(
                m_objRefBall,
                m_objDataRefBall,
                glm::translate(glm::mat4(1.0f), m_ball.position) *
                                          glm::mat4_cast(m_ball.totalRotated) *
                                          ballScaleMatrix());

        // the moving quads
        for (size_t i = 0; i < m_objDataRefQuad.size(); i++) {
            for (size_t j = 0; j < m_objDataRefQuad[i].size(); j++) {
                m_levelDrawer.updateModelMatrixForObject(
                        m_objRefQuad[i],
                        m_objDataRefQuad[i][j],
                        glm::translate(glm::mat4(1.0f), m_movingQuads[i].positions[j]) *
                        glm::scale(glm::mat4(1.0f), m_movingQuads[i].scale));
            }
        }
        return true;
    }

} // namespace movingSafeAreas