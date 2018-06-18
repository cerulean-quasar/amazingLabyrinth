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

class Random {
private:
    int fd;
public:
    Random() : fd(-1) { }
    unsigned int getUInt(unsigned int lowerBound, unsigned int upperBound);
    ~Random();
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
    glm::vec3 normal;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions();
    bool operator==(const Vertex& other) const;
};

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
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
    ~Cell() {}

    bool visited() const { return mVisited; }
    bool isStart() const { return mIsStart; }
    bool isEnd() const { return mIsEnd; }
    bool topWallExists() const { return mTopWallExists; }
    bool bottomWallExists() const { return mBottomWallExists; }
    bool leftWallExists() const { return mLeftWallExists; }
    bool rightWallExists() const { return mRightWallExists; }
};

class Maze {
private:
    unsigned int const numberBlocksPerCell = 2;
    glm::vec3 lightingSource;
    unsigned int numberRows;
    unsigned int numberColumns;
    std::chrono::high_resolution_clock::time_point prevTime;

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

    glm::mat4 view;
    glm::mat4 proj;
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
    void loadModel(std::string const &modelFile, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);
    void loadModelFloor();
    float getRowCenterPosition(unsigned int row);
    float getColumnCenterPosition(unsigned int col);
    glm::vec3 getCellCenterPosition(unsigned int row, unsigned int col);
public:
    Maze(unsigned int inNumberRows, unsigned int inNumberColumns)
        :numberRows(inNumberRows), numberColumns(inNumberColumns)
    {
        cells.resize(numberRows);
        for (unsigned int i = 0; i < numberRows; i++) {
            cells[i].resize(numberColumns);
        }

        ball.acceleration = {0.0f, 0.0f, 0.0f};
        ball.velocity = {0.0f, 0.0f, 0.0f};
        lightingSource = glm::vec3(0.0f, 0.0f, 1.28f/*0.01-1.28*/);
        unsigned int i = std::max(numberRows, numberColumns);
        scaleBall = glm::scale(glm::vec3(1.0f/(i*2.0f + 1),
                                          1.0f/(i*2.0f + 1),
                                          1.0f/(i*2.0f + 1)));
    }

    void loadModels();
    void setView();
    void updatePerspectiveMatrix(int surfaceWidth, int surfaceHeight);
    Cell const &getCell(unsigned int row, unsigned int column);
    void generate();
    void updateAcceleration(float x, float y, float z);
    bool updateData();
    void generateModelMatrices();

    std::vector<Vertex> const &getFloorVertices() { return floorVertices; }
    std::vector<uint32_t> const &getFloorIndices() { return floorIndices; }
    glm::mat4 const &getFloorModelMatrix() { return floorModelMatrix; }

    std::vector<Vertex> const &getBallVertices() { return ballVertices; }
    std::vector<uint32_t> const &getBallIndices() { return ballIndices; }
    glm::mat4 const &getBallModelMatrix() { return modelMatrixBall; }

    std::vector<Vertex> const &getHoleVertices() { return holeVertices; }
    std::vector<uint32_t> const &getHoleIndices() { return holeIndices; }
    glm::mat4 const &getHoleModelMatrix() { return modelMatrixHole; }

    std::vector<Vertex> const &getVertices() { return vertices; }
    std::vector<uint32_t> const &getIndices() { return indices; }
    std::vector<glm::mat4> const &getModelMatricesMaze() { return modelMatricesMaze; }
    uint32_t getNumberWalls() { return modelMatricesMaze.size(); }
    glm::mat4 getProjectionMatrix() { return proj; }
    glm::mat4 getViewMatrix() { return view; }
    glm::vec3 getLightingSource() { return lightingSource; }
    glm::mat4 getViewLightSource();

    ~Maze() {}
};
#endif