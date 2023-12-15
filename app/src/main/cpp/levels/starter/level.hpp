/**
 * Copyright 2023 Cerulean Quasar. All Rights Reserved.
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
#ifndef AMAZING_LABYRINTH_STARTER_LEVEL_HPP
#define AMAZING_LABYRINTH_STARTER_LEVEL_HPP

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../common.hpp"
#include <boost/optional.hpp>
#include "level.hpp"
#include "../basic/level.hpp"
#include "loadData.hpp"
#include "../../levelDrawer/levelDrawer.hpp"

namespace starter {
    class Level : public basic::Level {
    private:
        static float constexpr m_wallThickness = m_modelSize / 10.0f;

        float const maxPosX;
        float const maxPosY;
        float const errVal;

        std::chrono::high_resolution_clock::time_point prevTime;

        // level starter text and directions.  One string per page.
        std::vector<std::string> text;
        uint32_t textIndex;
        bool transitionText;

        glm::vec3 textScale;

        levelDrawer::DrawObjReference m_objRefBall;
        levelDrawer::DrawObjDataReference m_objDataRefBall;

        levelDrawer::DrawObjReference m_objRefTextBox;
        levelDrawer::DrawObjDataReference m_objDataRefTextBox;
    protected:
        static char constexpr const *ModelNameCorridor = "Corridor";
        static char constexpr const *ModelNameMazeBegin = "Begin";
        static char constexpr const *ModelNameMazeEnd = "End";
        static char constexpr const *ModelNameCorridorCorner = "CorridorCorner";
        static char constexpr const *ModelNameTextBox = "TextBox";

    public:
        static char constexpr const *m_name = "starter";

        Level(levelDrawer::Adaptor inLevelDrawer,
                     std::shared_ptr<LevelConfigData> const &lcd,
                     std::shared_ptr<LevelSaveData> const & /*sd*/,
                     float maxZ)
                : basic::Level(std::move(inLevelDrawer), lcd, maxZ, true,
                               objectNoShadowsRenderDetailsName, levelDrawer::DefaultConfig::getDefaultParameters()),
                  maxPosX(m_width / 2 - ballRadius() -
                          m_wallThickness * ballDiameter() / m_modelSize),
                  maxPosY(m_height / 2 - ballRadius() -
                          m_wallThickness * ballDiameter() / m_modelSize),
                  errVal(ballDiameter() / 5.0f),
                  text{lcd->startupMessages}
        {
            prevTime = std::chrono::high_resolution_clock::now();

            textIndex = 0;
            transitionText = false;

            textScale = {m_width / 2 - m_scaleBall, m_height / 2 - 2 * m_scaleBall, 1.0f};

            m_ball.prevPosition = {10.0f, 0.0f, 0.0f};
            m_ball.position = {-maxPosX, -maxPosY, m_mazeFloorZ - ballRadius()};
            m_ball.velocity = {0.0f, 0.0f, 0.0f};
            m_ball.acceleration = {0.0f, 0.0f, 0.0f};
            m_ball.totalRotated = glm::quat();

            float xpos = maxPosX;
            float ypos = maxPosY;
            float size = ballDiameter() * (1.0f + m_wallThickness) / m_modelSize;

            glm::vec3 cornerScale{size, size, size};
            glm::vec3 corridorHScale = glm::vec3{2 * (xpos - ballRadius()) / m_modelSize, size, size};
            glm::vec3 corridorVScale = glm::vec3{size, 2 * (ypos - ballRadius()) / m_modelSize, size};
            glm::vec3 zaxis = glm::vec3{0.0f, 0.0f, 1.0f};
            glm::vec3 mazeColor = glm::vec3{0.8f, 0.8f, 0.8f};

            // the start of maze
            auto const &modelStartMazeData = findModelsAndTextures(ModelNameMazeBegin);
            auto objIndex = m_levelDrawer.addObject(modelStartMazeData.models[0], getFirstTexture(modelStartMazeData));
            m_levelDrawer.addModelMatrixForObject(
                    objIndex,
                    glm::translate(glm::mat4(1.0f),
                                   glm::vec3(-xpos, -ypos, m_mazeFloorZ - ballRadius())) *
                    glm::scale(glm::mat4(1.0f), cornerScale));

            // the end of maze
            auto const &modelEndMazeData = findModelsAndTextures(ModelNameMazeEnd);
            objIndex = m_levelDrawer.addObject(modelEndMazeData.models[0], getFirstTexture(modelEndMazeData));
            m_levelDrawer.addModelMatrixForObject(
                    objIndex,
                    glm::translate(glm::mat4(1.0f),
                                   glm::vec3(-xpos, ypos, m_mazeFloorZ - ballRadius())) *
                    glm::scale(glm::mat4(1.0f), cornerScale));

            // the corners of the maze
            auto const &modelCornerData = findModelsAndTextures(ModelNameCorridorCorner);
            objIndex = m_levelDrawer.addObject(modelCornerData.models[0], getFirstTexture(modelCornerData));

            // bottom corner
            m_levelDrawer.addModelMatrixForObject(
                    objIndex,
                    glm::translate(glm::mat4(1.0f),
                                   glm::vec3(xpos, -ypos, m_mazeFloorZ - ballRadius())) *
                    glm::scale(glm::mat4(1.0f), cornerScale) *
                    glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), zaxis));

            // top corner
            m_levelDrawer.addModelMatrixForObject(
                    objIndex,
                    glm::translate(glm::mat4(1.0f),
                                   glm::vec3(xpos, ypos, m_mazeFloorZ - ballRadius())) *
                    glm::scale(glm::mat4(1.0f), cornerScale));

            // the corridors of the maze
            auto const &modelCorridorData = findModelsAndTextures(ModelNameCorridor);
            objIndex = m_levelDrawer.addObject(modelCorridorData.models[0], getFirstTexture(modelCorridorData));

            // bottom
            m_levelDrawer.addModelMatrixForObject(
                    objIndex,
                    glm::translate(glm::mat4(1.0f),
                                   glm::vec3(0.0f, -ypos, m_mazeFloorZ - ballRadius())) *
                    glm::scale(glm::mat4(1.0f), corridorHScale) *
                    glm::rotate(glm::mat4(1.0), glm::radians(90.0f), zaxis));

            // side
            m_levelDrawer.addModelMatrixForObject(
                    objIndex,
                    glm::translate(glm::mat4(1.0f),
                                   glm::vec3(xpos, 0.0f, m_mazeFloorZ - ballRadius())) *
                    glm::scale(glm::mat4(1.0f), corridorVScale));

            // top
            m_levelDrawer.addModelMatrixForObject(
                    objIndex,
                    glm::translate(glm::mat4(1.0f),
                                   glm::vec3(0.0f, ypos, m_mazeFloorZ - ballRadius())) *
                    glm::scale(glm::mat4(1.0f), corridorHScale) *
                    glm::rotate(glm::mat4(1.0), glm::radians(90.0f), zaxis));

            // ball
            auto const &modelBallData = findModelsAndTextures(ModelNameBall);
            m_objRefBall = m_levelDrawer.addObject(modelBallData.models[0], getFirstTexture(modelBallData));

            glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), m_ball.position) *
                                   glm::mat4_cast(m_ball.totalRotated) *
                                   ballScaleMatrix();
            m_objDataRefBall = m_levelDrawer.addModelMatrixForObject(
                    m_objRefBall, modelMatrix);

            // the text box at the center of the maze
            auto const &modelTextBoxData = findModelsAndTextures(ModelNameTextBox);
            m_objRefTextBox = m_levelDrawer.addObject(modelTextBoxData.models[0],
                    std::make_shared<levelDrawer::TextureDescriptionText>(text[textIndex]));

            m_objDataRefTextBox = m_levelDrawer.addModelMatrixForObject(
                    m_objRefTextBox,
                    glm::translate(glm::mat4(1.0f), glm::vec3(-ballRadius(), 0.0f, m_mazeFloorZ)) *
                    glm::scale(glm::mat4(1.0f), textScale));
        }

        void clearText();

        bool isInBottomCorridor();

        bool isInSideCorridor();

        bool updateData() override;

        bool updateDrawObjects() override;

        char const *name() override { return m_name; }

        std::vector<uint8_t> saveData(levelTracker::GameSaveData const &,
                                      char const *) override
        { return std::vector<uint8_t>{}; }

        void getLevelFinisherCenter(float &x, float &y, float &z) override {
            x = 0.0f;
            y = 0.0f;
            z = m_mazeFloorZ;
        };

        void start() override {
            prevTime = std::chrono::high_resolution_clock::now();
        }
    };
} // namespace starter

#endif // AMAZING_LABYRINTH_STARTER_LEVEL_HPP
