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

#include "mazeOpenArea.hpp"
#include "maze.hpp"

bool MazeOpenArea::updateData() {
    if (m_finished) {
        // the maze is finished, do nothing and return false (drawing is not necessary).
        return false;
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count();
    prevTime = currentTime;

    ball.velocity += ball.acceleration * time - viscosity * ball.velocity;
    ball.position += ball.velocity * time;

    float halfWallWidth = m_width/2/3/(numberColumns*numberBlocksPerCell);
    auto cell = getCell(ball.row, ball.col);
    if (cell.leftWallExists() && ball.position.x < leftWall(ball.col) + scale + halfWallWidth) {
        if (ball.velocity.x < 0.0f) {
            ball.velocity.x = 0.0f;
        }
        ball.position.x = leftWall(ball.col) + scale + halfWallWidth;
    }
    if (cell.rightWallExists() && ball.position.x > rightWall(ball.col) - scale - halfWallWidth) {
        if (ball.velocity.x > 0.0f) {
            ball.velocity.x = 0.0f;
        }
        ball.position.x = rightWall(ball.col) - scale - halfWallWidth;
    }
    if (cell.bottomWallExists() && ball.position.y > bottomWall(ball.row) - scale - halfWallWidth) {
        if (ball.velocity.y > 0.0f) {
            ball.velocity.y = 0.0f;
        }
        ball.position.y = bottomWall(ball.row) - scale - halfWallWidth;
    }
    if (cell.topWallExists() && ball.position.y < topWall(ball.row) + scale + halfWallWidth) {
        if (ball.velocity.y < 0.0f) {
            ball.velocity.y = 0.0f;
        }
        ball.position.y = topWall(ball.row) + scale + halfWallWidth;
    }

    float cellHeight = m_height / (numberRows*numberBlocksPerCell+1)*numberBlocksPerCell;
    float cellWidth = m_width / (numberColumns*numberBlocksPerCell+1)*numberBlocksPerCell;

    float cellCenterX = getColumnCenterPosition(ball.col);
    float cellCenterY = getRowCenterPosition(ball.row);
    float deltax = ball.position.x - cellCenterX;
    float deltay = ball.position.y - cellCenterY;

    if (deltay > cellHeight*2.0f/3.0f && ball.row != numberRows - 1) {
        ball.row++;
    } else if (deltay < -cellHeight*2.0f/3.0f && ball.row != 0) {
        ball.row--;
    }

    if (deltax > cellWidth*2.0f/3.0f && ball.col != numberColumns - 1) {
        ball.col++;
    } else if (deltax < -cellWidth*2.0f/3.0f && ball.col != 0) {
        ball.col--;
    }

    if (checkFinishCondition(time)) {
        m_finished = true;
        return true;
    }

    glm::vec3 axis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), ball.velocity);
    if (glm::length(axis) != 0) {
        float scaleFactor = 10.0f;
        glm::quat q = glm::angleAxis(glm::length(ball.velocity)*time*scaleFactor, glm::normalize(axis));

        ball.totalRotated = glm::normalize(q * ball.totalRotated);
    }
    modelMatrixBall = glm::translate(glm::mat4(1.0f), ball.position) * glm::mat4_cast(ball.totalRotated) * scaleBall;

    bool drawingNecessary = glm::length(ball.position - ball.prevPosition) > 0.007;
    if (drawingNecessary) {
        ball.prevPosition = ball.position;
    }
    return drawingNecessary;
}

Maze::MazeWallModelMatrixGeneratorFcn MazeOpenArea::getMazeWallModelMatricesGenerator() {
    return {[] (std::vector<bool> const &wallsExist,
                float width,
                float height,
                float maxZ,
                unsigned int nbrCols,
                unsigned int nbrRows,
                float scaleWallZ) -> std::vector<glm::mat4> {
        std::vector<glm::mat4> matrices;
        glm::mat4 trans;
        glm::mat4 scale  = glm::scale(glm::mat4(1.0f),
                glm::vec3(width/2/3/(nbrCols*numberBlocksPerCell),
                          height/2/3/(nbrRows*numberBlocksPerCell),
                          scaleWallZ));

        // Create the model matrices for the maze walls.
        for (unsigned int i = 0; i < nbrRows*numberBlocksPerCell+1; i++) {
            for (unsigned int j = 0; j < nbrCols*numberBlocksPerCell+1; j++) {
                if (wallsExist[i*(nbrCols*numberBlocksPerCell+1)+j]) {
                    trans = glm::translate(glm::mat4(1.0f),
                            glm::vec3(width / (nbrCols * numberBlocksPerCell+1) * (j + 0.5f) - width/2,
                                      height / (nbrRows * numberBlocksPerCell+1) * (i + 0.5f) - height/2,
                                      maxZ - m_originalWallHeight * scaleWallZ / 2.0f));
                    matrices.push_back(trans * scale);
                    if (j > 0 && wallsExist[i*(nbrCols*numberBlocksPerCell+1) + j - 1]) {
                        trans = glm::translate(glm::mat4(1.0f),
                                glm::vec3(width / (nbrCols * numberBlocksPerCell+1) * (j + 0.5f) -
                                          width/2 - width/3/(nbrCols*numberBlocksPerCell+1),
                                          height / (nbrRows * numberBlocksPerCell+1) * (i + 0.5f) - height/2,
                                          maxZ - m_originalWallHeight * scaleWallZ / 2.0f));
                        matrices.push_back(trans * scale);
                    }
                    if (j < nbrCols*numberBlocksPerCell && wallsExist[i*(nbrCols*numberBlocksPerCell+1)+j+1]) {
                        trans = glm::translate(glm::mat4(1.0f),
                                glm::vec3(width / (nbrCols * numberBlocksPerCell+1) * (j + 0.5f) -
                                          width/2 + width/3/(nbrCols*numberBlocksPerCell+1),
                                          height / (nbrRows * numberBlocksPerCell+1) * (i + 0.5f) - height/2,
                                          maxZ - m_originalWallHeight * scaleWallZ / 2.0f));
                        matrices.push_back(trans * scale);
                    }
                    if (i > 0 && wallsExist[(i-1)*(nbrCols*numberBlocksPerCell+1)+j]) {
                        trans = glm::translate(glm::mat4(1.0f),
                                glm::vec3(width / (nbrCols * numberBlocksPerCell+1) * (j + 0.5f) - width/2,
                                          height / (nbrRows * numberBlocksPerCell+1) * (i + 0.5f) - height/2 -
                                          height/3/(nbrRows*numberBlocksPerCell+1),
                                          maxZ - m_originalWallHeight * scaleWallZ / 2.0f));
                        matrices.push_back(trans * scale);
                    }
                    if (i < nbrRows*numberBlocksPerCell && wallsExist[(i+1)*(nbrCols*numberBlocksPerCell+1)+j]) {
                        trans = glm::translate(glm::mat4(1.0f),
                                glm::vec3(width / (nbrCols * numberBlocksPerCell+1) * (j + 0.5f) - width/2,
                                          height / (nbrRows * numberBlocksPerCell+1) * (i + 0.5f) - height/2 +
                                          height/3/(nbrRows*numberBlocksPerCell+1),
                                          maxZ - m_originalWallHeight * scaleWallZ / 2.0f));
                        matrices.push_back(trans * scale);
                    }
                }
            }
        }

        return std::move(matrices);
    }};
}
