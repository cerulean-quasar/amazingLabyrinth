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
#ifndef AMAZING_LABYRINTH_LEVEL_STARTER_HPP
#define AMAZING_LABYRINTH_LEVEL_STARTER_HPP

#include <string>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "graphics.hpp"
#include "level.hpp"

class LevelStarter : public Level {
private:
    std::string const ballImage = "textures/ballLevelStarter.png";
    std::string const holeImage = "textures/holeLevelStarter.png";
    std::string const corridorImageH1 = "textures/corridorH1.png";
    std::string const corridorImageV = "textures/corridorV.png";
    std::string const corridorImageH2 = "textures/corridorH2.png";
    float const scale = 10.0f;
    float const maxPosX = 0.7f;
    float const maxPosY = 1.1f;
    float const maxPosYNoCorridors = maxPosY - 2*maxPosX/scale;
    float const errVal = 0.05f;

    std::chrono::high_resolution_clock::time_point prevTime;

    // level starter text and directions.  One string per page.
    std::vector<std::string> text;
    uint32_t textIndex;
    bool transitionText;

    // quadVertices are the vertices for the text quad, the hole quad and the corridor quad.
    std::vector<Vertex> quadVertices;
    std::vector<uint32_t> quadIndices;

    glm::vec3 textScale;
    glm::vec3 holeScale;
    glm::vec3 corridorVScale;
    glm::vec3 corridorHScale;

    std::vector<Vertex> ballVertices;
    std::vector<uint32_t> ballIndices;
    glm::vec3 ballScale;

    // data on where the ball is, how fast it is moving, etc.
    struct {
        glm::vec3 prevPosition;
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec3 acceleration;
        glm::quat totalRotated;
    } ball;

public:
    LevelStarter() {
        prevTime = std::chrono::high_resolution_clock::now();
        getQuad(quadVertices, quadIndices);
        loadModel(MODEL_BALL, ballVertices, ballIndices);

        textIndex = 0;
        transitionText = false;

        textScale = {maxPosX+2*maxPosX/scale, maxPosYNoCorridors, 1.0f};
        holeScale = {2*maxPosX/scale, 2*maxPosX/scale, 1.0f};
        corridorHScale = {1.0f, 2*maxPosX/scale, 1.0f};
        corridorVScale = {2*maxPosX/scale, 1.0f, 1.0f};
        ballScale = {2*maxPosX/scale, 2*maxPosX/scale, 2*maxPosX/scale};

        ball.prevPosition = { 10.0f, 0.0f, 0.0f};
        ball.position = {-maxPosX, -maxPosY, 0.0f};
        ball.velocity = {0.0f, 0.0f, 0.0f};
        ball.acceleration = {0.0f, 0.0f, 0.0f};
        ball.totalRotated = glm::quat();
    }

    void clearText();
    void addTextString(std::string const inText);
    bool isInBottomCorridor();
    bool isInSideCorridor();
    void confineBall();

    virtual void loadModels() { }
    virtual void generate() { }
    virtual glm::vec4 getBackgroundColor() { return glm::vec4(0.0f, 0.0f, 0.0f, 0.0f); }
    virtual void generateModelMatrices() { }

    virtual void updateAcceleration(float x, float y, float z) {
        ball.acceleration = {-x, -y, 0.0f};
    }
    virtual bool updateData();
    virtual bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures);
    virtual bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged);
    virtual void start() {
        prevTime = std::chrono::high_resolution_clock::now();
    }
};
#endif