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

#include "fixedMaze.hpp"

glm::vec4 FixedMaze::getBackgroundColor() {
    return glm::vec4{0.0f, 0.0f, 0.0f, 0.0f};
}

void FixedMaze::updateAcceleration(float x, float y, float z) {
    m_ball.acceleration = glm::vec3{-x, -y, 0.0f/* -z  */};
}

void FixedMaze::setBallZPos() {
    m_ball.position.z = getZPos(m_ball.position.x, m_ball.position.y) + m_scaleBall/2.0f;
    if (m_ball.position.z > 0.0f) {
        m_ball.position.z = m_maxZ - m_scaleBall;
    }
}

size_t FixedMaze::getXCell(float x) {
    return static_cast<size_t>(std::floor((x + m_width/2)/m_width * m_rowWidth));
}

size_t FixedMaze::getYCell(float y) {
    return static_cast<size_t>(std::floor((y + m_height/2)/m_height * m_rowHeight));
}

void boundsCheck(size_t &cell, size_t size) {
    if (cell < 0) {
        cell = 0;
    }

    if (cell > size - 1) {
        cell = size - 1;
    }
}

float FixedMaze::getZPos(float x, float y) {
    float maxZ = m_maxZ - MODEL_MAXZ;
    size_t xcellmin = getXCell(x - m_scaleBall * MODEL_BALL_SIZE/2.0f);
    size_t xcellmax = getXCell(x + m_scaleBall * MODEL_BALL_SIZE/2.0f);
    size_t ycellmin = getYCell(y - m_scaleBall * MODEL_BALL_SIZE/2.0f);
    size_t ycellmax = getYCell(y + m_scaleBall * MODEL_BALL_SIZE/2.0f);

    boundsCheck(xcellmin, m_rowWidth);
    boundsCheck(xcellmax, m_rowWidth);
    boundsCheck(ycellmin, m_rowHeight);
    boundsCheck(ycellmax, m_rowHeight);

    for (size_t i = xcellmin; i <= xcellmax; i++) {
        for (size_t j = ycellmin; j <= ycellmax; j++) {
            float z = m_depthMap[j * m_rowWidth + i];
            if (z > m_maxZ + MODEL_MAXZ || z < m_maxZ - MODEL_MAXZ) {
                // if we are outside of the range the model is supposed to be in for Z, then
                // assume the ball is in the "hole".  Return the special value: m_maxZ - MODEL_MAXZ
                // to indicate this.
                return m_maxZ - MODEL_MAXZ;
            } else  if (z > maxZ) {
                maxZ = z;
            }
        }
    }

    return maxZ;
}

bool FixedMaze::updateData() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - m_prevTime).count();
    m_prevTime = currentTime;

    m_ball.velocity += m_ball.acceleration * time - m_viscosity * m_ball.velocity;
    m_ball.position += m_ball.velocity * time;

    float errDistance = m_scaleBall;

    float maxX = m_width/2 - m_scaleBall/2;
    float minX = -m_width/2 + m_scaleBall/2;
    float maxY = m_height/2 - m_scaleBall/2;
    float minY = -m_height/2 + m_scaleBall/2;
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


    glm::vec3 axis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), m_ball.velocity);
    if (glm::length(axis) != 0) {
        float scaleFactor = 10.0f;
        glm::quat q = glm::angleAxis(glm::length(m_ball.velocity)*time*scaleFactor, glm::normalize(axis));

        m_ball.totalRotated = glm::normalize(q * m_ball.totalRotated);
    }

    setBallZPos();

    bool drawingNecessary = glm::length(m_ball.position - m_ball.prevPosition) > 0.005;
    if (drawingNecessary) {
        m_ball.prevPosition = m_ball.position;
    }
    return drawingNecessary;
}

bool FixedMaze::updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) {
    if (objs.empty() && textures.empty()) {
        /* floor */
        //objs.emplace_back(m_testObj, nullptr);
        auto obj = m_worldMap.begin();
        obj->first->texture = m_testObj->texture;
        objs.emplace_back(obj->first, nullptr);

        textures.insert(
                std::make_pair(std::make_shared<TextureDescriptionDummy>(m_gameRequester),
                               m_testTexture));
        return true;
    }

    return false;
}

bool FixedMaze::updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged) {
    texturesChanged = false;
    if (objs.empty()) {
        /* ball */
        std::shared_ptr<DrawObject> ballObj = std::make_shared<DrawObject>();
        ballObj->vertices = m_ballVertices;
        ballObj->indices = m_ballIndices;
        ballObj->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester,
                                                                    m_ballTextureName);
        textures.insert(std::make_pair(ballObj->texture, std::shared_ptr<TextureData>()));
        glm::mat4 modelMatrixBall = glm::translate(glm::mat4(1.0f), m_ball.position) *
                                    glm::mat4_cast(m_ball.totalRotated) *
                                    glm::scale(glm::mat4(1.0f),
                                               glm::vec3{m_scaleBall, m_scaleBall,
                                                         m_scaleBall});
        ballObj->modelMatrices.push_back(modelMatrixBall);
        objs.emplace_back(ballObj, nullptr);
        texturesChanged = true;
    } else {
        glm::mat4 modelMatrixBall = glm::translate(glm::mat4(1.0f), m_ball.position) *
                                    glm::mat4_cast(m_ball.totalRotated) *
                                    glm::scale(glm::mat4(1.0f),
                                               glm::vec3{m_scaleBall, m_scaleBall,
                                                         m_scaleBall});
        std::shared_ptr<DrawObject> ballObj = objs[0].first;
        ballObj->modelMatrices[0] = modelMatrixBall;

    }
    return true;
}

void FixedMaze::start() {
    m_prevTime = std::chrono::high_resolution_clock::now();
}

void FixedMaze::getLevelFinisherCenter(float &x, float &y) {
    x = 0.0f;
    y = 0.0f;
}

void FixedMaze::loadModels() {
    loadModel(m_gameRequester->getAssetStream(MODEL_BALL), m_ballVertices, m_ballIndices);
}

FixedMaze::FixedMaze(std::shared_ptr<GameRequester> inGameRequester, float width, float height, float maxZ)
    : Level{inGameRequester, width, height, maxZ},
    m_scaleBall{width/10.0f/2.0f}
{
    init();
}

FixedMaze::FixedMaze(std::shared_ptr<GameRequester> inGameRequester,
    std::shared_ptr<FixedMazeSaveData> sd,
    float width, float height, float maxZ)
    : Level{inGameRequester, width, height, maxZ},
    m_scaleBall{width/10.0f/2.0f}
{
    init();
}

void FixedMaze::init()
{
    loadModels();

    auto worldObj = std::make_shared<DrawObject>();
    loadModel(m_gameRequester->getAssetStream("models/mountainLandscape.obj"), worldObj->vertices, worldObj->indices);
    worldObj->modelMatrices.push_back(
            glm::translate(glm::mat4(1.0f), glm::vec3{0.0f, 0.0f, m_maxZ}) *
            glm::scale(glm::mat4(1.0f), glm::vec3{m_width/MODEL_WIDTH, m_height/MODEL_HEIGHT, 1.0f}) *
            glm::mat4_cast(glm::angleAxis(3.1415926f/2.0f, glm::vec3(1.0f, 0.0f, 0.0f))));
    worldObj->texture = nullptr;
    m_worldMap.emplace_back(worldObj, nullptr);

    m_rowWidth = static_cast<uint32_t>(std::floor(20.0f/m_scaleBall * m_width));
    m_testTexture = m_gameRequester->getDepthTexture(m_worldMap, m_width, m_height, m_rowWidth,
            m_depthMap);
    m_rowHeight = m_depthMap.size()/m_rowWidth;

    m_testObj = std::make_shared<DrawObject>();
    m_testObj->texture = std::make_shared<TextureDescriptionDummy>(m_gameRequester);
    getQuad(m_testObj->vertices, m_testObj->indices);
    m_testObj->modelMatrices.push_back(glm::translate(glm::mat4(1.0f), glm::vec3{0.0f, 0.0f, m_maxZ}) *
                                       glm::scale(glm::mat4(1.0f), glm::vec3{m_width/2.0f, m_height/2.0f, 1.0f}));

    m_ball.position = {0.0f, 0.0f, 0.0f};
    setBallZPos();
}
