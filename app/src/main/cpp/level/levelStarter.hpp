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

#include "../graphics.hpp"
#include "../common.hpp"
#include <boost/optional.hpp>
#include "level.hpp"

class LevelStarter : public Level {
private:
    std::string const ballImage = "textures/levelStarter/ballLevelStarter.png";
    std::string const holeImage = "textures/levelStarter/holeLevelStarter.png";
    std::string const corridorImageH1 = "textures/levelStarter/corridorH1.png";
    std::string const corridorImageV = "textures/levelStarter/corridorV.png";
    std::string const corridorImageH2 = "textures/levelStarter/corridorH2.png";

    float const scale;
    float const maxPosX;
    float const maxPosY;
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
    LevelStarter(std::shared_ptr<GameRequester> inGameRequester,
            boost::optional<GameBundle> const &inGameSaveData, float width, float height, float maxZ)
            : Level(std::move(inGameRequester), width, height, maxZ),
              scale(10.0f),
              maxPosX(m_width/2-m_width/2/scale),
              maxPosY(m_height/2-m_width/2/scale)
    {
        prevTime = std::chrono::high_resolution_clock::now();
        getQuad(quadVertices, quadIndices);
        loadModel(m_gameRequester->getAssetStream(MODEL_BALL), ballVertices, ballIndices);

        textIndex = 0;
        transitionText = false;

        textScale = {m_width/2, m_height/2, 1.0f};
        holeScale = {m_width/2/scale, m_width/2/scale, 1.0f};
        corridorHScale = {m_width/2, m_width/2/scale, 1.0f};
        corridorVScale = {m_width/2/scale, m_height/2-m_width/scale, 1.0f};
        ballScale = {m_width/2/scale, m_width/2/scale, m_width/2/scale};

        ball.prevPosition = { 10.0f, 0.0f, 0.0f};
        ball.position = {-maxPosX, -maxPosY, m_maxZ - ballScale.z*m_originalBallDiameter/2.0f};
        ball.velocity = {0.0f, 0.0f, 0.0f};
        ball.acceleration = {0.0f, 0.0f, 0.0f};
        ball.totalRotated = glm::quat();

        if (inGameSaveData) {
            auto levelStateIt = inGameSaveData->find(KeyLevelIsAtStart);
            if (levelStateIt != inGameSaveData->end()) {
                m_finished = ! boost::get<bool>(levelStateIt->second);
            }
        }

    }

    void clearText();
    void addTextString(std::string const &inText);
    bool isInBottomCorridor();
    bool isInSideCorridor();
    void confineBall();

    void saveLevelData(GameBundle &saveData) override {
        saveData.insert(std::make_pair(KeyLevelIsAtStart, GameBundleValue(true)));
    }

    void init() override {}

    glm::vec4 getBackgroundColor() override { return glm::vec4(0.0f, 0.0f, 0.0f, 0.0f); }

    void updateAcceleration(float x, float y, float z) override {
        ball.acceleration = {-x, -y, 0.0f};
    }
    bool updateData() override;
    bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) override;
    bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged) override;
    void start() override {
        prevTime = std::chrono::high_resolution_clock::now();
    }
};
#endif