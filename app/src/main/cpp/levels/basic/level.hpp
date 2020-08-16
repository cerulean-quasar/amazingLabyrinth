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

#ifndef AMAZING_LABYRINTH_BASIC_LEVEL_HPP
#define AMAZING_LABYRINTH_BASIC_LEVEL_HPP

#include <functional>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../../common.hpp"
#include "loadData.hpp"
#include "../../levelTracker/types.hpp"
#include "../../mathGraphics.hpp"
#include "../../levelDrawer/levelDrawer.hpp"
#include "../../levelDrawer/modelTable/modelLoader.hpp"
#include "../../levelDrawer/textureTable/textureLoader.hpp"

namespace basic {
    class Level {
    private:
        glm::vec3 dragForce() {
            if (glm::length(m_ball.velocity) < m_lengthTooSmallToNormalize) {
                return glm::vec3{0.0f, 0.0f, 0.0f};
            }
            return -m_dragConstant * glm::dot(m_ball.velocity, m_ball.velocity) *
                   glm::normalize(m_ball.velocity);
        }

    protected:
        static float constexpr m_originalBallDiameter = 2.0f;
        static float constexpr m_dragConstant = 0.2f;
        static float constexpr m_accelerationAdjustment = 0.25f;
        static float constexpr m_lengthTooSmallToNormalize = 0.001f;
        static float constexpr m_modelSize = 2.0f;

        std::shared_ptr<GameRequester> m_gameRequester;
        levelDrawer::Adaptor m_levelDrawer;
        bool m_finished;
        float m_width;
        float m_height;
        float m_diagonal;
        float const m_mazeFloorZ;
        bool const m_ignoreZMovement;
        float m_scaleBall;
        bool m_bounce;
        std::string m_ballModel;
        std::string m_ballTexture;

        // data on where the ball is, how fast it is moving, etc.
        struct {
            glm::vec3 prevPosition;
            glm::vec3 position;
            glm::vec3 velocity;
            glm::vec3 acceleration;
            glm::quat totalRotated;
        } m_ball;

        float ballRadius() { return m_originalBallDiameter * m_scaleBall / 2.0f; }

        float ballDiameter() { return m_originalBallDiameter * m_scaleBall; }

        glm::mat4 ballScaleMatrix() { return glm::scale(glm::mat4(1.0f),
                                                        glm::vec3(m_scaleBall, m_scaleBall,
                                                                  m_scaleBall));
        }

        glm::vec3 getUpdatedVelocity(glm::vec3 const &acceleration, float timeDiff) {
            glm::vec3 drag = dragForce();

            glm::vec3 velocity = m_ball.velocity;
            if (glm::length(velocity) < glm::length(drag * timeDiff)) {
                velocity = acceleration * timeDiff;
            } else {
                velocity += acceleration * timeDiff + drag * timeDiff;
            }

            return velocity;
        }

        bool drawingNecessary() {
            bool _drawingNecessary =
                    glm::length(m_ball.position - m_ball.prevPosition) > m_diagonal / 200.0f;
            if (_drawingNecessary) {
                m_ball.prevPosition = m_ball.position;
            }

            return _drawingNecessary;
        }

        void updateRotation(float timeDiff) {
            glm::vec3 axis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), m_ball.velocity);
            if (glm::length(axis) > m_lengthTooSmallToNormalize) {
                float scaleFactor = 10.0f;
                glm::quat q = glm::angleAxis(glm::length(m_ball.velocity) * timeDiff * scaleFactor,
                                             glm::normalize(axis));

                m_ball.totalRotated = glm::normalize(q * m_ball.totalRotated);
            }
        }

        bool checkBallBorders(glm::vec3 &position, glm::vec3 &velocity);
    public:
        static float constexpr m_floatErrorAmount = 0.0001f;

        void updateAcceleration(float x, float y, float z) {
            m_ball.acceleration =
                    m_accelerationAdjustment * glm::vec3{-x, -y, m_ignoreZMovement ? 0 : -z};
        }

        virtual bool updateData() = 0;

        virtual bool updateDrawObjects() = 0;

        virtual void start() = 0;

        bool isFinished() { return m_finished; }

        virtual void getLevelFinisherCenter(float &x, float &y, float &z) = 0;

        virtual bool tap(float, float) { return false; }

        virtual bool drag(float, float, float, float) { return false; }

        virtual bool dragEnded(float, float) { return false; }

        virtual float getZForTapCoords() { return m_mazeFloorZ; }

        virtual char const *name() = 0;

        virtual std::vector<uint8_t> saveData(levelTracker::GameSaveData const &gsd, char const *saveLevelDataKey) = 0;

        Level(
                levelDrawer::Adaptor inLevelDrawer,
                std::shared_ptr<LevelConfigData> const &lcd,
                float mazeFloorZ,
                bool ignoreZMovement)
                : m_levelDrawer{std::move(inLevelDrawer)},
                  m_finished(false),
                  m_mazeFloorZ{mazeFloorZ},
                  m_ignoreZMovement{ignoreZMovement},
                  m_bounce{lcd->bounceEnabled},
                  m_ballModel{lcd->ballModel},
                  m_ballTexture{lcd->ballTexture}
        {
            if (!lcd) {
                throw (std::runtime_error("Level Configuration missing"));
            }

            m_levelDrawer.requestRenderDetails(shadowsChainingRenderDetailsName);

            auto projView = m_levelDrawer.getProjectionView();
            auto wh = getWidthHeight(mazeFloorZ, projView.first, projView.second);
            m_width = wh.first;
            m_height = wh.second;
            m_diagonal = glm::length(glm::vec2{m_width, m_height});
            m_scaleBall = lcd->ballSizeDiagonalRatio * m_diagonal;
            m_ball.totalRotated = glm::quat();
            m_ball.acceleration = {0.0f, 0.0f, 0.0f};
            m_ball.velocity = {0.0f, 0.0f, 0.0f};

            // put am out of bounds previous position so that we are sure to draw on the first
            // draw cycle.
            m_ball.prevPosition = {-10.0f, 0.0f, 0.0f};

            // the derived level will change this
            m_ball.position = {0.0f, 0.0f, 0.0f};
        }

        virtual ~Level() = default;
    };
} // namespace basic
#endif // AMAZING_LABYRINTH_BASIC_LEVEL_HPP
