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
    modelMatrixBall = glm::translate(ball.position) * glm::toMat4(ball.totalRotated) * scaleBall;

    bool drawingNecessary = glm::length(ball.position - ball.prevPosition) > 0.007;
    if (drawingNecessary) {
        ball.prevPosition = ball.position;
    }
    return drawingNecessary;
}

void MazeOpenArea::generateModelMatrices() {
    unsigned int rowEnd;
    unsigned int colEnd;
    std::vector<bool> wallsExist;

    generateMazeVector(m_rowEnd, m_colEnd, wallsExist);

    glm::mat4 trans;
    glm::mat4 scale  = glm::scale(glm::vec3(m_width/2/3/(numberColumns*numberBlocksPerCell),
                                            m_height/2/3/(numberRows*numberBlocksPerCell),
                                            m_scaleWallZ));

    // Create the model matrices.

    // the walls.
    for (unsigned int i = 0; i < numberRows*numberBlocksPerCell+1; i++) {
        for (unsigned int j = 0; j < numberColumns*numberBlocksPerCell+1; j++) {
            if (wallsExist[i*(numberColumns*numberBlocksPerCell+1)+j]) {
                trans = glm::translate(
                        glm::vec3(m_width / (numberColumns * numberBlocksPerCell+1) * (j + 0.5f) - m_width/2,
                                  m_height / (numberRows * numberBlocksPerCell+1) * (i + 0.5f) - m_height/2,
                                  m_maxZ - m_originalWallHeight * m_scaleWallZ / 2.0f));
                modelMatricesMaze.push_back(trans * scale);
                if (j > 0 && wallsExist[i*(numberColumns*numberBlocksPerCell+1) + j - 1]) {
                    trans = glm::translate(
                            glm::vec3(m_width / (numberColumns * numberBlocksPerCell+1) * (j + 0.5f) -
                                          m_width/2 - m_width/3/(numberColumns*numberBlocksPerCell+1),
                                      m_height / (numberRows * numberBlocksPerCell+1) * (i + 0.5f) - m_height/2,
                                      m_maxZ - m_originalWallHeight * m_scaleWallZ / 2.0f));
                    modelMatricesMaze.push_back(trans * scale);
                }
                if (j < numberColumns*numberBlocksPerCell && wallsExist[i*(numberColumns*numberBlocksPerCell+1)+j+1]) {
                    trans = glm::translate(
                            glm::vec3(m_width / (numberColumns * numberBlocksPerCell+1) * (j + 0.5f) -
                                          m_width/2 + m_width/3/(numberColumns*numberBlocksPerCell+1),
                                      m_height / (numberRows * numberBlocksPerCell+1) * (i + 0.5f) - m_height/2,
                                      m_maxZ - m_originalWallHeight * m_scaleWallZ / 2.0f));
                    modelMatricesMaze.push_back(trans * scale);
                }
                if (i > 0 && wallsExist[(i-1)*(numberColumns*numberBlocksPerCell+1)+j]) {
                    trans = glm::translate(
                            glm::vec3(m_width / (numberColumns * numberBlocksPerCell+1) * (j + 0.5f) - m_width/2,
                                      m_height / (numberRows * numberBlocksPerCell+1) * (i + 0.5f) - m_height/2 -
                                         m_height/3/(numberRows*numberBlocksPerCell+1),
                                      m_maxZ - m_originalWallHeight * m_scaleWallZ / 2.0f));
                    modelMatricesMaze.push_back(trans * scale);
                }
                if (i < numberRows*numberBlocksPerCell && wallsExist[(i+1)*(numberColumns*numberBlocksPerCell+1)+j]) {
                    trans = glm::translate(
                            glm::vec3(m_width / (numberColumns * numberBlocksPerCell+1) * (j + 0.5f) - m_width/2,
                                      m_height / (numberRows * numberBlocksPerCell+1) * (i + 0.5f) - m_height/2 +
                                          m_height/3/(numberRows*numberBlocksPerCell+1),
                                      m_maxZ - m_originalWallHeight * m_scaleWallZ / 2.0f));
                    modelMatricesMaze.push_back(trans * scale);
                }
            }
        }
    }

    // the ball
    ball.position = getCellCenterPosition(ball.row, ball.col);

    // cause the frame to be drawn when the program comes up for the first time.
    ball.prevPosition = {-10.0f,0.0f,0.0f};

    trans = glm::translate(ball.position);
    modelMatrixBall = trans*glm::toMat4(ball.totalRotated)*scaleBall;

    // the hole
    trans = glm::translate(getCellCenterPosition(m_rowEnd, m_colEnd));
    modelMatrixHole = trans*scaleBall;

    // the floor.
    floorModelMatrix = glm::translate(glm::vec3(0.0f, 0.0f, m_maxZ - m_originalWallHeight * m_scaleWallZ)) *
                       glm::scale(glm::vec3(m_width/2 + m_width / 2 / (numberColumns * numberBlocksPerCell),
                                            m_height/2 + m_height / 2 /(numberRows * numberBlocksPerCell), 1.0f));
}

