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
#ifndef AMAZING_LABYRINTH_MAZE_HPP
#define AMAZING_LABYRINTH_MAZE_HPP
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <deque>
#include <string>
#include <array>

#include "../../graphics.hpp"
#include "../levelFinish.hpp"
#include "../level.hpp"
#include "../generatedMazeAlgorithms.hpp"

struct MazeSaveData : public LevelSaveData {
    static int constexpr mazeVersion = 1;
    uint32_t nbrRows;
    uint32_t ballRow;
    uint32_t ballCol;
    Point<float> ballPos;
    uint32_t rowEnd;
    uint32_t colEnd;
    std::vector<uint32_t> wallTextures;
    std::vector<uint8_t> mazeWallsVector;
    MazeSaveData(MazeSaveData &&other) noexcept
            : LevelSaveData{mazeVersion},
              nbrRows{other.nbrRows},
              ballRow{other.ballRow},
              ballCol{other.ballCol},
              ballPos{other.ballPos},
              rowEnd{other.rowEnd},
              colEnd{other.colEnd},
              wallTextures{other.wallTextures},
              mazeWallsVector{std::move(other.mazeWallsVector)} {
    }

    MazeSaveData()
            : LevelSaveData{mazeVersion},
              nbrRows{0},
              ballRow{0},
              ballCol{0},
              ballPos{0.0f, 0.0f},
              rowEnd{0},
              colEnd{0},
              mazeWallsVector{}
    {
    }

    MazeSaveData(
            uint32_t nbrRows_,
            uint32_t ballRow_,
            uint32_t ballCol_,
            Point<float> &&ballPos_,
            uint32_t rowEnd_,
            uint32_t colEnd_,
            std::vector<uint32_t> &&wallTextures_,
            std::vector<uint8_t> &&mazeWallsVector_)
            : LevelSaveData{mazeVersion},
              nbrRows{nbrRows_},
              ballRow{ballRow_},
              ballCol{ballCol_},
              ballPos{ballPos_},
              rowEnd{rowEnd_},
              colEnd{colEnd_},
              wallTextures{std::move(wallTextures_)},
              mazeWallsVector{std::move(mazeWallsVector_)}
    {
    }
};

class Maze;

class Maze : public Level {
protected:
    using MazeWallModelMatrixGeneratorFcn = std::function<std::vector<glm::mat4>(std::vector<bool> const &,
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
    std::string ballTexture;
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

public:

    struct CreateParameters {
        unsigned int numberRows;
        GeneratedMazeBoard::Mode mode;
    };

private:
    static Maze::MazeWallModelMatrixGeneratorFcn getMazeWallModelMatricesGenerator();
    void initializeCell(uint8_t nibble, size_t row, size_t col);

public:
    Maze(std::shared_ptr<GameRequester> inGameRequester,
         std::shared_ptr<MazeSaveData> sd,
         float width,
         float height,
         float floorZ,
         MazeWallModelMatrixGeneratorFcn wallModelMatrixGeneratorFcn = getMazeWallModelMatricesGenerator())
            :Level(std::move(inGameRequester), width, height, floorZ, true,
                   height/((sd ?sd->nbrRows:1.0f) *numberBlocksPerCell +1)/glm::length(glm::vec2{width, height})/m_originalBallDiameter,
                   false),
             drawHole{true},
             m_mazeBoard{sd->nbrRows,
                         static_cast<uint32_t>(std::floor((sd->nbrRows)*m_width/m_height)),
                         GeneratedMazeBoard::Mode::none}
    {
        preGenerate();

        loadModels();

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
        modelMatrixBall = glm::translate(glm::mat4(1.0f), m_ball.position)*glm::mat4_cast(m_ball.totalRotated)*scaleBall;
    }

    Maze(std::shared_ptr<GameRequester> inGameRequester,
        CreateParameters const &createParameters,
        float width,
        float height,
        float maxZ,
        MazeWallModelMatrixGeneratorFcn wallModelMatrixGeneratorFcn = getMazeWallModelMatricesGenerator())
            :Level(std::move(inGameRequester), width, height, maxZ, true,
                    height/(createParameters.numberRows *numberBlocksPerCell +1)/glm::length(glm::vec2{width, height})/m_originalBallDiameter,
                    false),
             drawHole{true},
             m_mazeBoard{createParameters.numberRows,
                         static_cast<uint32_t>(std::floor((createParameters.numberRows)*m_width/m_height)),
                         createParameters.mode}
    {
        preGenerate();

        loadModels();
        generateModelMatrices(wallModelMatrixGeneratorFcn);
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
    bool updateData() override ;
    bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) override;
    bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures,
            bool &texturesChanged) override;
    void start() override {
        prevTime = std::chrono::high_resolution_clock::now();
    }

    void initAddWallTexture(std::string const &texturePath) { wallTextures.push_back(texturePath); }
    void doneAddingWallTextures();
    void initSetFloorTexture(std::string const &texturePath) { floorTexture = texturePath; }
    void initSetHoleTexture(std::string const &texturePath) { holeTexture = texturePath; }
    void initSetBallTexture(std::string const &texturePath) { ballTexture = texturePath; }
    SaveLevelDataFcn getSaveLevelDataFcn() override;

    ~Maze() override = default;
};
#endif // AMAZING_LABYRINTH_MAZE_HPP