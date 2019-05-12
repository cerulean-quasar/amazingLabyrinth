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

#ifndef AMAZING_LABYRINTH_AVOID_VORTEX_LEVEL_HPP
#define AMAZING_LABYRINTH_AVOID_VORTEX_LEVEL_HPP

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

class AvoidVortexLevel : public Level {
private:
    static constexpr uint32_t numberOfVortexes = 8;
    static constexpr float scaleFactor = 1.0f/16.0f;

    std::string holeTexture;
    std::string ballTexture;
    std::string vortexTexture;
    std::string startVortexTexture;

    static constexpr float viscosity = 0.005f;
    float const maxX;
    float const maxY;
    Random random;
    std::chrono::high_resolution_clock::time_point prevTime;

    glm::vec3 holePosition;

    // the start position of the ball.  The ball goes back here if it touches a vortex.
    glm::vec3 startPosition;

    // the position of the quad being displayed to designate the start position of the ball.
    glm::vec3 startPositionQuad;

    // if the ball touches these vortexes, it goes back to startPosition.
    std::vector<glm::vec3> vortexPositions;

    /* vertex and index data for drawing the ball. */
    std::vector<Vertex> ballVertices;
    std::vector<uint32_t> ballIndices;

    /* vertex and index data for drawing the hole. */
    std::vector<Vertex> quadVertices;
    std::vector<uint32_t> quadIndices;

    glm::mat4 modelMatrixHole;
    glm::mat4 modelMatrixBall;
    glm::mat4 modelMatrixStartVortex;
    std::vector<glm::mat4> modelMatrixVortexes;

    // the scale matrix for the ball.
    glm::mat4 scale;

    // data on where the ball is, how fast it is moving, etc.
    struct {
        glm::vec3 prevPosition;
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec3 acceleration;
        glm::quat totalRotated;
    } ball;

    bool ballProximity(glm::vec3 const &objPosition);
    void loadModels();
    void generate();
    void generateModelMatrices();
public:
    virtual glm::vec4 getBackgroundColor() { return glm::vec4(0.0, 0.0, 0.0, 1.0); }
    virtual void updateAcceleration(float x, float y, float z) { ball.acceleration = {-x, -y, 0.0f}; }
    virtual bool updateData();
    virtual bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures);
    virtual bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged);
    virtual void start() {
        prevTime = std::chrono::high_resolution_clock::now();
    }

    virtual void getLevelFinisherCenter(float &x, float &y) {
        x = holePosition.x;
        y = holePosition.y;
    }

    virtual void init() {
        loadModels();
        generate();
        generateModelMatrices();
    }
    void initSetHoleTexture(std::string const &texture) { holeTexture = texture; }
    void initSetVortexTexture(std::string const &texture) { vortexTexture = texture; }
    void initSetStartVortexTexture(std::string const &texture) { startVortexTexture = texture; }
    void initSetBallTexture(std::string const &texture) { ballTexture = texture; }

    AvoidVortexLevel(std::shared_ptr<GameRequester> inGameRequester, float width, float height, float maxZ)
            : Level(std::move(inGameRequester), width, height, maxZ),
              maxX(m_width/2),
              maxY(m_height/2),
              prevTime(std::chrono::high_resolution_clock::now()) { }
    virtual ~AvoidVortexLevel() {}
};
#endif