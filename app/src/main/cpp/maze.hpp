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
#ifndef AMAZING_LABYRINTH_MAZE_HPP
#define AMAZING_LABYRINTH_MAZE_HPP
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <vector>
#include <deque>
#include <string>
#include <array>

#include "vulkanWrapper.hpp"
#include "graphics.hpp"
#include "levelFinish.hpp"
#include "level.hpp"


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
    ~Cell() {}

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
    static constexpr float viscosity = 0.01f;
    static constexpr unsigned int numberBlocksPerCell = 2;
    Random random;
    std::vector<std::string> wallTextures;
    std::string ballTexture;
    std::string floorTexture;
    std::string holeTexture;
    unsigned int const numberRows;
    unsigned int const numberColumns;
    float const scale;
    std::chrono::high_resolution_clock::time_point prevTime;
    bool drawHole;

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
    std::vector<std::vector<Cell> > cells;

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

    void addCellOption(unsigned int r, unsigned int c, std::vector<std::pair<unsigned int, unsigned int> > &options);
    void loadModelFloor();
    float getRowCenterPosition(unsigned int row);
    float getColumnCenterPosition(unsigned int col);
    glm::vec3 getCellCenterPosition(unsigned int row, unsigned int col);
    bool ballInProximity(float x, float y);
    Cell const &getCell(unsigned int row, unsigned int column);

    void loadModels();
    void generateBFS();
    void generateDFS();
    virtual void generateModelMatrices();
    void generateMazeVector(uint32_t &rowEnd, uint32_t &colEnd, std::vector<bool> &wallsExist);

public:
    enum Mode {
        BFS = 0,
        DFS = 1
    };

private:
    Mode m_mode;

public:
    Maze(unsigned int inNumberRows, Mode inMode, uint32_t width, uint32_t height)
        :Level(width, height),
         numberRows(inNumberRows),
         numberColumns(inNumberRows*width/height),
         scale(1.0f/(std::max(numberRows, numberColumns)*numberBlocksPerCell + 1)),
         m_mode(inMode),
         drawHole{true}
    {
        cells.resize(numberRows);
        for (unsigned int i = 0; i < numberRows; i++) {
            cells[i].resize(numberColumns);
        }

        prevTime = std::chrono::high_resolution_clock::now();
        ball.totalRotated = glm::quat();
        ball.acceleration = {0.0f, 0.0f, 0.0f};
        ball.velocity = {0.0f, 0.0f, 0.0f};
        unsigned int i = std::max(numberRows, numberColumns);
        scaleBall = glm::scale(glm::vec3(scale, scale, scale));
    }

    virtual void init() {
        loadModels();
        if (m_mode == BFS) {
            generateBFS();
        } else {
            generateDFS();
        }
        generateModelMatrices();
    }

    virtual glm::vec4 getBackgroundColor() { return {0.0f, 0.0f, 0.0f, 0.0f}; }
    virtual void updateAcceleration(float x, float y, float z);
    virtual bool updateData();
    virtual bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures);
    virtual bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged);
    virtual void start() {
        prevTime = std::chrono::high_resolution_clock::now();
    }

    void initAddWallTexture(std::string const &texturePath) { wallTextures.push_back(texturePath); }
    void initSetFloorTexture(std::string const &texturePath) { floorTexture = texturePath; }
    void initSetHoleTexture(std::string const &texturePath) { holeTexture = texturePath; }
    void initSetBallTexture(std::string const &texturePath) { ballTexture = texturePath; }

    virtual ~Maze() {}
};
#endif