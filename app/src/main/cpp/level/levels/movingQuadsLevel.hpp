/**
 * Copyright 2019 Cerulean Quasar. All Rights Reserved.
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

#ifndef AMAZING_LABYRINTH_MOVING_QUADS_LEVEL_HPP
#define AMAZING_LABYRINTH_MOVING_QUADS_LEVEL_HPP

#include <cstdint>
#include <vector>
#include <list>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "../../graphics.hpp"
#include "../../random.hpp"
#include "../level.hpp"

class MovingQuadsLevel : public Level {
private:
    static constexpr uint32_t numberOfMidQuadRows = 4;
    static constexpr float scaleFactor = 1.0f/8.0f;
    static constexpr float viscosity = 0.01f;
    static constexpr uint32_t minQuadsInRow = 1;
    static constexpr uint32_t maxQuadsInRow = 3;
    static constexpr float minQuadMovingSpeed = 0.01f;
    static constexpr float maxQuadMovingSpeed = 0.2f;
    static constexpr float spaceBetweenQuadsX = 0.05f;
    float const maxX;
    float const maxY;

    std::string m_startQuadTexture;
    std::string m_ballTexture;
    std::string m_endQuadTexture;
    std::vector<std::string> m_middleQuadTextures;

    std::chrono::high_resolution_clock::time_point m_prevTime;

    glm::vec3 m_endQuadPosition;
    glm::vec3 m_startQuadPosition;
    float m_quadScaleY;

    // the start position of the ball.  The ball goes back here if it falls off the quads.
    glm::vec3 m_startPosition;

    // moving quads.  The goal is to keep the ball on these moving quads.
    struct MovingQuadRow {
        std::vector<glm::vec3> positions;
        float speed;
        glm::vec3 scale;
    };
    std::vector<MovingQuadRow> m_movingQuads;

    /* vertex and index data for drawing the ball. */
    std::vector<Vertex> m_ballVertices;
    std::vector<uint32_t> m_ballIndices;

    /* vertex and index data for drawing the hole. */
    std::vector<Vertex> m_quadVertices;
    std::vector<uint32_t> m_quadIndices;

    // data on where the ball is, how fast it is moving, etc.
    struct {
        glm::vec3 prevPosition;
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec3 acceleration;
        glm::quat totalRotated;
        glm::mat4 scale;
    } m_ball;

    bool ballOnQuad(glm::vec3 const &centerPos, float xSize);

    void loadModels();
    void generate();
public:
    virtual glm::vec4 getBackgroundColor() { return glm::vec4(0.2, 0.2, 1.0, 1.0); }
    virtual void updateAcceleration(float x, float y, float z) { m_ball.acceleration = {-x, -y, 0.0f}; }
    virtual bool updateData();
    virtual bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures);
    virtual bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged);
    virtual void start() {
        m_prevTime = std::chrono::high_resolution_clock::now();
    }

    virtual void getLevelFinisherCenter(float &x, float &y) {
        x = 0.0f;
        y = 0.0f;
    }

    virtual void init() {
        loadModels();
        generate();
    }

    void initSetEndQuadTexture(std::string const &texture) { m_endQuadTexture = texture; }
    void initAddMiddleQuadTexture(std::string const &texture) { m_middleQuadTextures.push_back(texture); }
    void initSetStartQuadTexture(std::string const &texture) { m_startQuadTexture = texture; }
    void initSetBallTexture(std::string const &texture) { m_ballTexture = texture; }

    MovingQuadsLevel(std::shared_ptr<GameRequester> inGameRequester, float width, float height, float maxZ)
            : Level(std::move(inGameRequester), width, height, maxZ),
              maxX(m_width/2),
              maxY(m_height/2),
              m_prevTime(std::chrono::high_resolution_clock::now()) { }
    virtual ~MovingQuadsLevel() = default;
};
#endif /* AMAZING_LABYRINTH_MOVING_QUADS_LEVEL_HPP */