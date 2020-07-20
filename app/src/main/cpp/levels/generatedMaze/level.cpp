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
#include <list>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "level.hpp"
#include "../basic/level.hpp"

namespace generatedMaze {
    char constexpr const *Level::m_name;

    bool Level::ballInProximity(float x, float y) {
        float errDistance = ballRadius();
        return glm::length(m_ball.position - glm::vec3{x, y, m_ball.position.z}) < errDistance;
    }

    bool Level::updateData() {
        if (m_finished) {
            // the maze is finished, do nothing and return false (drawing is not necessary).
            return false;
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        float difftime = std::chrono::duration<float, std::chrono::seconds::period>(
                currentTime - prevTime).count();
        prevTime = currentTime;

        m_ball.velocity = getUpdatedVelocity(m_ball.acceleration, difftime);
        m_ball.position += m_ball.velocity * difftime;

        auto cell = m_mazeBoard.getCell(m_ballCell.row, m_ballCell.col);
        float cellCenterX = getColumnCenterPosition(m_ballCell.col);
        float cellCenterY = getRowCenterPosition(m_ballCell.row);

        size_t numberRows = m_mazeBoard.numberRows();
        size_t numberColumns = m_mazeBoard.numberColumns();

        if (cell.isEnd() && ballInProximity(cellCenterX, cellCenterY)) {
            m_finished = true;
            m_ball.position.x = cellCenterX;
            m_ball.position.y = cellCenterY;
            m_ball.velocity = {0.0f, 0.0f, 0.0f};
            return true;
        }
        if (cell.leftWallExists() && m_ball.position.x < cellCenterX) {
            if (m_ball.velocity.x < 0.0f) {
                m_ball.velocity.x = 0.0f;
            }
            m_ball.position.x = cellCenterX;
        }
        if (cell.rightWallExists() && cellCenterX < m_ball.position.x) {
            if (m_ball.velocity.x > 0.0f) {
                m_ball.velocity.x = 0.0f;
            }
            m_ball.position.x = cellCenterX;
        }
        if (cell.bottomWallExists() && cellCenterY < m_ball.position.y) {
            if (m_ball.velocity.y > 0.0f) {
                m_ball.velocity.y = 0.0f;
            }
            m_ball.position.y = cellCenterY;
        }
        if (cell.topWallExists() && m_ball.position.y < cellCenterY) {
            if (m_ball.velocity.y < 0.0f) {
                m_ball.velocity.y = 0.0f;
            }
            m_ball.position.y = cellCenterY;
        }

        float cellHeight = m_height / (numberRows * numberBlocksPerCell + 1) * numberBlocksPerCell;
        float cellWidth = m_width / (numberColumns * numberBlocksPerCell + 1) * numberBlocksPerCell;

        float delta = cellWidth / 5.0f;
        if (m_ball.position.x > cellCenterX + delta || m_ball.position.x < cellCenterX - delta) {
            m_ball.position.y = cellCenterY;
            m_ball.velocity.y = 0.0f;
        }

        delta = cellHeight / 5.0f;
        if (m_ball.position.y > cellCenterY + delta || m_ball.position.y < cellCenterY - delta) {
            m_ball.position.x = cellCenterX;
            m_ball.velocity.x = 0.0f;
        }

        float deltax = m_ball.position.x - cellCenterX;
        float deltay = m_ball.position.y - cellCenterY;

        if (deltay > cellHeight / 2.0f && m_ballCell.row != numberRows - 1) {
            m_ballCell.row++;
        } else if (deltay < -cellHeight / 2.0f && m_ballCell.row != 0) {
            m_ballCell.row--;
        }

        if (deltax > cellWidth / 2.0f && m_ballCell.col != numberColumns - 1) {
            m_ballCell.col++;
        } else if (deltax < -cellWidth / 2.0f && m_ballCell.col != 0) {
            m_ballCell.col--;
        }

        updateRotation(difftime);
        modelMatrixBall = glm::translate(glm::mat4(1.0f), m_ball.position) *
                          glm::mat4_cast(m_ball.totalRotated) * scaleBall;

        return drawingNecessary();
    }

    float Level::getRowCenterPosition(unsigned int row) {
        return m_height / (m_mazeBoard.numberRows() * numberBlocksPerCell + 1) *
               (row * numberBlocksPerCell + 1.5f) - m_height / 2;
    }

    float Level::getColumnCenterPosition(unsigned int col) {
        return m_width / (m_mazeBoard.numberColumns() * numberBlocksPerCell + 1) *
               (col * numberBlocksPerCell + 1.5f) - m_width / 2;
    }

    float Level::getBallZPosition() {
        return m_mazeFloorZ - m_originalWallHeight * m_scaleWallZ / 2.0f;
    }

    glm::vec3 Level::getCellCenterPosition(unsigned int row, unsigned int col) {
        return glm::vec3(getColumnCenterPosition(col),
                         getRowCenterPosition(row),
                         getBallZPosition());
    }

    void Level::generateMazeVector(std::vector<bool> &wallsExist) {
        size_t numberRows = m_mazeBoard.numberRows();
        size_t numberColumns = m_mazeBoard.numberColumns();

        wallsExist.resize(
                (numberRows * numberBlocksPerCell + 1) * (numberColumns * numberBlocksPerCell + 1),
                false);
        for (unsigned int i = 0; i < numberRows * numberBlocksPerCell; i += numberBlocksPerCell) {
            for (unsigned int j = 0;
                 j < numberColumns * numberBlocksPerCell; j += numberBlocksPerCell) {
                Cell const &cell = m_mazeBoard.getCell(i / numberBlocksPerCell,
                                                       j / numberBlocksPerCell);
                if (cell.topWallExists()) {
                    for (unsigned int k = 0; k < numberBlocksPerCell + 1; k++) {
                        wallsExist[i * (numberColumns * numberBlocksPerCell + 1) + j + k] = true;
                    }
                }

                if (cell.leftWallExists()) {
                    for (unsigned int k = 0; k < numberBlocksPerCell + 1; k++) {
                        wallsExist[(i + k) * (numberColumns * numberBlocksPerCell + 1) + j] = true;
                    }
                }
            }
            // right border
            for (unsigned int k = 0; k < numberBlocksPerCell; k++) {
                wallsExist[(i + k) * (numberColumns * numberBlocksPerCell + 1) +
                           numberColumns * numberBlocksPerCell] = true;
            }
        }

        // bottom wall.
        for (unsigned int i = 0; i < numberColumns * numberBlocksPerCell + 1; i++) {
            wallsExist[
                    numberRows * numberBlocksPerCell * (numberColumns * numberBlocksPerCell + 1) +
                    i] = true;
        }

        // the ball
        m_ballCell.row = m_mazeBoard.rowStart();
        m_ballCell.col = m_mazeBoard.colStart();
    }

    Level::MazeWallModelMatrixGeneratorFcn Level::getMazeWallModelMatricesGenerator() {
        return {[](std::vector<bool> const &wallsExist,
                   float width,
                   float height,
                   float maxZ,
                   unsigned int nbrCols,
                   unsigned int nbrRows,
                   float scaleWallZ) -> std::vector<glm::mat4> {
            std::vector<glm::mat4> matrices;
            glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f),
                                            glm::vec3(
                                                    width / 2 / (nbrCols * numberBlocksPerCell + 1),
                                                    height / 2 /
                                                    (nbrRows * numberBlocksPerCell + 1),
                                                    scaleWallZ));

            // Create the model matrices for the maze walls.
            for (unsigned int i = 0; i < nbrRows * numberBlocksPerCell + 1; i++) {
                for (unsigned int j = 0; j < nbrCols * numberBlocksPerCell + 1; j++) {
                    if (wallsExist[i * (nbrCols * numberBlocksPerCell + 1) + j]) {
                        glm::mat4 trans = glm::translate(glm::mat4(1.0f),
                                                         glm::vec3(width /
                                                                   (nbrCols * numberBlocksPerCell +
                                                                    1) * (j + 0.5) - width / 2,
                                                                   height /
                                                                   (nbrRows * numberBlocksPerCell +
                                                                    1) * (i + 0.5) - height / 2,
                                                                   maxZ - m_originalWallHeight *
                                                                          scaleWallZ / 2.0f));
                        matrices.push_back(trans * scaleMat);
                    }
                }
            }

            return std::move(matrices);
        }};
    }

    void Level::generateModelMatrices(MazeWallModelMatrixGeneratorFcn &wallModelMatrixGeneratorFcn) {
        size_t numberRows = m_mazeBoard.numberRows();
        size_t numberColumns = m_mazeBoard.numberColumns();

        std::vector<bool> wallsExist;

        generateMazeVector(wallsExist);

        // Create the model matrices.

        // the walls
        modelMatricesMaze = wallModelMatrixGeneratorFcn(wallsExist, m_width, m_height, m_mazeFloorZ,
                                                        numberColumns, numberRows, m_scaleWallZ);

        // the ball
        m_ball.position = getCellCenterPosition(m_ballCell.row, m_ballCell.col);

        // cause the frame to be drawn when the program comes up for the first time.
        m_ball.prevPosition = {-10.0f, 0.0f, 0.0f};

        glm::mat4 trans = glm::translate(glm::mat4(1.0f), m_ball.position);
        modelMatrixBall = trans * glm::mat4_cast(m_ball.totalRotated) * scaleBall;

        // the hole
        glm::vec3 holePos = getCellCenterPosition(m_mazeBoard.rowEnd(), m_mazeBoard.colEnd());
        trans = glm::translate(glm::mat4(1.0f), holePos);
        modelMatrixHole = trans * scaleBall;

        // the floor.
        floorModelMatrix = glm::translate(glm::mat4(1.0f),
                                   glm::vec3(0.0f, 0.0f, m_mazeFloorZ - m_originalWallHeight *
                                                                                 m_scaleWallZ)) *
                           glm::scale(glm::mat4(1.0f),
                                   glm::vec3(m_width / 2 + m_width / 2 /(numberColumns * numberBlocksPerCell),
                                           m_height / 2 + m_height / 2 / (numberRows * numberBlocksPerCell), 1.0f));
    }

    bool Level::updateDrawObjects() {
        m_levelDrawer.updateModelMatrixForObject(m_objIndexBall, m_objDataIndexBall, modelMatrixBall);
        return true;
    }
} // namespace generatedMaze
