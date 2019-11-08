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

#include "movingQuadsLevel.hpp"

constexpr float MovingQuadsLevel::scaleFactor;
constexpr float MovingQuadsLevel::viscosity;

void MovingQuadsLevel::loadModels() {
    loadModel(m_gameRequester->getAssetStream(MODEL_BALL), m_ballVertices, m_ballIndices);
    getQuad(m_quadVertices, m_quadIndices);
}

void MovingQuadsLevel::preGenerate() {
    m_startQuadPosition = {0.0f, -maxY, m_maxZ - m_originalBallDiameter * scaleFactor};
    m_endQuadPosition = {0.0f, maxY, m_maxZ - m_originalBallDiameter * scaleFactor};
    m_startPosition = {0.0f, -maxY, m_maxZ - m_originalBallDiameter * scaleFactor / 2.0f};
    m_quadScaleY = maxY / (numberOfMidQuadRows + 1.0f);

    m_ball.totalRotated = glm::quat();
    m_ball.acceleration = {0.0f, 0.0f, 0.0f};
    m_ball.velocity = {0.0f, 0.0f, 0.0f};
    m_ball.scale = glm::scale(glm::vec3(scaleFactor, scaleFactor, scaleFactor));

    // set to very large previous position (in vector length) so that it will get drawn
    // the first time through.
    m_ball.prevPosition = {-10.0f, 0.0f, m_maxZ - scaleFactor * m_originalBallDiameter / 2.0f};
    m_ball.position = m_startPosition;
}

void MovingQuadsLevel::generate() {
    // The moving quads move at a random speed in the x direction.  There is a random number of
    // them in each row.
    Random randomGenerator;
    for (int i = 0; i < numberOfMidQuadRows; i++) {
        MovingQuadRow row;
        int numberOfQuadsInRow = randomGenerator.getUInt(minQuadsInRow, maxQuadsInRow);
        do {
            row.speed = randomGenerator.getFloat(-maxQuadMovingSpeed, maxQuadMovingSpeed);
        } while (fabsf(row.speed) < minQuadMovingSpeed);

        row.scale = {maxX/numberOfQuadsInRow - spaceBetweenQuadsX*(numberOfQuadsInRow-1), m_quadScaleY, 1.0f};
        float xpos = randomGenerator.getFloat(-maxX, 2.0f*maxX/numberOfQuadsInRow - maxX);
        for (int j = 0; j < numberOfQuadsInRow; j++) {
            glm::vec3 pos{2.0f*maxX/numberOfQuadsInRow*j-xpos,
                          2.0f*maxY/(numberOfMidQuadRows+1)*(i+1)-maxY,
                          m_maxZ-m_originalBallDiameter*scaleFactor};
            row.positions.push_back(pos);
        }

        m_movingQuads.push_back(row);
    }
}

bool MovingQuadsLevel::ballOnQuad(glm::vec3 const &quadCenterPos, float xSize) {
    return m_ball.position.x < quadCenterPos.x + xSize/2 + m_width/10 &&
           m_ball.position.x > quadCenterPos.x - xSize/2 - m_width/10 &&
           m_ball.position.y < quadCenterPos.y + m_quadScaleY/2 + m_height/10 &&
           m_ball.position.y > quadCenterPos.y - m_quadScaleY/2 - m_height/10;
}

bool MovingQuadsLevel::updateData() {
    if (m_finished) {
        // the maze is finished, do nothing and return false (drawing is not necessary).
        return false;
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - m_prevTime).count();
    m_prevTime = currentTime;

    m_ball.velocity += m_ball.acceleration * time - viscosity * m_ball.velocity;
    m_ball.position += m_ball.velocity * time;

    // if the ball is on the end quad, then the user won.
    if (ballOnQuad(m_endQuadPosition, 2*maxX)) {
        m_finished = true;
        return true;
    }

    // If the ball is not on any quads, then it goes back to the beginning.
    bool isOnQuads = false;
    if (ballOnQuad(m_startQuadPosition, 2*maxX)) {
        isOnQuads = true;
    } else {
        for (auto const &movingQuadRow : m_movingQuads) {
            for (auto const &movingQuadPos : movingQuadRow.positions) {
                if (ballOnQuad(movingQuadPos, movingQuadRow.scale.x)) {
                    isOnQuads = true;
                    break;
                }
            }
        }
    }

    if (!isOnQuads) {
        m_ball.position = m_startPosition;
    }

    // Move the moving quads.
    for (auto &movingQuadRow : m_movingQuads) {
        for (auto &movingQuadPos : movingQuadRow.positions) {
            movingQuadPos.x += movingQuadRow.speed * time;
        }
        if ((movingQuadRow.positions[0].x < -maxX && movingQuadRow.speed < 0) ||
            (movingQuadRow.positions[movingQuadRow.positions.size()-1].x > maxX && movingQuadRow.speed > 0)) {
            movingQuadRow.speed = -movingQuadRow.speed;
        }
    }

    float minX = -maxX;
    float minY = -maxY;
    if (m_ball.position.x > maxX) {
        m_ball.position.x = maxX;
        if (m_ball.velocity.x > 0) {
            m_ball.velocity.x = -m_ball.velocity.x;
        }
    }

    if (m_ball.position.x < minX) {
        m_ball.position.x = minX;
        if (m_ball.velocity.x < 0) {
            m_ball.velocity.x = -m_ball.velocity.x;
        }
    }

    if (m_ball.position.y > maxY) {
        m_ball.position.y = maxY;
        if (m_ball.velocity.y > 0) {
            m_ball.velocity.y = -m_ball.velocity.y;
        }
    }

    if (m_ball.position.y < minY) {
        m_ball.position.y = minY;
        if (m_ball.velocity.y < 0) {
            m_ball.velocity.y = -m_ball.velocity.y;
        }
    }

    if (glm::length(m_ball.velocity) != 0) {
        glm::vec3 axis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), m_ball.velocity);
        float scaleFactor = 10.0f;
        glm::quat q = glm::angleAxis(glm::length(m_ball.velocity)*time*scaleFactor, glm::normalize(axis));

        m_ball.totalRotated = glm::normalize(q * m_ball.totalRotated);
    }

    bool drawingNecessary = glm::length(m_ball.position - m_ball.prevPosition) > 0.00005;
    if (drawingNecessary) {
        m_ball.prevPosition = m_ball.position;
    }
    return drawingNecessary;
}

bool MovingQuadsLevel::updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) {
    if (!objs.empty()) {
        return false;
    }

    glm::mat4 scale = glm::scale(glm::vec3{2*maxX, m_quadScaleY, 1.0f});

    // the end quad
    std::shared_ptr<DrawObject> endObj = std::make_shared<DrawObject>();
    endObj->vertices = m_quadVertices;
    endObj->indices = m_quadIndices;
    endObj->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester, m_endQuadTexture);
    textures.insert(std::make_pair(endObj->texture, std::shared_ptr<TextureData>()));
    endObj->modelMatrices.push_back(glm::translate(m_endQuadPosition) * scale);
    objs.push_back(std::make_pair(endObj, std::shared_ptr<DrawObjectData>()));

    // the start quad
    std::shared_ptr<DrawObject> startVortexObj = std::make_shared<DrawObject>();
    startVortexObj->vertices = m_quadVertices;
    startVortexObj->indices = m_quadIndices;
    startVortexObj->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester, m_startQuadTexture);
    textures.insert(std::make_pair(startVortexObj->texture, std::shared_ptr<TextureData>()));
    startVortexObj->modelMatrices.push_back(glm::translate(m_startQuadPosition) * scale);
    objs.push_back(std::make_pair(startVortexObj, std::shared_ptr<DrawObjectData>()));

    return true;
}

bool MovingQuadsLevel::updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesUpdated) {
    texturesUpdated = false;
    if (objs.empty()) {
        // the ball is first...
        std::shared_ptr<DrawObject> ballObj{new DrawObject{}};
        ballObj->vertices = m_ballVertices;
        ballObj->indices = m_ballIndices;
        ballObj->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester, m_ballTexture);
        textures.insert(std::make_pair(ballObj->texture, std::shared_ptr<TextureData>()));
        ballObj->modelMatrices.push_back(glm::translate(m_ball.position) *
                                         glm::toMat4(m_ball.totalRotated) * m_ball.scale);
        objs.push_back(std::make_pair(ballObj, std::shared_ptr<DrawObjectData>()));

        // next we have the moving quads
        size_t nbrRowsForTexture = m_movingQuads.size()/m_middleQuadTextures.size();
        size_t leftover = m_movingQuads.size() % m_middleQuadTextures.size();
        size_t i = 0;
        for (auto const &movingQuadRow : m_movingQuads) {
            std::shared_ptr<DrawObject> quadObj{new DrawObject{}};
            quadObj->vertices = m_quadVertices;
            quadObj->indices = m_quadIndices;
            size_t textureNumber = 0;
            if (nbrRowsForTexture == 0) {
                textureNumber = i;
            } else if (i <= leftover*nbrRowsForTexture) {
                textureNumber = i / (nbrRowsForTexture+1);
            } else {
                textureNumber = i / nbrRowsForTexture;
            }
            quadObj->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester,
                    m_middleQuadTextures[textureNumber]);
            textures.insert(std::make_pair(quadObj->texture, std::shared_ptr<TextureData>()));
            for (auto const &movingQuadPos : movingQuadRow.positions) {
                quadObj->modelMatrices.push_back(glm::translate(movingQuadPos) *
                                                 glm::scale(movingQuadRow.scale));
            }
            objs.push_back(std::make_pair(quadObj, std::shared_ptr<DrawObjectData>()));
            i++;
        }

        texturesUpdated = true;
    } else {
        // the ball is first...
        objs[0].first->modelMatrices[0] = glm::translate(m_ball.position) *
                glm::toMat4(m_ball.totalRotated) * m_ball.scale;

        // next, the moving quads
        for (int i = 1; i < objs.size(); i++) {
            for (int j = 0; j < objs[i].first->modelMatrices.size(); j++) {
                objs[i].first->modelMatrices[j] = glm::translate(m_movingQuads[i-1].positions[j]) *
                        glm::scale(m_movingQuads[i-1].scale);
            }
        }
    }
    return true;
}
