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
#include <glm/gtc/matrix_transform.hpp>

#include "avoidVortexLevel.hpp"

void AvoidVortexLevel::loadModels() {
    loadModel(m_gameRequester->getAssetStream(MODEL_BALL), ballVertices, ballIndices);
    getQuad(quadVertices, quadIndices);
}

constexpr float AvoidVortexLevel::viscosity;

void AvoidVortexLevel::preGenerate() {
    ball.position.z = m_maxZ-scaleFactor*m_originalBallDiameter/2;
    holePosition.z = m_maxZ-scaleFactor*m_originalBallDiameter;
    startPosition.z = ball.position.z;
}

void AvoidVortexLevel::postGenerate() {
    scale = glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor, scaleFactor, scaleFactor));

    ball.totalRotated = glm::quat();
    ball.acceleration = {0.0f, 0.0f, 0.0f};
    ball.velocity = {0.0f, 0.0f, 0.0f};

    // set to very large previous position (in vector length) so that it will get drawn
    // the first time through.
    ball.prevPosition = {-10.0f, 0.0f, m_maxZ-scaleFactor*m_originalBallDiameter/2};

    startPositionQuad = startPosition;
    startPositionQuad.z = m_maxZ-scaleFactor*m_originalBallDiameter;
}

void AvoidVortexLevel::generate() {
    /* ensure that the ball and hole are not near each other. They need to be farther apart than
     * the vortexes.  To make the maze fun and harder. */
    float smallestDistance = 4*scaleFactor;
    do {
        holePosition.x = random.getFloat(-maxX, maxX);
        holePosition.y = random.getFloat(-maxY, maxY);

        ball.position.x = random.getFloat(-maxX, maxX);
        ball.position.y = random.getFloat(-maxY, maxY);
    } while (glm::length(ball.position - holePosition) < smallestDistance*4.0f);
    startPosition = ball.position;

    /* ensure that the vortexes are not near each other or the hole or the ball. */
    do {
        glm::vec3 vortexPositionCandidate =
                {random.getFloat(-maxX, maxX), random.getFloat(-maxY, maxY),
                 m_maxZ-scaleFactor*m_originalBallDiameter};

        float distance = glm::length(vortexPositionCandidate - ball.position);
        if (distance < smallestDistance) {
            continue;
        }

        distance = glm::length(vortexPositionCandidate - holePosition);
        if (distance < smallestDistance) {
            continue;
        }

        for (auto const &vortexPosition : vortexPositions) {
            distance = glm::length(vortexPosition - vortexPositionCandidate);
            if (distance < smallestDistance) {
                break;
            }
        }

        if (distance >= smallestDistance) {
            vortexPositions.push_back(vortexPositionCandidate);
        }
    } while (vortexPositions.size() < numberOfVortexes);
}

bool AvoidVortexLevel::ballProximity(glm::vec3 const &objPosition) {
    float errDistance = scaleFactor*1.5f;
    if (glm::length(ball.position - objPosition) < errDistance) {
        ball.position.x = objPosition.x;
        ball.position.y = objPosition.y;
        ball.velocity = {0.0f, 0.0f, 0.0f};
        return true;
    }
    return false;
}

bool AvoidVortexLevel::updateData() {
    if (m_finished) {
        // the maze is finished, do nothing and return false (drawing is not necessary).
        return false;
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count();
    prevTime = currentTime;

    ball.velocity += ball.acceleration * time - viscosity * ball.velocity;
    ball.position += ball.velocity * time;

    if (ballProximity(holePosition)) {
        m_finished = true;
        return true;
    }

    for (auto &&vortexPosition : vortexPositions) {
        if (ballProximity(vortexPosition)) {
            ball.position = startPosition;
            return true;
        }
    }

    float minX = -maxX;
    float minY = -maxY;
    if (ball.position.x > maxX) {
        ball.position.x = maxX;
        if (ball.velocity.x > 0) {
            ball.velocity.x = -ball.velocity.x;
        }
    }

    if (ball.position.x < minX) {
        ball.position.x = minX;
        if (ball.velocity.x < 0) {
            ball.velocity.x = -ball.velocity.x;
        }
    }

    if (ball.position.y > maxY) {
        ball.position.y = maxY;
        if (ball.velocity.y > 0) {
            ball.velocity.y = -ball.velocity.y;
        }
    }

    if (ball.position.y < minY) {
        ball.position.y = minY;
        if (ball.velocity.y < 0) {
            ball.velocity.y = -ball.velocity.y;
        }
    }

    if (glm::length(ball.velocity) != 0) {
        glm::vec3 axis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), ball.velocity);
        float scaleFactor = 10.0f;
        glm::quat q = glm::angleAxis(glm::length(ball.velocity)*time*scaleFactor, glm::normalize(axis));

        ball.totalRotated = glm::normalize(q * ball.totalRotated);
    }
    modelMatrixBall = glm::translate(glm::mat4(1.0f), ball.position) * glm::mat4_cast(ball.totalRotated) * scale;

    bool drawingNecessary = glm::length(ball.position - ball.prevPosition) > 0.005;
    if (drawingNecessary) {
        ball.prevPosition = ball.position;
    }
    return drawingNecessary;
}

void AvoidVortexLevel::generateModelMatrices() {
    // the ball
    glm::mat4 trans = glm::translate(glm::mat4(1.0f), ball.position);
    modelMatrixBall = trans*scale;

    // the hole
    trans = glm::translate(glm::mat4(1.0f), holePosition);
    modelMatrixHole = trans*scale;

    // the starting vortex (the ball shows up here if it enters a vortex).
    trans = glm::translate(glm::mat4(1.0f), startPositionQuad);
    modelMatrixStartVortex = trans * scale;

    // the vortexes
    for (auto && vortexPosition : vortexPositions) {
        trans = glm::translate(glm::mat4(1.0f), vortexPosition);
        modelMatrixVortexes.push_back(trans*scale);
    }
}

bool AvoidVortexLevel::updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) {
    if (objs.size() > 0) {
        return false;
    }

    // the hole
    std::shared_ptr<DrawObject> holeObj(new DrawObject());
    holeObj->vertices = quadVertices;
    holeObj->indices = quadIndices;
    holeObj->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester, holeTexture);
    textures.insert(std::make_pair(holeObj->texture, std::shared_ptr<TextureData>()));
    holeObj->modelMatrices.push_back(modelMatrixHole);
    objs.push_back(std::make_pair(holeObj, std::shared_ptr<DrawObjectData>()));

    // the vortexes
    std::shared_ptr<DrawObject> vortexObj(new DrawObject());
    vortexObj->vertices = quadVertices;
    vortexObj->indices = quadIndices;
    vortexObj->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester, vortexTexture);
    textures.insert(std::make_pair(vortexObj->texture, std::shared_ptr<TextureData>()));
    for (auto &&modelMatrixVortex : modelMatrixVortexes) {
        vortexObj->modelMatrices.push_back(modelMatrixVortex);
    }
    objs.push_back(std::make_pair(vortexObj, std::shared_ptr<DrawObjectData>()));

    // the start position vortex
    std::shared_ptr<DrawObject> startVortexObj(new DrawObject());
    startVortexObj->vertices = quadVertices;
    startVortexObj->indices = quadIndices;
    startVortexObj->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester, startVortexTexture);
    textures.insert(std::make_pair(startVortexObj->texture, std::shared_ptr<TextureData>()));
    startVortexObj->modelMatrices.push_back(modelMatrixStartVortex);
    objs.push_back(std::make_pair(startVortexObj, std::shared_ptr<DrawObjectData>()));

    return true;
}

bool AvoidVortexLevel::updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesUpdated) {
    texturesUpdated = false;
    std::vector<glm::mat4> ballModelMatrices;
    ballModelMatrices.push_back(modelMatrixBall);
    if (objs.empty()) {
        objs.push_back(std::make_pair(std::shared_ptr<DrawObject>(new DrawObject()),
                                      std::shared_ptr<DrawObjectData>()));
        DrawObject *ballObj = objs[0].first.get();
        ballObj->vertices = ballVertices;
        ballObj->indices = ballIndices;
        ballObj->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester, ballTexture);
        textures.insert(std::make_pair(ballObj->texture, std::shared_ptr<TextureData>()));
        ballObj->modelMatrices.push_back(modelMatrixBall);
        texturesUpdated = true;
    } else {
        DrawObject *ballObj = objs[0].first.get();
        ballObj->modelMatrices[0] = modelMatrixBall;
    }
    return true;
}
