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

#include <list>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/transform.hpp>
#include "levelFinish.hpp"

ManyQuadCoverUpLevelFinish::ManyQuadCoverUpLevelFinish() : totalNumberReturned(0) {
    prevTime = std::chrono::high_resolution_clock::now();
    imagePaths =
            {"textures/flower1.png",
             "textures/flower2.png",
             "textures/flower3.png",
             "textures/flower4.png"};
    float range = 1.5f;
    for (uint32_t i = 0; i < totalNumberObjectsForSide; i++) {
        for (uint32_t j = 0; j < totalNumberObjectsForSide; j++) {
            if (translateVectors.size() > 0) {
                uint32_t index = random.getUInt(0, translateVectors.size());
                std::list<glm::vec3>::iterator it = translateVectors.begin();
                for (int k = 0; k < index; k++) {
                    it++;
                }
                translateVectors.insert(it, glm::vec3(2.0f / (totalNumberObjectsForSide - 1) * i - 1.0f,
                                          range / (totalNumberObjectsForSide - 1) * j - range/2,
                                          0.0f));
            } else {
                translateVectors.push_back(glm::vec3(2.0f / (totalNumberObjectsForSide - 1) * i - 1.0f,
                                         range / (totalNumberObjectsForSide - 1) * j - range/2,
                                         0.0f));
            }
        }
    }
}

bool ManyQuadCoverUpLevelFinish::updateDrawObjects(DrawObjectTable &drawObjects, TextureMap &textures, bool &texturesUpdated) {
    texturesUpdated = false;
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
            currentTime - prevTime).count();

    if (time < timeThreshold) {
        return false;
    }
    prevTime = currentTime;

    if (shouldUnveil) {
        if (totalNumberReturned == 0) {
            finished = true;
            return false;
        }

        uint32_t i = random.getUInt(0, drawObjects.size() - 1);
        drawObjects[i].first->modelMatrices.pop_back();
        if (drawObjects[i].first->modelMatrices.size() == 0) {
            drawObjects.erase(drawObjects.begin()+i);
        }
        totalNumberReturned --;
    } else {
        if (totalNumberReturned >= totalNumberObjects) {
            finished = true;
            return false;
        }

        float sideLength = random.getFloat(0.2f, 0.4f);
        glm::mat4 scale = glm::scale(glm::vec3(sideLength, sideLength, 1.0f));
        glm::vec3 translateVector = translateVectors.back();
        translateVectors.pop_back();
        translateVector.z = 0.1f + totalNumberReturned * 0.01f;
        glm::mat4 trans = glm::translate(translateVector);

        // the first imagePaths.size() objects are the linear order of objects in imagePaths,
        // then a texture image is selected at random.
        size_t index;
        if (totalNumberReturned < imagePaths.size()) {
            index = drawObjects.size();
            std::shared_ptr<DrawObject> obj(new DrawObject());
            obj->modelMatrices.push_back(trans * scale);
            obj->texture.reset(new TextureDescriptionPath(imagePaths[index]));
            textures.insert(std::make_pair(obj->texture, std::shared_ptr<TextureData>()));
            getQuad(obj->vertices, obj->indices);
            obj->modelMatrices.push_back(trans * scale);

            drawObjects.push_back(std::make_pair(obj, std::shared_ptr<DrawObjectData>()));
            texturesUpdated = true;
        } else {
            index = random.getUInt(0, imagePaths.size() - 1);
            drawObjects[index].first->modelMatrices.push_back(trans * scale);
        }

        totalNumberReturned++;
    }

    return true;
}

GrowingQuadLevelFinish::GrowingQuadLevelFinish(float x, float y) {
    transVector = {x, y, transZ};
    prevTime = std::chrono::high_resolution_clock::now();
    timeSoFar = 0.0f;
    imagePath = "textures/starField.png";
}

bool GrowingQuadLevelFinish::updateDrawObjects(DrawObjectTable &drawObjects, TextureMap &textures, bool &texturesUpdated) {
    texturesUpdated = false;
    if (finished) {
        return false;
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count();

    if (time < timeThreshold) {
        return false;
    }

    prevTime = currentTime;

    int multiplier = 1;
    float size = minSize;
    if (shouldUnveil) {
        size = finalSize;
        multiplier = -1;
    }

    if (drawObjects.size() == 0) {
        timeSoFar = 0;
        scaleVector = {minSize, minSize, minSize};
        std::shared_ptr<DrawObject> obj(new DrawObject());

        getQuad(obj->vertices, obj->indices);
        obj->texture.reset(new TextureDescriptionPath(imagePath));
        textures.insert(std::make_pair(obj->texture, std::shared_ptr<TextureData>()));
        obj->modelMatrices.push_back(glm::translate(transVector) * glm::scale(scaleVector));
        drawObjects.push_back(std::make_pair(obj, std::shared_ptr<DrawObjectData>()));
        texturesUpdated = true;
    } else {
        timeSoFar += time;
        size = size + multiplier * timeSoFar / totalTime * (finalSize - minSize);
        scaleVector = {size, size, size};
        drawObjects[0].first->modelMatrices[0] = glm::translate(transVector) * glm::scale(scaleVector);
        if (size >= finalSize && !shouldUnveil) {
            timeSoFar = 0.0f;
            finished = true;
        }
        if (size <= minSize && shouldUnveil) {
            finished = true;
        }
    }

    return true;
}
