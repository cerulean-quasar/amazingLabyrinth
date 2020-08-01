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

#include <list>
#include <glm/gtc/matrix_transform.hpp>
#include "types.hpp"
#include "../../common.hpp"

namespace manyQuadsCoverUp {
    char constexpr const *LevelFinisher::m_name;

    LevelFinisher::LevelFinisher(
            levelDrawer::Adaptor inLevelDrawer, std::shared_ptr<LevelConfigData> const &lcd,
            float centerX, float centerY, float centerZ, float maxZ)
            : finisher::LevelFinisher(std::move(inLevelDrawer), centerX, centerY, centerZ, maxZ),
              totalNumberReturned(0),
              imagePaths{lcd->textures}
    {
        prevTime = std::chrono::high_resolution_clock::now();
        float range = m_height;
        for (uint32_t i = 0; i < totalNumberObjectsForSide; i++) {
            for (uint32_t j = 0; j < totalNumberObjectsForSide; j++) {
                if (!translateVectors.empty()) {
                    uint32_t index = random.getUInt(0, translateVectors.size());
                    auto it = translateVectors.begin();
                    for (uint32_t k = 0; k < index; k++) {
                        it++;
                    }
                    translateVectors.insert(it, glm::vec3(
                            m_width / (totalNumberObjectsForSide - 1) * i - m_width / 2.0f,
                            range / (totalNumberObjectsForSide - 1) * j - range / 2,
                            0.0f));
                } else {
                    translateVectors.push_back(glm::vec3(
                            m_width / (totalNumberObjectsForSide - 1) * i - m_width / 2.0f,
                            range / (totalNumberObjectsForSide - 1) * j - range / 2,
                            0.0f));
                }
            }
        }
    }

    void LevelFinisher::start() {
        prevTime = std::chrono::high_resolution_clock::now();
    }

    bool LevelFinisher::updateDrawObjects() {
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

            uint32_t i = random.getUInt(0, m_objRefs.size() - 1);
            size_t nbrObjsData = m_levelDrawer.numberObjectsDataForObject(m_objRefs[i]);
            m_levelDrawer.removeObjectData(m_objRefs[i], m_objDataRefs[i][nbrObjsData-1]);
            if (--nbrObjsData == 0) {
                m_levelDrawer.removeObject(m_objRefs[i]);
                m_objRefs.erase(m_objRefs.begin() + i);
                m_objDataRefs.erase(m_objDataRefs.begin() + i);
            } else {
                m_objDataRefs[i].pop_back();
            }

            totalNumberReturned --;
        } else {
            if (totalNumberReturned >= totalNumberObjects) {
                finished = true;
                return false;
            }

            float sideLength = random.getFloat(0.1f*m_height, 0.3f*m_height);
            glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(sideLength, sideLength, 1.0f));
            glm::vec3 translateVector = translateVectors.back();
            translateVectors.pop_back();
            translateVector.z = m_maxZ +
                    (static_cast<int32_t>(totalNumberReturned) -
                            static_cast<int32_t>(totalNumberObjects)) * 0.01f;
            glm::mat4 trans = glm::translate(glm::mat4(1.0f), translateVector);

            // the first imagePaths.size() objects are the linear order of objects in imagePaths,
            // then a texture image is selected at random.
            if (totalNumberReturned < imagePaths.size()) {
                auto model = std::make_shared<levelDrawer::ModelDescriptionQuad>();
                auto texture = std::make_shared<levelDrawer::TextureDescriptionPath>(imagePaths[totalNumberReturned]);
                auto objRef = m_levelDrawer.addObject(model, texture);
                m_objRefs.push_back(objRef);
                auto objDataRef = m_levelDrawer.addModelMatrixForObject(objRef, trans * scale);
                m_objDataRefs.push_back(std::vector<levelDrawer::DrawObjDataReference>{objDataRef});
            } else {
                size_t index;
                index = random.getUInt(0, imagePaths.size() - 1);
                auto objDataRef = m_levelDrawer.addModelMatrixForObject(m_objRefs[index], trans * scale);
                m_objDataRefs[index].push_back(objDataRef);
            }

            totalNumberReturned++;
        }

        return true;
    }
} // namespace manyQuadsCoverUp

namespace growingQuad {
    char constexpr const *LevelFinisher::m_name;

    LevelFinisher::LevelFinisher(
            levelDrawer::Adaptor inLevelDrawer, std::shared_ptr<LevelConfigData> const &lcd,
            float centerX, float centerY, float centerZ, float maxZ)
            : finisher::LevelFinisher(std::move(inLevelDrawer), centerX, centerY, centerZ, maxZ),
              finalSize{1.5f * std::max(m_width, m_height)},
              minSize{0.005f * std::min(m_width, m_height)},
              imagePath{lcd->texture},
              m_objRef{0},
              m_objDataRef{0}
    {
        transVector = {m_centerX, m_centerY, maxZ};
        prevTime = std::chrono::high_resolution_clock::now();
        timeSoFar = 0.0f;
        scaleVector = {minSize, minSize, minSize};
        m_objRef = m_levelDrawer.addObject(
                std::make_shared<levelDrawer::ModelDescriptionQuad>(),
                std::make_shared<levelDrawer::TextureDescriptionPath>(imagePath));
        m_objDataRef = m_levelDrawer.addModelMatrixForObject(
                m_objRef,
                glm::translate(glm::mat4(1.0f), transVector) *
                glm::scale(glm::mat4(1.0f), scaleVector));
    }

    void LevelFinisher::start() {
        prevTime = std::chrono::high_resolution_clock::now();
        timeSoFar = 0.0f;
    }

    bool
    LevelFinisher::updateDrawObjects()
    {
        if (finished) {
            return false;
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(
                currentTime - prevTime).count();

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

            timeSoFar += time;
            size = size + multiplier * timeSoFar / totalTime * (finalSize - minSize);
            scaleVector = {size, size, size};
            m_levelDrawer.updateModelMatrixForObject(
                    m_objRef,
                    m_objDataRef,
                    glm::translate(glm::mat4(1.0f), transVector) *
                    glm::scale(glm::mat4(1.0f), scaleVector));

            if (size >= finalSize && !shouldUnveil) {
                timeSoFar = 0.0f;
                finished = true;
            }

            if (size <= minSize && shouldUnveil) {
                finished = true;
            }

        return true;
    }
} // namespace growingQuad