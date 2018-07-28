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
#include "openAreaLevel.hpp"

static std::string const MODEL_BALL("models/ball.obj");

void OpenAreaLevel::loadModels() {
    loadModel(MODEL_BALL, ballVertices, ballIndices);
    getQuad(holeVertices, holeIndices);
}

void OpenAreaLevel::updateAcceleration(float x, float y, float z) {
    ball.acceleration = glm::vec3(-x,-y,0.0f);
}

bool OpenAreaLevel::updateData() {
    if (finished) {
        // the maze is finished, do nothing and return false (drawing is not necessary).
        return false;
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count();
    prevTime = currentTime;

    float vx, vy;
    if (ball.velocity.x < 0.0f) {
        vx = viscosity;
    } else {
        vx = -viscosity;
    }
    if (ball.velocity.y < 0.0f) {
        vy = viscosity;
    } else {
        vy = -viscosity;
    }
    ball.velocity += ball.acceleration * time + glm::vec3(vx, vy, 0.0f);
    ball.position += ball.velocity * time;

    float errDistance = 0.05f;
    if (ball.position.x < holePosition.x + errDistance && ball.position.x > holePosition.x - errDistance &&
        ball.position.y < holePosition.y + errDistance && ball.position.y > holePosition.y - errDistance) {
        finished = true;
        ball.position.x = holePosition.x;
        ball.position.y = holePosition.y;
        ball.velocity = {0.0f, 0.0f, 0.0f};
        return true;
    }

    float maxX = 1.0f;
    float minX = -1.0f;
    float maxY = 1.0f;
    float minY = -1.0f;
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


    glm::vec3 axis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), ball.velocity);
    if (glm::length(axis) != 0) {
        float scaleFactor = 10.0f;
        glm::quat q = glm::angleAxis(glm::length(ball.velocity)*time*scaleFactor, glm::normalize(axis));

        ball.totalRotated = glm::normalize(q * ball.totalRotated);
    }
    modelMatrixBall = glm::translate(ball.position) * glm::toMat4(ball.totalRotated) * scale;

    bool drawingNecessary = glm::length(ball.position - ball.prevPosition) > 0.00005;
    if (drawingNecessary) {
        ball.prevPosition = ball.position;
    }
    return drawingNecessary;
}

void OpenAreaLevel::generateModelMatrices() {
    // the ball
    glm::mat4 trans = glm::translate(ball.position);
    modelMatrixBall = trans*scale;

    // the hole
    trans = glm::translate(holePosition);
    modelMatrixHole = trans*scale;
}

bool OpenAreaLevel::updateStaticDrawObjects(DrawObjectTable &objs) {
    if (objs.size() > 0) {
        return false;
    }
    std::shared_ptr<DrawObject> holeObj(new DrawObject());

    std::vector<glm::mat4> holeModelMatrices;
    holeModelMatrices.push_back(modelMatrixHole);

    *holeObj = {holeVertices, holeIndices, holeTexture, holeModelMatrices};
    objs.push_back(std::make_pair(holeObj, std::shared_ptr<DrawObjectData>()));

    return true;
}

bool OpenAreaLevel::updateDynamicDrawObjects(DrawObjectTable &objs) {
    std::vector<glm::mat4> ballModelMatrices;
    ballModelMatrices.push_back(modelMatrixBall);
    if (objs.size() == 0) {
        objs.push_back(std::make_pair(std::shared_ptr<DrawObject>(new DrawObject()),
                                      std::shared_ptr<DrawObjectData>()));
        DrawObject *ballObj = objs[0].first.get();
        *ballObj = {ballVertices, ballIndices, ballTexture, ballModelMatrices};
    } else {
        DrawObject *ballObj = objs[0].first.get();
        ballObj->modelMatrices = ballModelMatrices;
    }
    return true;
}
