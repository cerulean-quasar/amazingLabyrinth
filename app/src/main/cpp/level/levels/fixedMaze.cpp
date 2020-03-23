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
    m_ball.acceleration = glm::vec3{x, y, 0.0f/* -z  */};
}

bool FixedMaze::updateData() {
    // need to postpone getting the depth texture till here because the functionality is
    // not available earlier on.
    // todo: fix this!
    if (!m_initialized) {
        init();
        m_initialized = true;
    }

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

    size_t xcell = static_cast<size_t>(std::floor((m_ball.position.x + m_width/2)/m_width * m_rowWidth));
    size_t ycell = static_cast<size_t>(std::floor((m_ball.position.y + m_height/2)/m_height * m_rowHeight));
    m_ball.position.z = m_depthMap[ycell * m_rowHeight + xcell] + m_scaleBall/2.0f;

    bool drawingNecessary = glm::length(m_ball.position - m_ball.prevPosition) > 0.005;
    if (drawingNecessary) {
        m_ball.prevPosition = m_ball.position;
    }
    return drawingNecessary;
}

bool FixedMaze::updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) {
    return true;
}

bool FixedMaze::updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged) {
    texturesChanged = false;
    if (objs.empty() && textures.empty() && m_initialized) {
        /* floor */
        texturesChanged = true;
        objs.emplace_back(m_testObj, nullptr);
        //auto obj = m_worldMap.begin();
        //obj->first->texture = m_testObj->texture;
        //objs.emplace_back(obj->first, nullptr);
        textures.insert(
                std::make_pair(std::make_shared<TextureDescriptionDummy>(m_gameRequester), m_testTexture));

        /* ball */
        objs.push_back(std::make_pair(std::shared_ptr<DrawObject>(new DrawObject()),
                                      std::shared_ptr<DrawObjectData>()));
        DrawObject *ballObj = objs[0].first.get();
        ballObj->vertices = m_ballVertices;
        ballObj->indices = m_ballIndices;
        ballObj->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester, m_ballTextureName);
        textures.insert(std::make_pair(ballObj->texture, std::shared_ptr<TextureData>()));
        glm::mat4 modelMatrixBall = glm::translate(glm::mat4(1.0f), m_ball.position) *
                                    glm::mat4_cast(m_ball.totalRotated) *
                                    glm::scale(glm::mat4(1.0f), m_scaleBall, m_scaleBall, m_scaleBall);
        ballObj->modelMatrices.push_back(modelMatrixBall);
        texturesChanged = true;
    } else {
        glm::mat4 modelMatrixBall = glm::translate(glm::mat4(1.0f), m_ball.position) *
            glm::mat4_cast(m_ball.totalRotated) *
            glm::scale(glm::mat4(1.0f), m_scaleBall, m_scaleBall, m_scaleBall);

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
    m_initialized{false},
    m_scaleBall{width/10.0f/2.0f}
{
    loadModels();
}

FixedMaze::FixedMaze(std::shared_ptr<GameRequester> inGameRequester,
    std::shared_ptr<FixedMazeSaveData> sd,
    float width, float height, float maxZ)
    : Level{inGameRequester, width, height, maxZ},
    m_initialized{false},
    m_scaleBall{width/10.0f/2.0f}
{
    loadModels();
}

void FixedMaze::init()
{
    auto worldObj = std::make_shared<DrawObject>();
    loadModel(m_gameRequester->getAssetStream("models/mountainLandscape.obj"), worldObj->vertices, worldObj->indices);
    worldObj->modelMatrices.push_back(
            glm::translate(glm::mat4(1.0f), glm::vec3{0.0f, 0.0f, m_maxZ}) *
            glm::scale(glm::mat4(1.0f), glm::vec3{m_width/MODEL_WIDTH, m_height/MODEL_HEIGHT, 1.0f}) *
            glm::mat4_cast(glm::angleAxis(3.1415926f/2.0f, glm::vec3(1.0f, 0.0f, 0.0f))));
    worldObj->texture = nullptr;
    m_worldMap.emplace_back(worldObj, nullptr);

    m_rowWidth = static_cast<uint32_t>(std::floor(10.0f/m_scaleBall * m_width));
    m_testTexture = m_gameRequester->getDepthTexture(m_worldMap, m_width, m_height, m_rowWidth,
            m_depthMap);
    m_rowHeight = m_depthMap.size()/m_rowWidth;

    m_testObj = std::make_shared<DrawObject>();
    m_testObj->texture = std::make_shared<TextureDescriptionDummy>(m_gameRequester);
    getQuad(m_testObj->vertices, m_testObj->indices);
    m_testObj->modelMatrices.push_back(glm::translate(glm::mat4(1.0f), glm::vec3{0.0f, 0.0f, m_maxZ}) *
                                       glm::scale(glm::mat4(1.0f), glm::vec3{m_width/2.0f, m_height/2.0f, 1.0f}));
}
