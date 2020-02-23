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
#ifndef AMAZING_LABYRINTH_MAZE_HPP
#define AMAZING_LABYRINTH_MAZE_HPP
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <vector>
#include <deque>
#include <string>
#include <array>

#include "../../vulkanWrapper.hpp"
#include "../../graphics.hpp"
#include "../levelFinish.hpp"
#include "../level.hpp"

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

class Cell {
    friend Maze;
private:
    bool mVisited;
    bool mIsStart;
    bool mIsEnd;
    bool mTopWallExists;
    bool mBottomWallExists;
    bool mLeftWallExists;
    bool mRightWallExists;
public:
    Cell() : mVisited(false), mIsStart(false), mIsEnd(false), mTopWallExists(true),
        mBottomWallExists(true), mLeftWallExists(true), mRightWallExists(true) {}
    ~Cell() = default;

    bool visited() const { return mVisited; }
    bool isStart() const { return mIsStart; }
    bool isEnd() const { return mIsEnd; }
    bool topWallExists() const { return mTopWallExists; }
    bool bottomWallExists() const { return mBottomWallExists; }
    bool leftWallExists() const { return mLeftWallExists; }
    bool rightWallExists() const { return mRightWallExists; }
};

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
    static constexpr float viscosity = 0.01f;
    static constexpr unsigned int numberBlocksPerCell = 2;
    Random random;
    std::vector<std::string> wallTextures;
    std::string ballTexture;
    std::string floorTexture;
    std::string holeTexture;
    unsigned int numberRows;
    unsigned int numberColumns;
    float scale;
    std::chrono::high_resolution_clock::time_point prevTime;
    bool drawHole;
    float m_scaleWallZ;

    // data on where the ball is, how fast it is moving, etc.
    struct {
        uint32_t row;
        uint32_t col;
        glm::vec3 prevPosition;
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec3 acceleration;
        glm::quat totalRotated;
    } ball;
    std::vector<std::vector<Cell>> cells;

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

    uint32_t m_rowEnd;
    uint32_t m_colEnd;

    std::vector<uint32_t> m_wallTextureIndices;

    void addCellOption(unsigned int r, unsigned int c, std::vector<std::pair<unsigned int, unsigned int> > &options);
    void loadModelFloor();
    float getRowCenterPosition(unsigned int row);
    float getColumnCenterPosition(unsigned int col);
    float getBallZPosition();
    glm::vec3 getCellCenterPosition(unsigned int row, unsigned int col);
    bool ballInProximity(float x, float y);
    Cell const &getCell(unsigned int row, unsigned int column);

    void loadModels();
    void generateBFS();
    void generateDFS();
    void generateModelMatrices(MazeWallModelMatrixGeneratorFcn &wallModelMatrixGeneratorFcn);
    void generateMazeVector(uint32_t &rowEnd, uint32_t &colEnd, std::vector<bool> &wallsExist);
    std::vector<uint8_t> getSerializedMazeWallVector();
    void generateCellsFromMazeVector(std::vector<uint8_t> const &mazeVector);

public:
    enum Mode {
        BFS = 0,
        DFS = 1
    };

    struct CreateParameters {
        unsigned int numberRows;
        Mode mode;
    };

private:
    static Maze::MazeWallModelMatrixGeneratorFcn getMazeWallModelMatricesGenerator();

public:
    Maze(std::shared_ptr<GameRequester> inGameRequester,
         std::shared_ptr<MazeSaveData> sd,
         float width,
         float height,
         float maxZ,
         MazeWallModelMatrixGeneratorFcn wallModelMatrixGeneratorFcn = getMazeWallModelMatricesGenerator())
            :Level(std::move(inGameRequester), width, height, maxZ),
             drawHole{true}
    {
        numberRows = sd->nbrRows;
        preGenerate();

        loadModels();

        m_wallTextureIndices = sd->wallTextures;
        generateCellsFromMazeVector(sd->mazeWallsVector);
        m_rowEnd = sd->rowEnd;
        m_colEnd = sd->colEnd;
        cells[m_rowEnd][m_colEnd].mIsEnd = true;

        // Set up the "start" to be where the ball was last since we don't know what the start
        // was anymore, but we do need a "valid" start for generateModelMatrices.
        cells[sd->ballRow][sd->ballCol].mIsStart = true;

        generateModelMatrices(wallModelMatrixGeneratorFcn);

        ball.row = sd->ballRow;
        ball.col = sd->ballCol;

        // these are set incorrectly by generateModelMatrices.... set them up again.
        ball.position = glm::vec3{sd->ballPos.x, sd->ballPos.y, getBallZPosition()};
        modelMatrixBall = glm::translate(ball.position)*glm::toMat4(ball.totalRotated)*scaleBall;
    }

    Maze(std::shared_ptr<GameRequester> inGameRequester,
        CreateParameters const &createParameters,
        float width,
        float height,
        float maxZ,
        MazeWallModelMatrixGeneratorFcn wallModelMatrixGeneratorFcn = getMazeWallModelMatricesGenerator())
            :Level(std::move(inGameRequester), width, height, maxZ),
             drawHole{true}
    {
        numberRows = createParameters.numberRows;
        preGenerate();

        loadModels();
        if (createParameters.mode == BFS) {
            generateBFS();
        } else {
            generateDFS();
        }
        generateModelMatrices(wallModelMatrixGeneratorFcn);
    }

    void preGenerate() {
        numberColumns = static_cast<uint32_t>(std::floor(numberRows*m_width/m_height));
        scale = m_width/(numberColumns*numberBlocksPerCell + 1)/2;
        m_scaleWallZ = scale*2;

        prevTime = std::chrono::high_resolution_clock::now();
        glm::vec3 xaxis{1.0f, 0.0f, 0.0f};
        ball.totalRotated = glm::angleAxis(glm::radians(270.0f), xaxis);
        ball.acceleration = {0.0f, 0.0f, 0.0f};
        ball.velocity = {0.0f, 0.0f, 0.0f};
        unsigned int i = std::max(numberRows, numberColumns);
        scaleBall = glm::scale(glm::vec3(scale, scale, scale));
        cells.resize(numberRows);

        for (unsigned int i = 0; i < numberRows; i++) {
            cells[i].resize(numberColumns);
        }
    }

    glm::vec4 getBackgroundColor() override { return {0.0f, 0.0f, 0.0f, 0.0f}; }
    void updateAcceleration (float x, float y, float z) override;
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
#endif