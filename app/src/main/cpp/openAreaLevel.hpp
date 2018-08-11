/**
 * Copyright 2018 Cerulean Quasar. All Rights Reserved.
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

#include "levelFinish.hpp"
#include "level.hpp"

class OpenAreaLevel : public Level {
private:
    std::string holeTexture;
    std::string ballTexture;

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

public:
    OpenAreaLevel() : prevTime(std::chrono::high_resolution_clock::now()) {}
    virtual void loadModels();
    virtual void generate() {
        float smallestDistance = 0.5f;
        do {
            holePosition.x = random.getFloat(-0.7f, 0.7f);
            holePosition.y = random.getFloat(-0.7f, 0.7f);

            ball.position.x = random.getFloat(-1.0f, 1.0f);
            ball.position.y = random.getFloat(-1.0f, 1.0f);
        } while (glm::length(ball.position - holePosition) < smallestDistance);

        unsigned int i = 2;
        float pos = 1.0f/(i*2.0f + 1);
        scale = glm::scale(glm::vec3(pos, pos, pos));

        ball.totalRotated = glm::quat();
        ball.acceleration = {0.0f, 0.0f, 0.0f};
        ball.velocity = {0.0f, 0.0f, 0.0f};
        ball.prevPosition = {-10.0f, 0.0f, -pos};
        ball.position.z = -pos;
        holePosition.z = -pos - pos/2;
    }
    virtual void updateAcceleration(float x, float y, float z);
    virtual glm::vec4 getBackgroundColor() { return {0.0f, 0.0f, 0.0f, 1.0f}; }
    virtual bool updateData();
    virtual void generateModelMatrices();
    virtual bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures);
    virtual bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged);
    virtual void start() {
        prevTime = std::chrono::high_resolution_clock::now();
    }

    void initSetHoleTexture(std::string const &texture) { holeTexture = texture; }
    void initSetBallTexture(std::string const &texture) { ballTexture = texture; }

    void getLevelFinisherCenter(float &x, float &y) {
        x = holePosition.x;
        y = holePosition.y;
    }

    virtual ~OpenAreaLevel() {}
};
#endif