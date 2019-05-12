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
#ifndef AMAZING_LABYRINTH_OPEN_AREA_LEVEL_HPP
#define AMAZING_LABYRINTH_OPEN_AREA_LEVEL_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "../levelFinish.hpp"
#include "../level.hpp"

class OpenAreaLevel : public Level {
private:
    std::string holeTexture;
    std::string ballTexture;

    float const ballScale;
    float const viscosity = 0.005f;
    Random random;
    std::chrono::high_resolution_clock::time_point prevTime;

    glm::vec3 holePosition;

    /* vertex and index data for drawing the ball. */
    std::vector<Vertex> ballVertices;
    std::vector<uint32_t> ballIndices;

    /* vertex and index data for drawing the hole. */
    std::vector<Vertex> holeVertices;
    std::vector<uint32_t> holeIndices;

    glm::mat4 modelMatrixHole;
    glm::mat4 modelMatrixBall;

    glm::mat4 scale;

    // data on where the ball is, how fast it is moving, etc.
    struct {
        glm::vec3 prevPosition;
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec3 acceleration;
        glm::quat totalRotated;
    } ball;

    void loadModels();
    void generate() {
        scale = glm::scale(glm::vec3(ballScale/m_originalBallDiameter,
                ballScale/m_originalBallDiameter, ballScale/m_originalBallDiameter));

        ball.totalRotated = glm::quat();
        ball.acceleration = {0.0f, 0.0f, 0.0f};
        ball.velocity = {0.0f, 0.0f, 0.0f};
        ball.prevPosition = {-10.0f, 0.0f, m_maxZ-ballScale/2};
        ball.position.z = m_maxZ-ballScale/2;
        holePosition.z = m_maxZ-ballScale;

        float smallestDistance = 0.5f;
        do {
            holePosition.x = random.getFloat(-m_width/2+ballScale/2, m_width/2-ballScale/2);
            holePosition.y = random.getFloat(-m_height/2+ballScale/2, m_height/2-ballScale/2);

            ball.position.x = random.getFloat(-m_width/2+ballScale/2, m_width/2-ballScale/2);
            ball.position.y = random.getFloat(-m_height/2+ballScale/2, m_height/2-ballScale/2);
        } while (glm::length(ball.position - holePosition) < smallestDistance);
    }
    void generateModelMatrices();

public:
    OpenAreaLevel(std::shared_ptr<GameRequester> inGameRequester, float width, float height, float maxZ)
            : Level(std::move(inGameRequester), width, height, maxZ), ballScale(m_width/10.0f),
            prevTime(std::chrono::high_resolution_clock::now()) {}
    virtual void updateAcceleration(float x, float y, float z);
    virtual glm::vec4 getBackgroundColor() { return {0.0f, 0.0f, 0.0f, 1.0f}; }
    virtual bool updateData();
    virtual bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures);
    virtual bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged);
    virtual void start() {
        prevTime = std::chrono::high_resolution_clock::now();
    }

    virtual void init() {
        loadModels();
        generate();
        generateModelMatrices();
    }

    void initSetHoleTexture(std::string const &texture) { holeTexture = texture; }
    void initSetBallTexture(std::string const &texture) { ballTexture = texture; }

    void getLevelFinisherCenter(float &x, float &y) {
        x = holePosition.x;
        y = holePosition.y;
    }

    virtual ~OpenAreaLevel() {}
};
#endif /* AMAZING_LABYRINTH_OPEN_AREA_LEVEL_HPP */
