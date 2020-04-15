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
#ifndef AMAZING_LABYRINTH_LEVEL_STARTER_HPP
#define AMAZING_LABYRINTH_LEVEL_STARTER_HPP

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

    float const maxPosX;
    float const maxPosY;
    float const errVal;

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

public:
    LevelStarter(std::shared_ptr<GameRequester> inGameRequester,
            float width, float height, float maxZ)
            : Level(std::move(inGameRequester), width, height, maxZ, true, 1.0f/50.0f, false),
              maxPosX(m_width/2-ballRadius()),
              maxPosY(m_height/2-ballRadius()),
              errVal(ballDiameter()/5.0f)
    {
        prevTime = std::chrono::high_resolution_clock::now();
        getQuad(quadVertices, quadIndices);
        loadModel(m_gameRequester->getAssetStream(MODEL_BALL), ballVertices, ballIndices);

        textIndex = 0;
        transitionText = false;

        textScale = {m_width/2-m_scaleBall, m_height/2-2*m_scaleBall, 1.0f};
        holeScale = {m_scaleBall, m_scaleBall, 1.0f};
        corridorHScale = {m_width/2, m_scaleBall, 1.0f};
        corridorVScale = {m_scaleBall, m_height/2-2*m_scaleBall, 1.0f};

        m_ball.prevPosition = { 10.0f, 0.0f, 0.0f};
        m_ball.position = {-maxPosX, -maxPosY, m_mazeFloorZ + ballRadius()};
        m_ball.velocity = {0.0f, 0.0f, 0.0f};
        m_ball.acceleration = {0.0f, 0.0f, 0.0f};
        m_ball.totalRotated = glm::quat();
    }

    void clearText();
    void addTextString(std::string const &inText);
    bool isInBottomCorridor();
    bool isInSideCorridor();

    glm::vec4 getBackgroundColor() override { return glm::vec4(0.0f, 0.0f, 0.0f, 0.0f); }

    bool updateData() override;
    bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) override;
    bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged) override;
    void start() override {
        prevTime = std::chrono::high_resolution_clock::now();
    }
};
#endif