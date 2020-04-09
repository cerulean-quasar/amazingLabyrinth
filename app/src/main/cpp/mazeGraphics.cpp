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

#include "mazeGraphics.hpp"

void LevelSequence::setViewLightingSource() {
    m_viewLightingSource = glm::lookAt(m_lightingSource, glm::vec3(0.0f, 0.0f, LevelTracker::m_maxZLevel), glm::vec3(0.0f, 1.0f, 0.0f));
}

void LevelSequence::setView() {
    /* glm::lookAt takes the eye position, the center position, and the up axis as parameters */
    m_view = getViewMatrix();
}

void LevelSequence::setLightingSource() {
    m_lightingSource = glm::vec3(0.0f, 1.0f, 1.28f);
}

void LevelSequence::updateAcceleration(float x, float y, float z) {
    if (m_levelStarter.get() != nullptr) {
        m_levelStarter->updateAcceleration(x, y, z);
    } else {
        m_level->updateAcceleration(x, y, z);
    }
}

void LevelSequence::initializeLevelData(std::shared_ptr<Level> const &level, DrawObjectTable &staticObjsData,
                                        DrawObjectTable &dynObjsData, TextureMap &textures) {
    staticObjsData.clear();
    dynObjsData.clear();
    textures.clear();
    level->updateStaticDrawObjects(staticObjsData, textures);
    bool texturesChanged;
    level->updateDynamicDrawObjects(dynObjsData, textures, texturesChanged);
    addTextures(textures);
    addObjects(staticObjsData, textures);
    addObjects(dynObjsData, textures);
    m_texturesChanged = true;
}

void LevelSequence::addObjects(DrawObjectTable &objs, TextureMap &textures) {
    for (auto &obj : objs) {
        obj.second = createObject(obj.first, textures);
    }
}

void LevelSequence::addTextures(TextureMap &textures) {
    for (auto it = textures.begin(); it != textures.end(); it++) {
        if (it->second.get() == nullptr) {
            it->second = createTexture(it->first);
        }
    }
}

void LevelSequence::initializeLevelTracker() {
    if (m_levelTracker != nullptr) {
        return;
    }

    setView();
    updatePerspectiveMatrix(m_surfaceWidth, m_surfaceHeight);

    m_levelTracker = std::make_shared<LevelTracker>(m_gameRequester,
        getPerspectiveMatrixForLevel(m_surfaceWidth, m_surfaceHeight), m_view);

    setLightingSource();
    setViewLightingSource();

    m_levelTracker->setLevel(m_levelName);
    m_level = m_levelGroupFcns.getLevelFcn(*m_levelTracker);
    m_levelStarter = m_levelGroupFcns.getStarterFcn(*m_levelTracker);

    float x, y;
    m_level->getLevelFinisherCenter(x, y);
    m_levelFinisher = m_levelGroupFcns.getFinisherFcn(*m_levelTracker, x, y,
            getPerspectiveMatrixForLevel(m_surfaceWidth, m_surfaceHeight), m_view);

    setupLightingSourceBuffer();

    if (m_levelStarter != nullptr && ! m_levelStarter->isFinished()) {
        initializeLevelData(m_levelStarter, m_levelStarterStaticObjsData,
                            m_levelStarterDynObjsData, m_texturesLevelStarter);
    }
    initializeLevelData(m_level, m_staticObjsData, m_dynObjsData, m_texturesLevel);
}

bool LevelSequence::updateData() {
    initializeLevelTracker();

    bool drawingNecessary = false;

    if (m_level->isFinished() || m_levelFinisher->isUnveiling()) {
        if (m_levelFinisher->isUnveiling()) {
            if (m_levelFinisher->isDone()) {
                m_levelFinisherObjsData.clear();
                m_texturesLevelFinisher.clear();
                float x, y;
                m_level->getLevelFinisherCenter(x, y);
                m_levelFinisher = m_levelGroupFcns.getFinisherFcn(*m_levelTracker, x, y, m_proj, m_view);
                m_levelStarter->start();
                return false;
            }
        } else {
            if (m_levelFinisher->isDone()) {

                m_levelTracker->gotoNextLevel();
                m_levelGroupFcns = m_levelTracker->getLevelGroupFcns();

                m_levelStarter = m_levelGroupFcns.getStarterFcn(*m_levelTracker);
                initializeLevelData(m_levelStarter, m_levelStarterStaticObjsData,
                                    m_levelStarterDynObjsData, m_texturesLevelStarter);

                m_level = m_levelGroupFcns.getLevelFcn(*m_levelTracker);
                initializeLevelData(m_level, m_staticObjsData, m_dynObjsData, m_texturesLevel);

                m_levelFinisher->unveilNewLevel();
                return false;
            }
        }

        drawingNecessary = m_levelFinisher->updateDrawObjects(m_levelFinisherObjsData,
                                                              m_texturesLevelFinisher,
                                                              m_texturesChanged);
        if (!drawingNecessary) {
            return false;
        }

        updateLevelData(m_levelFinisherObjsData, m_texturesLevelFinisher);
    } else if (m_levelStarter != nullptr) {
        drawingNecessary = m_levelStarter->updateData();

        if (m_levelStarter->isFinished()) {
            m_levelStarter.reset();
            m_levelStarterDynObjsData.clear();
            m_levelStarterStaticObjsData.clear();
            m_texturesLevelStarter.clear();
            m_level->start();
            return false;
        }

        if (drawingNecessary) {
            m_levelStarter->updateDynamicDrawObjects(m_levelStarterDynObjsData,
                                                     m_texturesLevelStarter, m_texturesChanged);
            updateLevelData(m_levelStarterDynObjsData, m_texturesLevelStarter);
        }
    } else {
        drawingNecessary = m_level->updateData();

        if (drawingNecessary) {
            m_level->updateDynamicDrawObjects(m_dynObjsData, m_texturesLevel, m_texturesChanged);
            updateLevelData(m_dynObjsData, m_texturesLevel);
        }
    }

    return drawingNecessary;
}

void LevelSequence::changeLevel(size_t level) {
    m_levelTracker->setLevel(level);
    m_levelGroupFcns = m_levelTracker->getLevelGroupFcns();
    m_levelStarter = m_levelGroupFcns.getStarterFcn(*m_levelTracker);
    initializeLevelData(m_levelStarter, m_levelStarterStaticObjsData,
                        m_levelStarterDynObjsData, m_texturesLevelStarter);

    m_level = m_levelGroupFcns.getLevelFcn(*m_levelTracker);
    initializeLevelData(m_level, m_staticObjsData, m_dynObjsData, m_texturesLevel);

    m_levelFinisherObjsData.clear();
    m_texturesLevelFinisher.clear();

    float x, y;
    m_level->getLevelFinisherCenter(x, y);
    m_levelFinisher = m_levelGroupFcns.getFinisherFcn(*m_levelTracker, x, y, m_proj, m_view);

    m_levelStarter->start();
}

template <typename data_type>
bool testMap(
    data_type const &expected,
    std::vector<data_type> const &valueMap,
    std::function<bool(data_type, data_type)> cmp)
{
    for (auto value : valueMap) {
        if (!cmp(expected, value)) {
            return false;
        }
    }

    return true;
}

bool Graphics::testDepthTexture(bool depth0to1) {
    float modelSize = 2.0f;

    auto obj = std::make_shared<DrawObject>();
    getQuad(obj->vertices, obj->indices);

    glm::mat4 view = m_levelSequence->viewMatrix();
    glm::mat4 proj = m_levelSequence->getPerspectiveMatrixForLevel(m_levelSequence->surfaceWidth(),
            m_levelSequence->surfaceHeight());
    auto widthHeight = getWidthHeight(0.0f, proj, view);
    glm::mat4 modelMatrix =
            glm::scale(glm::mat4(1.0f),
                       glm::vec3{widthHeight.first/modelSize, widthHeight.second/modelSize, 1.0f});

    glm::vec3 normal{0.5f, 0.5f, 0.5f};
    // set normal to bogus value that is different from the clear color.
    for (auto &vertex : obj->vertices) {
        vertex.normal = normal;
    }

    obj->modelMatrices.push_back(modelMatrix);
    obj->texture = nullptr;

    DrawObjectTable drawObjsData;
    drawObjsData.emplace_back(obj, nullptr);
    std::vector<float> depthMap;
    std::vector<glm::vec3> normalMap;
    getDepthTexture(drawObjsData, widthHeight.first, widthHeight.second, 200, -1.0f, 1.0f, depthMap, normalMap);

    float errVal = 0.01f;
    auto cmpDepth = std::function<bool(float, float)>([errVal](float v1, float v2) -> bool {
        return v1 < v2 + errVal && v1 > v2 - errVal;
    });

    auto cmpNormal = std::function<bool(glm::vec3, glm::vec3)>([errVal](glm::vec3 v1, glm::vec3 v2) -> bool {
        return v1.x < v2.x + errVal && v1.x > v2.x - errVal &&
                v1.y < v2.y + errVal && v1.y > v2.y - errVal &&
                v1.z < v2.z + errVal && v1.z > v2.z - errVal;
    });

    glm::vec4 expectedNormal4 = glm::transpose(glm::inverse(modelMatrix)) *
            glm::vec4{normal.x, normal.y, normal.z, 1.0f};
    glm::vec3 expectedNormal = glm::normalize(glm::vec3{expectedNormal4.x/expectedNormal4.w,
                                                        expectedNormal4.y/expectedNormal4.w,
                                                        expectedNormal4.z/expectedNormal4.w});
    return testMap<float>(0.0f, depthMap, cmpDepth) &&
            testMap<glm::vec3>(expectedNormal, normalMap, cmpNormal);
}