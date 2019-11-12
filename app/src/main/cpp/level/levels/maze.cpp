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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <istream>
#include <cstring>
#include <queue>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "maze.hpp"
#include "../../android.hpp"
#include "../../random.hpp"
#include "../../graphics.hpp"

static std::string const MODEL_HOLE("models/hole.obj");
constexpr float Maze::viscosity;

void Maze::updateAcceleration(float x, float y, float z) {
    ball.acceleration = glm::vec3(-x,-y,0.0f);
}

bool Maze::ballInProximity(float x, float y) {
    float errDistance = scale / 2;
    return glm::length(ball.position - glm::vec3{x, y, ball.position.z}) < errDistance;
}

bool Maze::updateData() {
    if (m_finished) {
        // the maze is finished, do nothing and return false (drawing is not necessary).
        return false;
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    float difftime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count();
    prevTime = currentTime;

    ball.velocity += ball.acceleration * difftime - viscosity * ball.velocity;
    ball.position += ball.velocity * difftime;

    auto cell = getCell(ball.row, ball.col);
    float cellCenterX = getColumnCenterPosition(ball.col);
    float cellCenterY = getRowCenterPosition(ball.row);// + scale;  // I don't know why I have to add scale here.
    if (cell.isEnd() && ballInProximity(cellCenterX, cellCenterY)) {
        m_finished = true;
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
        glm::quat q = glm::angleAxis(glm::length(ball.velocity)*difftime*scaleFactor, glm::normalize(axis));

        ball.totalRotated = glm::normalize(q * ball.totalRotated);
    }
    modelMatrixBall = glm::translate(ball.position) * glm::toMat4(ball.totalRotated) * scaleBall;

    bool drawingNecessary = glm::length(ball.position - ball.prevPosition) > 0.01;
    if (drawingNecessary) {
        ball.prevPosition = ball.position;
    }
    return drawingNecessary;
}

void Maze::generateDFS() {
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

void Maze::generateBFS() {
    std::list<std::pair<unsigned int, unsigned int> > path;
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
        std::pair<unsigned int, unsigned int> current = path.front();
        path.pop_front();
        if (cells[current.first][current.second].isEnd()) {
            continue;
        }

        std::vector<std::pair<unsigned int, unsigned int> > nextCellOptions;
        addCellOption(current.first - 1, current.second, nextCellOptions);
        addCellOption(current.first, current.second - 1, nextCellOptions);
        addCellOption(current.first + 1, current.second, nextCellOptions);
        addCellOption(current.first, current.second + 1, nextCellOptions);

        if (nextCellOptions.empty()) {
            continue;
        }

        for (auto const &next : nextCellOptions) {
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
            size_t size = path.size();
            unsigned int i = size == 0 ? 0 : random.getUInt(0, path.size());
            auto it = path.begin();
            for (int j = 0; j < i; j++, it++)
                /* do nothing */;
            path.insert(it, next);
        }
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
    loadModel(m_gameRequester->getAssetStream(MODEL_WALL), vertices, indices);
    loadModel(m_gameRequester->getAssetStream(MODEL_BALL), ballVertices, ballIndices);
    getQuad(holeVertices, holeIndices);
    loadModelFloor();
}

void Maze::loadModelFloor() {
    getQuad(floorVertices, floorIndices);
}

float Maze::getRowCenterPosition(unsigned int row) {
    return m_height / (numberRows * numberBlocksPerCell+1) * (row*numberBlocksPerCell +1.5f) - m_height/2;
}

float Maze::getColumnCenterPosition(unsigned int col) {
    return m_width / (numberColumns * numberBlocksPerCell+1) * (col*numberBlocksPerCell+1.5f) - m_width/2;
}

float Maze::getBallZPosition() {
    return m_maxZ - m_originalWallHeight*m_scaleWallZ/2.0f;
}

glm::vec3 Maze::getCellCenterPosition(unsigned int row, unsigned int col) {
   return glm::vec3(getColumnCenterPosition(col),
                    getRowCenterPosition(row),
                    getBallZPosition());
}

void Maze::generateMazeVector(uint32_t &rowEnd, uint32_t &colEnd, std::vector<bool> &wallsExist) {
    wallsExist.resize((numberRows*numberBlocksPerCell+1)*(numberColumns*numberBlocksPerCell+1), false);
    for (unsigned int i = 0; i < numberRows*numberBlocksPerCell; i+=numberBlocksPerCell) {
        for (unsigned int j = 0; j < numberColumns * numberBlocksPerCell; j += numberBlocksPerCell) {
            Cell const &cell = getCell(i / numberBlocksPerCell, j / numberBlocksPerCell);
            if (cell.topWallExists()) {
                for (unsigned int k = 0; k < numberBlocksPerCell+1; k++) {
                    wallsExist[i*(numberColumns*numberBlocksPerCell+1) + j + k] = true;
                }
            }

            if (cell.leftWallExists()) {
                for (unsigned int k = 0; k < numberBlocksPerCell+1; k++) {
                    wallsExist[(i+k)*(numberColumns*numberBlocksPerCell+1) + j] = true;
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
            wallsExist[(i+k) * (numberColumns*numberBlocksPerCell+1) + numberColumns * numberBlocksPerCell] = true;
        }
    }

    // bottom wall.
    for (unsigned int i = 0; i < numberColumns*numberBlocksPerCell+1; i++) {
        wallsExist[numberRows*numberBlocksPerCell*(numberColumns*numberBlocksPerCell+1) + i] = true;
    }

}

Maze::MazeWallModelMatrixGeneratorFcn Maze::getMazeWallModelMatricesGenerator() {
    return {[](std::vector<bool> const &wallsExist,
            float width,
            float height,
            float maxZ,
            unsigned int nbrCols,
            unsigned int nbrRows,
            float scaleWallZ) -> std::vector<glm::mat4>
    {
        std::vector<glm::mat4> matrices;
        glm::mat4 trans;
        glm::mat4 scaleMat  = glm::scale(glm::vec3(width/2/(nbrCols*numberBlocksPerCell+1),
                                                   height/2/(nbrRows*numberBlocksPerCell+1),
                                                   scaleWallZ));

        // Create the model matrices for the maze walls.
        for (unsigned int i = 0; i < nbrRows*numberBlocksPerCell+1; i++) {
            for (unsigned int j = 0; j < nbrCols*numberBlocksPerCell+1; j++) {
                if (wallsExist[i*(nbrCols*numberBlocksPerCell+1)+j]) {
                    trans = glm::translate(
                            glm::vec3(width / (nbrCols * numberBlocksPerCell+1) * (j + 0.5) - width/2,
                                      height / (nbrRows * numberBlocksPerCell+1) * (i + 0.5) - height/2,
                                      maxZ - m_originalWallHeight * scaleWallZ / 2.0f));
                    matrices.push_back(trans * scaleMat);
                }
            }
        }

        return std::move(matrices);
    }};
}

void Maze::generateModelMatrices(MazeWallModelMatrixGeneratorFcn &wallModelMatrixGeneratorFcn) {
    std::vector<bool> wallsExist;

    generateMazeVector(m_rowEnd, m_colEnd, wallsExist);

    // Create the model matrices.

    // the walls
    modelMatricesMaze = wallModelMatrixGeneratorFcn(wallsExist, m_width, m_height, m_maxZ, numberColumns, numberRows, m_scaleWallZ);

    // the ball
    ball.position = getCellCenterPosition(ball.row, ball.col);

    // cause the frame to be drawn when the program comes up for the first time.
    ball.prevPosition = {-10.0f,0.0f,0.0f};

    glm::mat4 trans = glm::translate(ball.position);
    modelMatrixBall = trans*glm::toMat4(ball.totalRotated)*scaleBall;

    // the hole
    glm::vec3 holePos = getCellCenterPosition(m_rowEnd, m_colEnd);
    trans = glm::translate(holePos);
    modelMatrixHole = trans*scaleBall;

    // the floor.
    floorModelMatrix = glm::translate(glm::vec3(0.0f, 0.0f, m_maxZ - m_originalWallHeight * m_scaleWallZ)) *
            glm::scale(glm::vec3(m_width/2 + m_width / 2 / (numberColumns * numberBlocksPerCell),
                                 m_height/2 + m_height / 2 /(numberRows * numberBlocksPerCell), 1.0f));
}

void Maze::doneAddingWallTextures() {
    if (m_wallTextureIndices.empty()) {
        m_wallTextureIndices.reserve(modelMatricesMaze.size());
        for (size_t i = 0; i < modelMatricesMaze.size(); i++) {
            m_wallTextureIndices.push_back(random.getUInt(0, wallTextures.size() - 1));
        }
    }
}

bool Maze::updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) {
    if (!objs.empty()) {
        // The objects were already updated, add nothing, update nothing.
        // These objects do not change or move.
        return false;
    }

    // the floor
    std::shared_ptr<DrawObject> floor(new DrawObject());
    floor->indices = floorIndices;
    floor->vertices = floorVertices;
    floor->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester, floorTexture);
    textures.insert(std::make_pair(floor->texture, std::shared_ptr<TextureData>()));
    floor->modelMatrices.push_back(floorModelMatrix);
    objs.push_back(std::make_pair(floor, std::shared_ptr<DrawObjectData>()));

    if (drawHole) {
        // the hole
        std::shared_ptr<DrawObject> hole(new DrawObject());
        hole->indices = holeIndices;
        hole->vertices = holeVertices;
        hole->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester, holeTexture);
        textures.insert(std::make_pair(hole->texture, std::shared_ptr<TextureData>()));
        hole->modelMatrices.push_back(modelMatrixHole);
        objs.push_back(std::make_pair(hole, std::shared_ptr<DrawObjectData>()));
    }

    if (wallTextures.empty()) {
        throw std::runtime_error("Maze wall textures not initialized.");
    }

    // the walls
    std::vector<std::shared_ptr<DrawObject> > wallObjs;

    for (size_t i = 0; i < wallTextures.size(); i++) {
        std::shared_ptr<DrawObject> wall(new DrawObject());
        wallObjs.push_back(wall);
    }

    for (size_t i = 0; i < m_wallTextureIndices.size(); i++) {
        wallObjs[m_wallTextureIndices[i]]->modelMatrices.push_back(modelMatricesMaze[i]);
    }

    for (size_t i = 0; i < wallObjs.size(); i++) {
        if (!wallObjs[i]->modelMatrices.empty()) {
            wallObjs[i]->indices = indices;
            wallObjs[i]->vertices = vertices;
            wallObjs[i]->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester, wallTextures[i]);
            textures.insert(std::make_pair(wallObjs[i]->texture, std::shared_ptr<TextureData>()));
            objs.push_back(std::make_pair(wallObjs[i], std::shared_ptr<DrawObjectData>()));
        }
    }

    return true;
}

bool Maze::updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged) {
    if (objs.empty()) {
        texturesChanged = true;
        objs.push_back(std::make_pair(std::shared_ptr<DrawObject>(new DrawObject()), std::shared_ptr<DrawObjectData>()));
        DrawObject *ballObj = objs[0].first.get();
        ballObj->indices = ballIndices;
        ballObj->vertices = ballVertices;
        ballObj->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester, ballTexture);
        textures.insert(std::make_pair(ballObj->texture, std::shared_ptr<TextureData>()));
    } else {
        texturesChanged = false;
    }

    // the ball
    DrawObject *ballObj = objs[0].first.get();
    if (ballObj->modelMatrices.empty()) {
        ballObj->modelMatrices.push_back(modelMatrixBall);
    } else {
        ballObj->modelMatrices[0] = modelMatrixBall;
    }

    return true;
}
