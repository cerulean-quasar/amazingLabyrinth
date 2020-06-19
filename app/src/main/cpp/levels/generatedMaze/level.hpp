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

#ifndef AMAZING_LABYRINTH_GENERATED_MAZE_LEVEL_HPP
#define AMAZING_LABYRINTH_GENERATED_MAZE_LEVEL_HPP
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>
#include <array>

#include "../../graphics.hpp"
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

        /* vertex and index data for drawing the floor. */
        std::vector<Vertex> floorVertices;
        std::vector<uint32_t> floorIndices;

        /* vertex and index data for drawing the walls. */
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        /* vertex and index data for drawing the hole. */
        std::vector<Vertex> holeVertices;
        std::vector<uint32_t> holeIndices;

        /* vertex and index data for drawing the ball. */
        std::vector<Vertex> ballVertices;
        std::vector<uint32_t> ballIndices;

        std::vector<uint32_t> m_wallTextureIndices;

        void loadModelFloor();

        float getRowCenterPosition(unsigned int row);

        float getColumnCenterPosition(unsigned int col);

        float getBallZPosition();

        glm::vec3 getCellCenterPosition(unsigned int row, unsigned int col);

        bool ballInProximity(float x, float y);

        void loadModels();

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
        Level(std::shared_ptr<GameRequester> inGameRequester,
             std::shared_ptr<LevelConfigData> const &lcd,
             std::shared_ptr<LevelSaveData> const &sd,
             glm::mat4 const &proj,
             glm::mat4 const &view,
             float floorZ,
             MazeWallModelMatrixGeneratorFcn wallModelMatrixGeneratorFcn = getMazeWallModelMatricesGenerator())
                : basic::Level(std::move(inGameRequester), lcd, proj, view, floorZ, true),
                  wallTextures{lcd->wallTextureNames},
                  floorTexture{lcd->mazeFloorTexture},
                  holeTexture{lcd->holeTexture},
                  drawHole{true},
                  m_mazeBoard{lcd->numberRows,
                              static_cast<uint32_t>(std::floor((lcd->numberRows) * m_width / m_height)),
                              getGeneratorType(lcd, sd)}
        {
            m_scaleBall = m_height / (lcd->numberRows * numberBlocksPerCell + 1) / m_originalBallDiameter;
            preGenerate();

            loadModels();

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

        glm::vec4 getBackgroundColor() override { return {0.0f, 0.0f, 0.0f, 0.0f}; }

        bool updateData() override;

        bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) override;

        bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures,
                                      bool &texturesChanged) override;

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
