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
#include "openAreaLevel.hpp"
#include "../level.hpp"

void OpenAreaLevel::loadModels() {
    loadModel(m_gameRequester->getAssetStream(MODEL_BALL), ballVertices, ballIndices);
    getQuad(holeVertices, holeIndices);
}

bool OpenAreaLevel::updateData() {
    if (m_finished) {
        // the maze is finished, do nothing and return false (drawing is not necessary).
        return false;
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    float timeDiff = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count();
    prevTime = currentTime;

    m_ball.velocity = getUpdatedVelocity(m_ball.acceleration, timeDiff);
    m_ball.position += m_ball.velocity * timeDiff;

    float errDistance = ballDiameter();
    if (glm::length(m_ball.position - holePosition) < errDistance) {
        m_finished = true;
        m_ball.position.x = holePosition.x;
        m_ball.position.y = holePosition.y;
        m_ball.velocity = {0.0f, 0.0f, 0.0f};
        return true;
    }

    checkBallBorders(m_ball.position, m_ball.velocity);
    updateRotation(timeDiff);
    modelMatrixBall = glm::translate(glm::mat4(1.0f), m_ball.position) * glm::mat4_cast(m_ball.totalRotated) * scale;

    return drawingNecessary();
}

void OpenAreaLevel::generateModelMatrices() {
    // the ball
    glm::mat4 trans = glm::translate(glm::mat4(1.0f), m_ball.position);
    modelMatrixBall = trans*scale;

    // the hole
    trans = glm::translate(glm::mat4(1.0f), holePosition);
    modelMatrixHole = trans*scale;
}

bool OpenAreaLevel::updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) {
    if (!objs.empty()) {
        return false;
    }
    std::shared_ptr<DrawObject> holeObj(new DrawObject());

    holeObj->vertices = holeVertices;
    holeObj->indices = holeIndices;
    holeObj->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester, holeTexture);
    textures.insert(std::make_pair(holeObj->texture, std::shared_ptr<TextureData>()));
    holeObj->modelMatrices.push_back(modelMatrixHole);
    objs.push_back(std::make_pair(holeObj, std::shared_ptr<DrawObjectData>()));

    return true;
}

bool OpenAreaLevel::updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesUpdated) {
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
