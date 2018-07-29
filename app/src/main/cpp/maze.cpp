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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <istream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <cstring>

#include "maze.hpp"
#include "android.hpp"
#include "random.hpp"
#include "graphics.hpp"

static std::string const MODEL_WALL("models/wall.obj");
static std::string const MODEL_BALL("models/ball.obj");
static std::string const MODEL_HOLE("models/hole.obj");

void Maze::updateAcceleration(float x, float y, float z) {
    ball.acceleration = glm::vec3(-x,-y,0.0f);
}

bool Maze::updateData() {
    if (finished) {
        // the maze is finished, do nothing and return false (drawing is not necessary).
        return false;
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count();
    prevTime = currentTime;

    ball.velocity += ball.acceleration * time;
    ball.position += ball.velocity * time;

    auto cell = getCell(ball.row, ball.col);
    float cellCenterX = getColumnCenterPosition(ball.col);
    float cellCenterY = getRowCenterPosition(ball.row);
    float errDistance = 0.01f;
    if (cell.isEnd() &&
            ball.position.x < cellCenterX + errDistance && ball.position.x > cellCenterX - errDistance &&
            ball.position.y < cellCenterY + errDistance && ball.position.y > cellCenterY - errDistance) {
        finished = true;
        ball.position.x = cellCenterX;
        ball.position.y = cellCenterY;
        ball.velocity = {0.0f, 0.0f, 0.0f};
        return true;
    }
    if (cell.leftWallExists() && ball.position.x < cellCenterX) {
        if (ball.velocity.x < 0.0f) {
            ball.velocity.x = 0.0f;
        }
        ball.position.x = cellCenterX;
    }
    if (cell.rightWallExists() && cellCenterX < ball.position.x) {
        if (ball.velocity.x > 0.0f) {
            ball.velocity.x = 0.0f;
        }
        ball.position.x = cellCenterX;
    }
    if (cell.bottomWallExists() && cellCenterY < ball.position.y) {
        if (ball.velocity.y > 0.0f) {
            ball.velocity.y = 0.0f;
        }
        ball.position.y = cellCenterY;
    }
    if (cell.topWallExists() && ball.position.y < cellCenterY) {
        if (ball.velocity.y < 0.0f) {
            ball.velocity.y = 0.0f;
        }
        ball.position.y = cellCenterY;
    }

    float cellHeight = 2.0f / numberRows;
    float cellWidth = 2.0f / numberColumns;

    float delta = cellWidth/5.0f;
    if (ball.position.x > cellCenterX + delta || ball.position.x < cellCenterX - delta) {
        ball.position.y = cellCenterY;
        ball.velocity.y = 0.0f;
    }

    delta = cellHeight/5.0f;
    if (ball.position.y > cellCenterY + delta || ball.position.y < cellCenterY - delta) {
        ball.position.x = cellCenterX;
        ball.velocity.x = 0.0f;
    }

    float deltax = ball.position.x - cellCenterX;
    float deltay = ball.position.y - cellCenterY;

    if (deltay > cellHeight || deltay < -cellHeight || deltax > cellWidth || deltax < -cellWidth) {
        deltax +=0.00001;
    }

    if (deltay > cellHeight/2.0f && ball.row != numberRows - 1) {
        ball.row++;
    } else if (deltay < -cellHeight/2.0f && ball.row != 0) {
        ball.row--;
    }

    if (deltax > cellWidth/2.0f && ball.col != numberColumns - 1) {
        ball.col++;
    } else if (deltax < -cellWidth/2.0f && ball.col != 0) {
        ball.col--;
    }

    glm::vec3 axis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), ball.velocity);
    if (glm::length(axis) != 0) {
        float scaleFactor = 10.0f;
        glm::quat q = glm::angleAxis(glm::length(ball.velocity)*time*scaleFactor, glm::normalize(axis));

        ball.totalRotated = glm::normalize(q * ball.totalRotated);
    }
    modelMatrixBall = glm::translate(ball.position) * glm::toMat4(ball.totalRotated) * scaleBall;

    bool drawingNecessary = glm::length(ball.position - ball.prevPosition) > 0.01;
    if (drawingNecessary) {
        ball.prevPosition = ball.position;
    }
    return drawingNecessary;
}

void Maze::generate() {
    std::vector<std::pair<unsigned int, unsigned int> > path;
    unsigned int rowStart, columnStart;
    unsigned int rowEnd, columnEnd;

    // the desired distance that they are apart is the average of the number of rows and the number
    // of columns divided by 2.
    unsigned int desiredDistanceSquared = (numberRows+numberColumns)*(numberRows+numberColumns)/16;
    unsigned int distanceSquared;
    do {
        rowStart = random.getUInt(0, numberRows-1);
        columnStart = random.getUInt(0, numberColumns-1);
        rowEnd = random.getUInt(0, numberRows-1);
        columnEnd = random.getUInt(0, numberColumns-1);
        distanceSquared = (rowEnd - rowStart)*(rowEnd - rowStart) +
            (columnEnd - columnStart)*(columnEnd - columnStart);
    } while (distanceSquared < desiredDistanceSquared);

    cells[rowStart][columnStart].mIsStart = true;
    cells[rowEnd][columnEnd].mIsEnd = true;

    cells[rowStart][columnStart].mVisited = true;
    path.push_back(std::make_pair(rowStart, columnStart));

    while (!path.empty()) {
        std::pair<unsigned int, unsigned int> current = path.back();
        if (cells[current.first][current.second].isEnd()) {
            path.pop_back();
            continue;
        }

        std::vector<std::pair<unsigned int, unsigned int> > nextCellOptions;
        addCellOption(current.first-1, current.second, nextCellOptions);
        addCellOption(current.first, current.second-1, nextCellOptions);
        addCellOption(current.first+1, current.second, nextCellOptions);
        addCellOption(current.first, current.second+1, nextCellOptions);

        if (nextCellOptions.empty()) {
            path.pop_back();
            continue;
        }
        unsigned int i = random.getUInt(0, nextCellOptions.size() - 1);

        std::pair<unsigned int, unsigned int> next = nextCellOptions[i];

        if (current.first == next.first) {
            if (next.second < current.second) {
                // the new cell is on the left of the current one.
                cells[current.first][current.second].mLeftWallExists = false;
                cells[next.first][next.second].mRightWallExists = false;
            } else {
                cells[current.first][current.second].mRightWallExists = false;
                cells[next.first][next.second].mLeftWallExists = false;
            }
        } else {
            if (next.first < current.first) {
                // the new cell is on the top of the current one.
                cells[current.first][current.second].mTopWallExists = false;
                cells[next.first][next.second].mBottomWallExists = false;
            } else {
                cells[current.first][current.second].mBottomWallExists = false;
                cells[next.first][next.second].mTopWallExists = false;
            }
        }

        cells[next.first][next.second].mVisited = true;
        path.push_back(next);
    }
}

void Maze::addCellOption(unsigned int r, unsigned int c, std::vector<std::pair<unsigned int, unsigned int> > &options) {
    if (r < numberRows && c < numberColumns && !cells[r][c].visited()) {
        options.push_back(std::make_pair(r, c));
    }
}

Cell const &Maze::getCell(unsigned int row, unsigned int column) {
    return cells[row][column];
}

void Maze::loadModels() {
    loadModel(MODEL_WALL, vertices, indices);
    loadModel(MODEL_BALL, ballVertices, ballIndices);
    loadModel(MODEL_HOLE, holeVertices, holeIndices);
    loadModelFloor();
}

void Maze::loadModelFloor() {
    getQuad(floorVertices, floorIndices);
}

float Maze::getRowCenterPosition(unsigned int row) {
    return 2.0f / (numberRows * numberBlocksPerCell) * (row*numberBlocksPerCell +1.5f) - 1.0f;
}

float Maze::getColumnCenterPosition(unsigned int col) {
    return 2.0f / (numberColumns * numberBlocksPerCell) * (col*numberBlocksPerCell+1.0f) - 1.0f;
}

glm::vec3 Maze::getCellCenterPosition(unsigned int row, unsigned int col) {
   return glm::vec3(getColumnCenterPosition(col),
                    getRowCenterPosition(row),
                    -1.0f - 3.0f/(2.0f*(numberRows+numberColumns)));
}

void Maze::generateModelMatrices() {
    unsigned int rowEnd;
    unsigned int colEnd;
    bool wallsExist[numberRows*numberBlocksPerCell+1][numberColumns*numberBlocksPerCell+1];
    memset(wallsExist, '\0', sizeof (wallsExist));
    for (unsigned int i = 0; i < numberRows*numberBlocksPerCell; i+=numberBlocksPerCell) {
        for (unsigned int j = 0; j < numberColumns * numberBlocksPerCell; j += numberBlocksPerCell) {
            Cell const &cell = getCell(i / numberBlocksPerCell, j / numberBlocksPerCell);
            if (cell.topWallExists()) {
                for (unsigned int k = 0; k <= numberBlocksPerCell; k++) {
                    wallsExist[i][j + k] = true;
                }
            }

            if (cell.leftWallExists()) {
                for (unsigned int k = 0; k < numberBlocksPerCell+1; k++) {
                    wallsExist[i+k][j] = true;
                }
            }

            // the ball
            if (cell.isStart()) {
                ball.row = i / numberBlocksPerCell;
                ball.col = j / numberBlocksPerCell;
            }

            // the hole
            if (cell.isEnd()) {
                rowEnd = i/numberBlocksPerCell;
                colEnd = j/numberBlocksPerCell;
            }
        }
        // right border
        for (unsigned int k = 0; k < numberBlocksPerCell; k++) {
            wallsExist[i+k][numberColumns * numberBlocksPerCell] = true;
        }
    }

    // bottom wall.
    for (unsigned int i = 0; i < numberColumns*numberBlocksPerCell+1; i++) {
        wallsExist[numberRows*numberBlocksPerCell][i] = true;
    }


    glm::mat4 trans;
    glm::mat4 scale  = glm::scale(glm::vec3(1.0f/(numberColumns*numberBlocksPerCell),
                                            1.0f/(numberRows*numberBlocksPerCell),
                                            1.0f/(numberRows+numberColumns)));

    // Create the model matrices.

    // the walls.
    for (unsigned int i = 0; i < numberRows*numberBlocksPerCell+1; i++) {
        for (unsigned int j = 0; j < numberColumns*numberBlocksPerCell+1; j++) {
            if (wallsExist[i][j]) {
                trans = glm::translate(
                        glm::vec3(2.0f / (numberColumns * numberBlocksPerCell) * j - 1.0f,
                                  2.0f / (numberRows * numberBlocksPerCell) * i - 1.0f,
                                  -1.0f));
                modelMatricesMaze.push_back(trans * scale);
            }
        }
    }

    // the ball
    ball.position = getCellCenterPosition(ball.row, ball.col);

    // cause the frame to be drawn when the program comes up for the first time.
    ball.prevPosition = {-10.0f,0.0f,0.0f};

    trans = glm::translate(ball.position);
    modelMatrixBall = trans*scaleBall;

    // the hole
    trans = glm::translate(getCellCenterPosition(rowEnd, colEnd));
    modelMatrixHole = trans*scaleBall;

    // the floor.
    floorModelMatrix = glm::translate(glm::vec3(0.0f, 0.0f, -1.0f -3.0f/(2.0f*(numberRows+numberColumns))));
}

bool Maze::updateStaticDrawObjects(DrawObjectTable &objs) {
    if (objs.size() > 0) {
        // The objects were already updated, add nothing, update nothing.
        // These objects do not change or move.
        return false;
    }

    // the floor
    std::shared_ptr<DrawObject> floor(new DrawObject());
    floor->indices = floorIndices;
    floor->vertices = floorVertices;
    floor->imagePath = floorTexture;
    floor->modelMatrices.push_back(floorModelMatrix);
    objs.push_back(std::make_pair(floor, std::shared_ptr<DrawObjectData>()));

    // the hole
    std::shared_ptr<DrawObject> hole(new DrawObject());
    hole->indices = holeIndices;
    hole->vertices = holeVertices;
    hole->imagePath = holeTexture;
    hole->modelMatrices.push_back(modelMatrixHole);
    objs.push_back(std::make_pair(hole, std::shared_ptr<DrawObjectData>()));

    if (wallTextures.size() == 0) {
        throw std::runtime_error("Maze wall textures not initialized.");
    }

    // the walls
    std::vector<std::shared_ptr<DrawObject> > wallObjs;

    for (size_t i = 0; i < wallTextures.size(); i++) {
        std::shared_ptr<DrawObject> wall(new DrawObject());
        wallObjs.push_back(wall);
    }

    for (auto && modelMatrixMaze : modelMatricesMaze) {
        uint32_t index = random.getUInt(0, wallTextures.size() - 1);
        wallObjs[index]->modelMatrices.push_back(modelMatrixMaze);
    }

    for (size_t i = 0; i < wallObjs.size(); i++) {
        if (wallObjs[i]->modelMatrices.size() > 0) {
            wallObjs[i]->indices = indices;
            wallObjs[i]->vertices = vertices;
            wallObjs[i]->imagePath = wallTextures[i];
            objs.push_back(std::make_pair(wallObjs[i], std::shared_ptr<DrawObjectData>()));
        }
    }

    return true;
}

bool Maze::updateDynamicDrawObjects(DrawObjectTable &objs) {
    if (objs.size() == 0) {
        objs.push_back(std::make_pair(std::shared_ptr<DrawObject>(new DrawObject()), std::shared_ptr<DrawObjectData>()));
        DrawObject *ballObj = objs[0].first.get();
        ballObj->indices = ballIndices;
        ballObj->vertices = ballVertices;
        ballObj->imagePath = ballTexture;
    }

    // the ball
    DrawObject *ballObj = objs[0].first.get();
    if (ballObj->modelMatrices.size() == 0) {
        ballObj->modelMatrices.push_back(modelMatrixBall);
    } else {
        ballObj->modelMatrices[0] = modelMatrixBall;
    }

    return true;
}
