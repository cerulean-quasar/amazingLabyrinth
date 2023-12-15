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
#include <vector>
#include "../generatedMaze/level.hpp"
#include "level.hpp"

namespace openAreaMaze {
    bool Level::updateData() {
        if (m_finished) {
            // the maze is finished, do nothing and return false (drawing is not necessary).
            return false;
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        float timeDiff = std::chrono::duration<float, std::chrono::seconds::period>(
                currentTime - prevTime).count();
        prevTime = currentTime;

        m_ball.velocity = getUpdatedVelocity(m_ball.acceleration, timeDiff);
        m_ball.position += m_ball.velocity * timeDiff;

        size_t numberRows = m_mazeBoard.numberRows();
        size_t numberColumns = m_mazeBoard.numberColumns();

        float halfWallWidth = m_width / 2 / 3 / (numberColumns * numberBlocksPerCell + 1);
        auto cell = m_mazeBoard.getCell(m_ballCell.row, m_ballCell.col);
        if (cell.leftWallExists() &&
            m_ball.position.x < leftWall(m_ballCell.col) + ballRadius() + halfWallWidth) {
            if (m_ball.velocity.x < 0.0f) {
                m_ball.velocity.x = 0.0f;
            }
            m_ball.position.x = leftWall(m_ballCell.col) + ballRadius() + halfWallWidth;
        }
        if (cell.rightWallExists() &&
            m_ball.position.x > rightWall(m_ballCell.col) - ballRadius() - halfWallWidth) {
            if (m_ball.velocity.x > 0.0f) {
                m_ball.velocity.x = 0.0f;
            }
            m_ball.position.x = rightWall(m_ballCell.col) - ballRadius() - halfWallWidth;
        }
        if (cell.bottomWallExists() &&
            m_ball.position.y > bottomWall(m_ballCell.row) - ballRadius() - halfWallWidth) {
            if (m_ball.velocity.y > 0.0f) {
                m_ball.velocity.y = 0.0f;
            }
            m_ball.position.y = bottomWall(m_ballCell.row) - ballRadius() - halfWallWidth;
        }
        if (cell.topWallExists() &&
            m_ball.position.y < topWall(m_ballCell.row) + ballRadius() + halfWallWidth) {
            if (m_ball.velocity.y < 0.0f) {
                m_ball.velocity.y = 0.0f;
            }
            m_ball.position.y = topWall(m_ballCell.row) + ballRadius() + halfWallWidth;
        }

        float cellHeight = m_height / (numberRows * numberBlocksPerCell + 1) * numberBlocksPerCell;
        float cellWidth = m_width / (numberColumns * numberBlocksPerCell + 1) * numberBlocksPerCell;

        float cellCenterX = getColumnCenterPosition(m_ballCell.col);
        float cellCenterY = getRowCenterPosition(m_ballCell.row);
        float deltax = m_ball.position.x - cellCenterX;
        float deltay = m_ball.position.y - cellCenterY;

        int rowinc = 0;
        int colinc = 0;
        if (deltay > cellHeight / 2.0f && m_ballCell.row != numberRows - 1) {
            rowinc++;
        } else if (deltay < -cellHeight / 2.0f && m_ballCell.row != 0) {
            rowinc--;
        }

        if (deltax > cellWidth / 2.0f && m_ballCell.col != numberColumns - 1) {
            colinc++;
        } else if (deltax < -cellWidth / 2.0f && m_ballCell.col != 0) {
            colinc--;
        }

        // stop balls from going through the corners of the maze.  If both rows and columns are
        // changing, the ball could go through a corner if the ball is in the cell were two would be
        // touching walls are not there.  If this is the case, as long as the ball has not registered
        // as being in the other cell, it is ok to go through the wall on that side.
        if (rowinc != 0 && colinc != 0) {
            colinc = 0;
        }

        m_ballCell.row += rowinc;
        m_ballCell.col += colinc;

        if (checkFinishCondition(timeDiff)) {
            m_finished = true;
            return true;
        }

        updateRotation(timeDiff);
        modelMatrixBall = glm::translate(glm::mat4(1.0f), m_ball.position) *
                          glm::mat4_cast(m_ball.totalRotated) * scaleBall;

        return drawingNecessary();
    }

    generatedMaze::Level::MazeWallModelMatrixGeneratorFcn Level::getMazeWallModelMatricesGenerator() {
        return {[](std::vector<bool> const &wallsExist,
                   float width,
                   float height,
                   float maxZ,
                   unsigned int nbrCols,
                   unsigned int nbrRows,
                   float scaleWallZ) -> std::vector <glm::mat4> {
            std::vector <glm::mat4> matrices;
            glm::mat4 trans;
            glm::mat4 scale = glm::scale(glm::mat4(1.0f),
                                         glm::vec3(width / 2 / 3 / (nbrCols * numberBlocksPerCell),
                                                   height / 2 / 3 / (nbrRows * numberBlocksPerCell),
                                                   scaleWallZ));

            // Create the model matrices for the maze walls.
            for (unsigned int i = 0; i < nbrRows * numberBlocksPerCell + 1; i++) {
                for (unsigned int j = 0; j < nbrCols * numberBlocksPerCell + 1; j++) {
                    if (wallsExist[i * (nbrCols * numberBlocksPerCell + 1) + j]) {
                        trans = glm::translate(glm::mat4(1.0f),
                                               glm::vec3(
                                                       width / (nbrCols * numberBlocksPerCell + 1) *
                                                       (j + 0.5f) - width / 2,
                                                       height /
                                                       (nbrRows * numberBlocksPerCell + 1) *
                                                       (i + 0.5f) - height / 2,
                                                       maxZ -
                                                       m_originalWallHeight * scaleWallZ / 2.0f));
                        matrices.push_back(trans * scale);
                        if (j > 0 && wallsExist[i * (nbrCols * numberBlocksPerCell + 1) + j - 1]) {
                            trans = glm::translate(glm::mat4(1.0f),
                                                   glm::vec3(width /
                                                             (nbrCols * numberBlocksPerCell + 1) *
                                                             (j + 0.5f) -
                                                             width / 2 - width / 3 / (nbrCols *
                                                                                      numberBlocksPerCell +
                                                                                      1),
                                                             height /
                                                             (nbrRows * numberBlocksPerCell + 1) *
                                                             (i + 0.5f) - height / 2,
                                                             maxZ -
                                                             m_originalWallHeight * scaleWallZ /
                                                             2.0f));
                            matrices.push_back(trans * scale);
                        }
                        if (j < nbrCols * numberBlocksPerCell &&
                            wallsExist[i * (nbrCols * numberBlocksPerCell + 1) + j + 1]) {
                            trans = glm::translate(glm::mat4(1.0f),
                                                   glm::vec3(width /
                                                             (nbrCols * numberBlocksPerCell + 1) *
                                                             (j + 0.5f) -
                                                             width / 2 + width / 3 / (nbrCols *
                                                                                      numberBlocksPerCell +
                                                                                      1),
                                                             height /
                                                             (nbrRows * numberBlocksPerCell + 1) *
                                                             (i + 0.5f) - height / 2,
                                                             maxZ -
                                                             m_originalWallHeight * scaleWallZ /
                                                             2.0f));
                            matrices.push_back(trans * scale);
                        }
                        if (i > 0 &&
                            wallsExist[(i - 1) * (nbrCols * numberBlocksPerCell + 1) + j]) {
                            trans = glm::translate(glm::mat4(1.0f),
                                                   glm::vec3(width /
                                                             (nbrCols * numberBlocksPerCell + 1) *
                                                             (j + 0.5f) - width / 2,
                                                             height /
                                                             (nbrRows * numberBlocksPerCell + 1) *
                                                             (i + 0.5f) - height / 2 -
                                                             height / 3 /
                                                             (nbrRows * numberBlocksPerCell + 1),
                                                             maxZ -
                                                             m_originalWallHeight * scaleWallZ /
                                                             2.0f));
                            matrices.push_back(trans * scale);
                        }
                        if (i < nbrRows * numberBlocksPerCell &&
                            wallsExist[(i + 1) * (nbrCols * numberBlocksPerCell + 1) + j]) {
                            trans = glm::translate(glm::mat4(1.0f),
                                                   glm::vec3(width /
                                                             (nbrCols * numberBlocksPerCell + 1) *
                                                             (j + 0.5f) - width / 2,
                                                             height /
                                                             (nbrRows * numberBlocksPerCell + 1) *
                                                             (i + 0.5f) - height / 2 +
                                                             height / 3 /
                                                             (nbrRows * numberBlocksPerCell + 1),
                                                             maxZ -
                                                             m_originalWallHeight * scaleWallZ /
                                                             2.0f));
                            matrices.push_back(trans * scale);
                        }
                    }
                }
            }

            return std::move(matrices);
        }};
    }

} // namespace openAreaMaze
