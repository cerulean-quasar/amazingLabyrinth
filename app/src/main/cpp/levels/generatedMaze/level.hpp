/**
 * Copyright 2022 Cerulean Quasar. All Rights Reserved.
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

#ifndef AMAZING_LABYRINTH_GENERATED_MAZE_LEVEL_HPP
#define AMAZING_LABYRINTH_GENERATED_MAZE_LEVEL_HPP
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>
#include <array>

#include "../../levelDrawer/levelDrawer.hpp"
#include "../basic/level.hpp"
#include "../generatedMazeAlgorithms.hpp"
#include "loadData.hpp"

namespace generatedMaze {
    class Level : public basic::Level {
    protected:
        using MazeWallModelMatrixGeneratorFcn = std::function<std::vector<glm::mat4>(
                std::vector<bool> const &,
                float width,
                float height,
                float maxZ,
                unsigned int nbrCols,
                unsigned int nbrRows,
                float scaleWallZ)>;
        static constexpr float m_originalWallHeight = 3.0f;
        static constexpr unsigned int numberBlocksPerCell = 2;
        Random random;
        std::vector<std::string> wallTextures;
        std::string floorTexture;
        std::string holeTexture;
        std::chrono::high_resolution_clock::time_point prevTime;
        bool drawHole;
        float m_scaleWallZ;

        // data on where the ball is, how fast it is moving, etc.
        struct {
            uint32_t row;
            uint32_t col;
        } m_ballCell;
        GeneratedMazeBoard m_mazeBoard;

        glm::mat4 scaleBall;
        std::vector<glm::mat4> modelMatricesMaze;
        glm::mat4 floorModelMatrix;
        glm::mat4 modelMatrixHole;
        glm::mat4 modelMatrixBall;

        // the indices stating how to identify the ball object
        levelDrawer::DrawObjReference m_objRefBall;
        levelDrawer::DrawObjDataReference m_objDataRefBall;

        std::vector<uint32_t> m_wallTextureIndices;

        levelDrawer::DrawObjReference m_objRefHole;
        levelDrawer::DrawObjDataReference m_objDataRefHole;
        levelDrawer::DrawObjReference m_objRefFloor;
        levelDrawer::DrawObjDataReference m_objDataRefFloor;
        std::vector<levelDrawer::DrawObjReference> m_objRefsWalls;
        std::vector<levelDrawer::DrawObjDataReference> m_objDataRefsWalls;

        float getRowCenterPosition(unsigned int row);

        float getColumnCenterPosition(unsigned int col);

        float getBallZPosition();

        glm::vec3 getCellCenterPosition(unsigned int row, unsigned int col);

        bool ballInProximity(float x, float y);

        void generateModelMatrices(MazeWallModelMatrixGeneratorFcn &wallModelMatrixGeneratorFcn);

        void generateMazeVector(std::vector<bool> &wallsExist);

        std::vector<uint8_t> getSerializedMazeWallVector();

        void generateCellsFromMazeVector(std::vector<uint8_t> const &mazeVector);

    private:
        static Level::MazeWallModelMatrixGeneratorFcn getMazeWallModelMatricesGenerator();
        static GeneratedMazeBoard::Mode getGeneratorType(
                std::shared_ptr<LevelConfigData> const &lcd,
                std::shared_ptr<LevelSaveData> const &sd)
        {
            if (sd) {
                return GeneratedMazeBoard::Mode::none;
            } else if (lcd->dfsSearch) {
                return GeneratedMazeBoard::Mode::DFS;
            } else {
                return GeneratedMazeBoard::Mode::BFS;
            }
        }

        void initializeCell(uint8_t nibble, size_t row, size_t col);

    public:
        static char constexpr const *m_name = "generatedMaze";

        // lcd must never be null
        // sd can be null if there is no save data.  If it is not null, the maze will be restored
        // from the save data, otherwise the maze will be generated.
        Level(levelDrawer::Adaptor inLevelDrawer,
              std::shared_ptr<LevelConfigData> const &lcd,
              std::shared_ptr<LevelSaveData> const &sd,
              float floorZ,
              std::string const &renderDetailsNameDefault = "",
              std::shared_ptr<renderDetails::Parameters> parametersDefault = nullptr,
              std::string const &renderDetailsNameBallOverride = "",
              std::shared_ptr<renderDetails::Parameters> parametersBallOverride = nullptr,
              std::string const &renderDetailsNameHoleOverride = "",
              std::shared_ptr<renderDetails::Parameters> parametersHoleOverride = nullptr,
              std::string const &renderDetailsNameFloorOverride = "",
              std::shared_ptr<renderDetails::Parameters> parametersFloorOverride = nullptr,
             MazeWallModelMatrixGeneratorFcn wallModelMatrixGeneratorFcn = getMazeWallModelMatricesGenerator())
                : basic::Level(std::move(inLevelDrawer), lcd, floorZ, true, renderDetailsNameDefault, parametersDefault),
                  wallTextures{lcd->wallTextureNames},
                  floorTexture{lcd->mazeFloorTexture},
                  holeTexture{lcd->holeTexture},
                  drawHole{true},
                  m_mazeBoard{lcd->numberRows,
                              static_cast<uint32_t>(std::floor((lcd->numberRows) * m_width / m_height)),
                              getGeneratorType(lcd, sd)}
        {
            m_levelDrawer.setClearColor(glm::vec4{0.0f, 0.0f, 0.0f, 1.0f});

            m_scaleBall = m_height / (lcd->numberRows * numberBlocksPerCell + 1) / m_originalBallDiameter;
            preGenerate();

            if (wallTextures.empty()) {
                throw std::runtime_error("Maze wall textures not initialized.");
            }

            if (sd) {
                m_mazeBoard.setEnd(sd->rowEnd, sd->colEnd);

                // Set up the "start" to be where the ball was last since we don't know what the start
                // was anymore, but we do need a "valid" start for generateModelMatrices.
                m_mazeBoard.setStart(sd->ballRow, sd->ballCol);

                m_wallTextureIndices = sd->wallTextures;
                generateCellsFromMazeVector(sd->mazeWallsVector);
                generateModelMatrices(wallModelMatrixGeneratorFcn);

                m_ballCell.row = sd->ballRow;
                m_ballCell.col = sd->ballCol;

                // these are set incorrectly by generateModelMatrices.... set them up again.
                m_ball.position = glm::vec3{sd->ballPos.x, sd->ballPos.y, getBallZPosition()};
                modelMatrixBall = glm::translate(glm::mat4(1.0f), m_ball.position) *
                                  glm::mat4_cast(m_ball.totalRotated) * scaleBall;
            } else {
                generateModelMatrices(wallModelMatrixGeneratorFcn);
                if (m_wallTextureIndices.empty()) {
                    m_wallTextureIndices.reserve(modelMatricesMaze.size());
                    for (size_t i = 0; i < modelMatricesMaze.size(); i++) {
                        m_wallTextureIndices.push_back(random.getUInt(0, wallTextures.size() - 1));
                    }
                }
            }

            // the floor
            if (parametersFloorOverride == nullptr || renderDetailsNameFloorOverride.length() == 0) {
                m_objRefFloor = m_levelDrawer.addObject(
                        std::make_shared<levelDrawer::ModelDescriptionQuad>(),
                        std::make_shared<levelDrawer::TextureDescriptionPath>(floorTexture));
            } else {
                m_objRefFloor = m_levelDrawer.addObject(
                        std::make_shared<levelDrawer::ModelDescriptionQuad>(),
                        std::make_shared<levelDrawer::TextureDescriptionPath>(floorTexture),
                        renderDetailsNameFloorOverride, parametersFloorOverride);
            }

            m_objDataRefFloor = m_levelDrawer.addModelMatrixForObject(m_objRefFloor, floorModelMatrix);

            // the hole
            if (drawHole) {
                if (parametersHoleOverride == nullptr || renderDetailsNameHoleOverride.length() == 0) {
                    m_objRefHole = m_levelDrawer.addObject(
                            std::make_shared<levelDrawer::ModelDescriptionQuad>(),
                            std::make_shared<levelDrawer::TextureDescriptionPath>(holeTexture));
                } else {
                    m_objRefHole = m_levelDrawer.addObject(
                            std::make_shared<levelDrawer::ModelDescriptionQuad>(),
                            std::make_shared<levelDrawer::TextureDescriptionPath>(holeTexture),
                            renderDetailsNameHoleOverride, parametersHoleOverride);
                }
            }

            m_objDataRefHole = m_levelDrawer.addModelMatrixForObject(m_objRefHole, modelMatrixHole);

            // the walls
            // always use the default renderDetails.
            m_objRefsWalls.reserve(wallTextures.size());
            for (auto wallTexture : wallTextures) {
                auto objIndex = m_levelDrawer.addObject(
                        std::make_shared<levelDrawer::ModelDescriptionCube>(),
                        std::make_shared<levelDrawer::TextureDescriptionPath>(wallTexture));
                m_objRefsWalls.push_back(objIndex);
            }

            m_objDataRefsWalls.reserve(m_wallTextureIndices.size());
            for (size_t i = 0; i < m_wallTextureIndices.size(); i++) {
                auto objIndex = m_levelDrawer.addModelMatrixForObject(
                        m_objRefsWalls[m_wallTextureIndices[i]],
                        modelMatricesMaze[i]);
                m_objDataRefsWalls.push_back(objIndex);
            }

            // the ball
            if (parametersBallOverride == nullptr || renderDetailsNameBallOverride.length() == 0) {
                m_objRefBall = m_levelDrawer.addObject(
                        std::make_shared<levelDrawer::ModelDescriptionPath>(m_ballModel),
                        std::make_shared<levelDrawer::TextureDescriptionPath>(m_ballTexture));
            } else {
                m_objRefBall = m_levelDrawer.addObject(
                        std::make_shared<levelDrawer::ModelDescriptionPath>(m_ballModel),
                        std::make_shared<levelDrawer::TextureDescriptionPath>(m_ballTexture),
                        renderDetailsNameBallOverride, parametersBallOverride);
            }

            m_objDataRefBall = m_levelDrawer.addModelMatrixForObject(m_objRefBall, modelMatrixBall);
        }

        void preGenerate() {
            m_scaleWallZ = ballDiameter();

            prevTime = std::chrono::high_resolution_clock::now();
            glm::vec3 xaxis{1.0f, 0.0f, 0.0f};
            m_ball.totalRotated = glm::angleAxis(glm::radians(270.0f), xaxis);
            m_ball.acceleration = {0.0f, 0.0f, 0.0f};
            m_ball.velocity = {0.0f, 0.0f, 0.0f};
            scaleBall = ballScaleMatrix();
        }

        bool updateData() override;

        bool updateDrawObjects() override;

        void getLevelFinisherCenter(float &x, float &y, float &z) override {
            auto center = getCellCenterPosition(m_mazeBoard.rowEnd(), m_mazeBoard.colEnd());
            x = center.x;
            y = center.y;
            z = m_mazeFloorZ;
        }

        void start() override {
            prevTime = std::chrono::high_resolution_clock::now();
        }

        char const *name() override { return m_name; }

        std::vector<uint8_t> saveData(levelTracker::GameSaveData const &gsd,
                                      char const *saveLevelDataKey) override;

        ~Level() override = default;
    };
}

#endif // AMAZING_LABYRINTH_GENERATED_MAZE_LEVEL_HPP
