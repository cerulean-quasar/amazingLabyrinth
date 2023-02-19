/**
 * Copyright 2022 Cerulean Quasar. All Rights Reserved.
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
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "level.hpp"

namespace starter {
    char constexpr const *Level::corridorImage;
    char constexpr const *Level::corridorBeginImage;
    char constexpr const *Level::corridorEndImage;
    char constexpr const *Level::corridorCornerImage;

    char constexpr const *Level::corridorModel;
    char constexpr const *Level::corridorBeginModel;
    char constexpr const *Level::corridorEndModel;
    char constexpr const *Level::corridorCornerModel;

    char constexpr const *Level::m_name;

    void Level::clearText() {
        text.clear();
    }

    bool Level::updateData() {
        if (m_finished) {
            return false;
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        float difftime = std::chrono::duration<float, std::chrono::seconds::period>(
                currentTime - prevTime).count();
        prevTime = currentTime;

        m_ball.position = getUpdatedPosition(difftime);
        m_ball.velocity = getUpdatedVelocity(difftime);

        if (isInBottomCorridor()) {
            m_ball.position.y = -maxPosY;
            m_ball.velocity.y = 0.0f;
        } else if (isInSideCorridor()) {
            m_ball.position.x = maxPosX;
            m_ball.velocity.x = 0.0f;
        } else { // is in top corridor
            m_ball.position.y = maxPosY;
            m_ball.velocity.y = 0.0f;
        }
        checkBallBorders(m_ball.position, m_ball.velocity);

        if (m_ball.position.x < -maxPosX + errVal && m_ball.position.y == maxPosY) {
            transitionText = true;
            m_ball.position.x = -maxPosX;
            m_ball.position.y = -maxPosY;
            if (textIndex == text.size() - 1) {
                m_finished = true;
                return true;
            }
        }

        updateRotation(difftime);
        return drawingNecessary();
    }

    bool Level::isInBottomCorridor() {
        if (m_ball.position.y < -maxPosY + errVal) {
            if (m_ball.position.x > maxPosX - errVal) {
                return -m_ball.velocity.x >= m_ball.velocity.y;
            }
            return true;
        }
        return false;
    }

    bool Level::isInSideCorridor() {
        if (m_ball.position.x > maxPosX - errVal) {
            if (m_ball.position.y < -maxPosY + errVal) {
                return m_ball.velocity.y >= -m_ball.velocity.x;
            }
            if (m_ball.position.y > maxPosY - errVal) {
                return -m_ball.velocity.y >= -m_ball.velocity.x;
            }
            return true;
        }
        return false;
    }

    bool Level::updateDrawObjects() {
        if (m_finished) {
            return false;
        }

        // the ball
        m_levelDrawer.updateModelMatrixForObject(
                m_objRefBall,
                m_objDataRefBall,
                glm::translate(glm::mat4(1.0f), m_ball.position) *
                glm::mat4_cast(m_ball.totalRotated) * ballScaleMatrix());

        if (transitionText && !m_finished) {
            textIndex++;

            m_levelDrawer.removeObject(m_objRefTextBox);
            m_objRefTextBox = m_levelDrawer.addObject(
                    std::make_shared<levelDrawer::ModelDescriptionQuad>(),
                    std::make_shared<levelDrawer::TextureDescriptionText>(text[textIndex]));

            m_objDataRefTextBox = m_levelDrawer.addModelMatrixForObject(
                    m_objRefTextBox,
                    glm::translate(glm::mat4(1.0f), glm::vec3(-ballRadius(), 0.0f, m_mazeFloorZ)) *
                    glm::scale(glm::mat4(1.0f), textScale));

            transitionText = false;
        }

        return true;
    }
} // namespace starter
