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
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "levelStarter.hpp"

void LevelStarter::clearText() {
    text.clear();
}

void LevelStarter::addTextString(std::string const &inText) {
    text.push_back(inText);
}

bool LevelStarter::updateData() {
    if (m_finished) {
        return false;
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    float difftime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count();
    prevTime = currentTime;

    float viscosity = 0.01f;
    ball.velocity += ball.acceleration * difftime - viscosity * ball.velocity;

    if (isInBottomCorridor()) {
        ball.position.y = -maxPosY;
        ball.velocity.y = 0.0f;

        ball.position.x += ball.velocity.x * difftime;
    } else if (isInSideCorridor()) {
        ball.position.x = maxPosX;
        ball.velocity.x = 0.0f;

        ball.position.y += ball.velocity.y * difftime;
    } else { // is in top corridor
        ball.position.y = maxPosY;
        ball.velocity.y = 0.0f;

        ball.position.x += ball.velocity.x * difftime;
    }
    confineBall();

    if (ball.position.x < -maxPosX + errVal && ball.position.y == maxPosY) {
        transitionText = true;
        ball.position.x = -maxPosX;
        ball.position.y = -maxPosY;
        if (textIndex == text.size() - 1) {
            m_finished = true;
            return true;
        }
    }

    glm::vec3 axis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), ball.velocity);
    if (glm::length(axis) != 0) {
        float scaleFactor = 10.0f;
        glm::quat q = glm::angleAxis(glm::length(ball.velocity)*difftime*scaleFactor, glm::normalize(axis));

        ball.totalRotated = glm::normalize(q * ball.totalRotated);
    }

    if (glm::length(ball.position - ball.prevPosition) < 0.005f) {
        return false;
    }

    ball.prevPosition = ball.position;

    return true;
}

void LevelStarter::confineBall() {
    if (ball.position.y < -maxPosY) {
        ball.velocity.y = 0.0f;
        ball.position.y = -maxPosY;
    }
    if (ball.position.y > maxPosY) {
        ball.velocity.y = 0.0f;
        ball.position.y = maxPosY;
    }

    if (ball.position.x < -maxPosX) {
        ball.velocity.x = 0.0f;
        ball.position.x = -maxPosX;
    }
    if (ball.position.x > maxPosX) {
        ball.velocity.x = 0.0f;
        ball.position.x = maxPosX;
    }
}

bool LevelStarter::isInBottomCorridor() {
    if (ball.position.y < -maxPosY + errVal) {
        if (ball.position.x > maxPosX - errVal) {
            return -ball.velocity.x >= ball.velocity.y;
        }
        return true;
    }
    return false;
}

bool LevelStarter::isInSideCorridor() {
    if (ball.position.x > maxPosX - errVal) {
        if (ball.position.y < -maxPosY + errVal) {
            return ball.velocity.y >= -ball.velocity.x;
        }
        if (ball.position.y > maxPosY - errVal) {
            return -ball.velocity.y >= -ball.velocity.x;
        }
        return true;
    }
    return false;
}

bool LevelStarter::updateDynamicDrawObjects(DrawObjectTable &drawObjsData, TextureMap &textures, bool &texturesChanged) {
    texturesChanged = false;

    if (m_finished) {
        return false;
    }

    if (drawObjsData.empty()) {
        // ball
        std::shared_ptr<DrawObject> obj = std::make_shared<DrawObject>();
        obj->vertices = ballVertices;
        obj->indices = ballIndices;
        obj->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester, ballImage);
        textures.insert(std::make_pair(obj->texture, std::shared_ptr<TextureData>()));
        obj->modelMatrices.push_back(
                glm::translate(ball.position) * glm::toMat4(ball.totalRotated) * glm::scale(ballScale));
        drawObjsData.push_back(std::make_pair(obj, std::shared_ptr<DrawObjectData>()));

        // Text box
        std::shared_ptr<DrawObject> obj1 = std::make_shared<DrawObject>();
        obj1->vertices = quadVertices;
        obj1->indices = quadIndices;
        obj1->texture = std::make_shared<TextureDescriptionText>(m_gameRequester, text[textIndex]);
        textures.insert(std::make_pair(obj1->texture, std::shared_ptr<TextureData>()));
        obj1->modelMatrices.push_back(
                glm::translate(glm::vec3(0.0f, 0.0f, m_maxZ - ballScale.z * m_originalBallDiameter)) *
                glm::scale(textScale));
        drawObjsData.push_back(std::make_pair(obj1, std::shared_ptr<DrawObjectData>()));
        texturesChanged = true;
    } else {
        drawObjsData[0].first->modelMatrices[0] = glm::translate(ball.position) *
                glm::toMat4(ball.totalRotated) * glm::scale(ballScale);

        if (transitionText && !m_finished) {
            textIndex++;
            textures.erase(drawObjsData[1].first->texture);
            drawObjsData[1].first->texture = std::make_shared<TextureDescriptionText>(m_gameRequester,
                    text[textIndex]);
            textures.insert(std::make_pair(drawObjsData[1].first->texture, std::shared_ptr<TextureData>()));
            transitionText = false;
            texturesChanged = true;
        }
    }

    return true;
}

bool LevelStarter::updateStaticDrawObjects(DrawObjectTable &drawObjsData, TextureMap &textures) {
    // Static Draw Objects are in the order: hole, then corridors (3 of them)
    if (!drawObjsData.empty()) {
        // these are static draw objects if they are already initialized just return false (no
        // need to update their draw data).
        return false;
    }

    // the hole
    std::shared_ptr<DrawObject> obj = std::make_shared<DrawObject>();
    obj->vertices = quadVertices;
    obj->indices = quadIndices;
    obj->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester, holeImage);
    textures.insert(std::make_pair(obj->texture, std::shared_ptr<TextureData>()));
    obj->modelMatrices.push_back(
            glm::translate(glm::vec3(-maxPosX, maxPosY, m_maxZ - ballScale.z*m_originalBallDiameter)) *
            glm::scale(holeScale));
    drawObjsData.push_back(std::make_pair(obj, std::shared_ptr<DrawObjectData>()));

    // Bottom corridor
    obj = std::make_shared<DrawObject>();
    std::shared_ptr<TextureDescription> textureCorridorH1 = std::make_shared<TextureDescriptionPath>(
            m_gameRequester, corridorImageH1);
    textures.insert(std::make_pair(textureCorridorH1, std::shared_ptr<TextureData>()));
    obj->vertices = quadVertices;
    obj->indices = quadIndices;
    obj->texture = textureCorridorH1;
    obj->modelMatrices.push_back(
            glm::translate(glm::vec3(0.0f, -maxPosY, m_maxZ - ballScale.z*m_originalBallDiameter)) *
            glm::scale(corridorHScale));
    drawObjsData.push_back(std::make_pair(obj, std::shared_ptr<DrawObjectData>()));

    // Side corridor
    obj = std::make_shared<DrawObject>();
    std::shared_ptr<TextureDescription> textureCorridorV = std::make_shared<TextureDescriptionPath>(
            m_gameRequester, corridorImageV);
    textures.insert(std::make_pair(textureCorridorV, std::shared_ptr<TextureData>()));
    obj->vertices = quadVertices;
    obj->indices = quadIndices;
    obj->texture = textureCorridorV;
    obj->modelMatrices.push_back(
            glm::translate(glm::vec3(maxPosX, 0.0f, m_maxZ - ballScale.z*m_originalBallDiameter)) *
            glm::scale(corridorVScale));
    drawObjsData.push_back(std::make_pair(obj, std::shared_ptr<DrawObjectData>()));

    // Top corridor
    obj = std::make_shared<DrawObject>();
    std::shared_ptr<TextureDescription> textureCorridorH2 = std::make_shared<TextureDescriptionPath>(
            m_gameRequester, corridorImageH2);
    textures.insert(std::make_pair(textureCorridorH2, std::shared_ptr<TextureData>()));
    obj->vertices = quadVertices;
    obj->indices = quadIndices;
    obj->texture = textureCorridorH2;
    obj->modelMatrices.push_back(
            glm::translate(glm::vec3(0.0f, maxPosY, m_maxZ - ballScale.z*m_originalBallDiameter)) *
            glm::scale(corridorHScale));
    drawObjsData.push_back(std::make_pair(obj, std::shared_ptr<DrawObjectData>()));

    return true;
}