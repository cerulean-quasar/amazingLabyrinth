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

#ifndef AMAZING_LABYRINTH_LEVEL_HPP
#define AMAZING_LABYRINTH_LEVEL_HPP

#include <functional>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../common.hpp"
#include "levelFinish.hpp"
#include "../saveData.hpp"

class Level {
private:
    glm::vec3 dragForce() {
        if (glm::length(m_ball.velocity) < m_lengthTooSmallToNormalize) {
            return glm::vec3{0.0f, 0.0f, 0.0f};
        }
        return -m_dragConstant * glm::dot(m_ball.velocity, m_ball.velocity) * glm::normalize(m_ball.velocity);
    }
protected:
    static float constexpr m_originalBallDiameter = 2.0f;
    static float constexpr m_dragConstant = 0.2f;
    static float constexpr m_accelerationAdjustment = 0.25f;
    static float constexpr m_lengthTooSmallToNormalize = 0.001f;
    static float constexpr m_floatErrorAmount = 0.0001f;
    static float constexpr m_modelSize = 2.0f;

    std::shared_ptr<GameRequester> m_gameRequester;
    bool m_finished;
    float const m_width;
    float const m_height;
    float const m_diagonal;
    float const m_mazeFloorZ;
    float const m_scaleBall;
    bool const m_ignoreZMovement;
    bool m_bounce;

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
    glm::mat4 ballScaleMatrix() { return glm::scale(glm::mat4(1.0f), glm::vec3(m_scaleBall, m_scaleBall, m_scaleBall)); }

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
        bool _drawingNecessary = glm::length(m_ball.position - m_ball.prevPosition) > m_diagonal/200.0f;
        if (_drawingNecessary) {
            m_ball.prevPosition = m_ball.position;
        }

        return _drawingNecessary;
    }

    void updateRotation(float timeDiff) {
        glm::vec3 axis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), m_ball.velocity);
        if (glm::length(axis) > m_lengthTooSmallToNormalize) {
            float scaleFactor = 10.0f;
            glm::quat q = glm::angleAxis(glm::length(m_ball.velocity)*timeDiff*scaleFactor, glm::normalize(axis));

            m_ball.totalRotated = glm::normalize(q * m_ball.totalRotated);
        }
    }

    bool checkBallBorders(glm::vec3 &position, glm::vec3 &velocity) {
        bool wallHit = false;
        if (position.x < -m_width/2.0f + m_scaleBall * m_originalBallDiameter  / 2.0f) {
            // left wall
            position.x = -m_width/2.0f + m_scaleBall * m_originalBallDiameter * (0.5f /*+ 1/5.0f*/);
            if (velocity.x < 0.0f) {
                if (m_bounce) {
                    velocity.x = -velocity.x;
                } else {
                    velocity.x = 0.0f;
                }
            }
            wallHit = true;
        }

        if (position.x > m_width/2.0f - m_scaleBall * m_originalBallDiameter  / 2.0f) {
            // right wall
            position.x = m_width/2.0f - m_scaleBall * m_originalBallDiameter * (0.5f /*+ 1/5.0f*/);
            if (velocity.x > 0.0f) {
                if (m_bounce) {
                    velocity.x = -velocity.x;
                } else {
                    velocity.x = 0.0f;
                }
            }
            wallHit = true;
        }

        if (position.y < -m_height/2.0f + m_scaleBall * m_originalBallDiameter / 2.0f) {
            // bottom wall
            position.y = -m_height/2.0f + m_scaleBall * m_originalBallDiameter * (0.5f /*+ 1/5.0f*/);
            if (velocity.y < 0.0f) {
                if (m_bounce) {
                    velocity.y = -velocity.y;
                } else {
                    velocity.y = 0.0f;
                }
            }
            wallHit = true;
        }

        if (position.y > m_height/2.0f - m_scaleBall * m_originalBallDiameter  / 2.0f) {
            // top wall
            position.y = m_height/2.0f - m_scaleBall * m_originalBallDiameter * (0.5f /*+ 1/5.0f*/);
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

public:
    virtual glm::vec4 getBackgroundColor() = 0;
    void updateAcceleration(float x, float y, float z) {
        m_ball.acceleration = m_accelerationAdjustment * glm::vec3{-x, -y, m_ignoreZMovement ? 0 :-z};
    }
    virtual bool updateData() = 0;
    virtual bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) = 0;
    virtual bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged) = 0;
    virtual void start() = 0;
    bool isFinished() { return m_finished; }
    virtual void getLevelFinisherCenter(float &x, float &y) {
        x = 0.0f;
        y = 0.0f;
    }

    using SaveLevelDataFcn = std::function<std::vector<uint8_t>(std::shared_ptr<GameSaveData> gsd)>;
    virtual SaveLevelDataFcn getSaveLevelDataFcn();
    static SaveLevelDataFcn  getBasicSaveLevelDataFcn();

    Level(
        std::shared_ptr<GameRequester> inGameRequester,
        float width,
        float height,
        float mazeFloorZ,
        bool ignoreZMovement,
        float ballScaleFactorDiagonal,
        bool bounce = true)
        : m_gameRequester{std::move(inGameRequester)},
          m_finished(false),
          m_width(width),
          m_height(height),
          m_diagonal{glm::length(glm::vec2{m_width, m_height})},
          m_mazeFloorZ{mazeFloorZ},
          m_scaleBall{ballScaleFactorDiagonal * m_diagonal},
          m_ignoreZMovement{ignoreZMovement},
          m_bounce{bounce}
    {
        m_ball.totalRotated = glm::quat();
        m_ball.acceleration = {0.0f, 0.0f, 0.0f};
        m_ball.velocity = {0.0f, 0.0f, 0.0f};
        m_ball.prevPosition = {-10.0f, 0.0f, m_mazeFloorZ + m_scaleBall * m_originalBallDiameter / 2.0f};
        m_ball.position = {0.0f, 0.0f, 0.0f};
    }

    virtual ~Level() = default;
};
#endif /* AMAZING_LABYRINTH_LEVEL_HPP */
