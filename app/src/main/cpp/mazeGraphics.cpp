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
    m_viewLightingSource = glm::lookAt(m_lightingSource,
            glm::vec3(0.0f, 0.0f, levelTracker::Loader::m_maxZLevel),
            glm::vec3(0.0f, 1.0f, 0.0f));
}

void LevelSequence::setView() {
    /* glm::lookAt takes the eye position, the center position, and the up axis as parameters */
    m_view = getViewMatrix();
}

void LevelSequence::setLightingSource() {
    m_lightingSource = glm::vec3(1.0f, 1.0f, 1.5f);
}

void LevelSequence::updateAcceleration(float x, float y, float z) {
    if (m_levelStarter) {
        m_levelStarter->updateAcceleration(x, y, z);
    } else if (m_level) {
        m_level->updateAcceleration(x, y, z);
    }
}

void LevelSequence::initializeLevelData(std::shared_ptr<basic::Level> const &level, DrawObjectTable &staticObjsData,
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
    for (auto &texture : textures) {
        if (texture.second.get() == nullptr) {
            texture.second = createTexture(texture.first);
        }
    }
}

bool LevelSequence::updateData(bool alwaysUpdateDynObjs) {
    m_initialize();

    bool drawingNecessary = false;

    if (m_level->isFinished() || m_levelFinisher->isUnveiling()) {
        auto proj = getPerspectiveMatrixForLevel(m_surfaceWidth, m_surfaceHeight);
        if (m_levelFinisher->isUnveiling()) {
            if (m_levelFinisher->isDone()) {
                m_levelFinisherObjsData.clear();
                m_texturesLevelFinisher.clear();
                float x, y, z;
                m_level->getLevelFinisherCenter(x, y, z);
                m_levelFinisher = m_levelGroupFcns.getFinisherFcn(m_gameRequester, proj, m_view, x, y, z);
                m_levelStarter->start();
                return false;
            }
        } else {
            if (m_levelFinisher->isDone()) {

                m_levelTracker->gotoNextLevel();
                m_levelGroupFcns = m_levelTracker->getLevelGroupFcns(m_surfaceWidth, m_surfaceHeight);

                m_levelStarter = m_levelGroupFcns.getStarterFcn(m_gameRequester, proj, m_view);
                initializeLevelData(m_levelStarter, m_levelStarterStaticObjsData,
                                    m_levelStarterDynObjsData, m_texturesLevelStarter);

                m_level = m_levelGroupFcns.getLevelFcn(m_gameRequester, proj, m_view);
                initializeLevelData(m_level, m_staticObjsData, m_dynObjsData, m_texturesLevel);

                saveLevelData();

                m_levelFinisher->unveilNewLevel();
                return false;
            }
        }

        drawingNecessary = m_levelFinisher->updateDrawObjects(m_levelFinisherObjsData,
                                                              m_texturesLevelFinisher,
                                                              m_texturesChanged);
        if (!drawingNecessary && !alwaysUpdateDynObjs) {
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

        if (drawingNecessary || alwaysUpdateDynObjs) {
            m_levelStarter->updateDynamicDrawObjects(m_levelStarterDynObjsData,
                                                     m_texturesLevelStarter, m_texturesChanged);
            updateLevelData(m_levelStarterDynObjsData, m_texturesLevelStarter);
        }
    } else {
        drawingNecessary = m_level->updateData();

        if (drawingNecessary || alwaysUpdateDynObjs) {
            m_level->updateDynamicDrawObjects(m_dynObjsData, m_texturesLevel, m_texturesChanged);
            updateLevelData(m_dynObjsData, m_texturesLevel);
        }
    }

    return drawingNecessary || alwaysUpdateDynObjs;
}

void LevelSequence::changeLevel(std::string const &level) {
    m_levelTracker->setLevel(level);
    m_levelGroupFcns = m_levelTracker->getLevelGroupFcns(m_surfaceWidth, m_surfaceHeight);
    if (!m_initialize()) {
        cleanupLevelData();
        auto proj = getPerspectiveMatrixForLevel(m_surfaceWidth, m_surfaceHeight);
        m_level = m_levelGroupFcns.getLevelFcn(m_gameRequester, proj, m_view);
        m_levelStarter = m_levelGroupFcns.getStarterFcn(m_gameRequester, proj, m_view);

        float x, y, z;
        m_level->getLevelFinisherCenter(x, y, z);
        m_levelFinisher = m_levelGroupFcns.getFinisherFcn(m_gameRequester, proj, m_view, x, y, z);

        if (m_levelStarter != nullptr && ! m_levelStarter->isFinished()) {
            initializeLevelData(m_levelStarter, m_levelStarterStaticObjsData,
                                m_levelStarterDynObjsData, m_texturesLevelStarter);
        }
        initializeLevelData(m_level, m_staticObjsData, m_dynObjsData, m_texturesLevel);
    }
}

// Called in preparation to calling notifySurfaceChanged
void LevelSequence::cleanupLevelData() {
    m_levelStarterDynObjsData.clear();
    m_levelStarterStaticObjsData.clear();
    m_texturesLevelStarter.clear();

    m_staticObjsData.clear();
    m_dynObjsData.clear();
    m_texturesLevel.clear();

    m_levelFinisherObjsData.clear();
    m_texturesLevelFinisher.clear();
}

void LevelSequence::notifySurfaceChanged(uint32_t surfaceWidth, uint32_t surfaceHeight, glm::mat4 preTransform) {
    updatePretransform(preTransform);
    m_surfaceWidth = surfaceWidth;
    m_surfaceHeight = surfaceHeight;
    updatePerspectiveMatrix(surfaceWidth, surfaceHeight);
    setupCommonBuffers();
    glm::mat4 projForLevel = getPerspectiveMatrixForLevel(m_surfaceWidth, m_surfaceHeight);
    m_levelGroupFcns = m_levelTracker->getLevelGroupFcns(m_surfaceWidth, m_surfaceHeight);

    // need to regenerate the maze if the width/height of the surface changed.
    if (m_levelStarter) {
        m_levelStarter = m_levelGroupFcns.getStarterFcn(m_gameRequester, projForLevel, m_view);
        initializeLevelData(m_levelStarter, m_levelStarterStaticObjsData,
                            m_levelStarterDynObjsData, m_texturesLevelStarter);
    }

    m_level = m_levelGroupFcns.getLevelFcn(m_gameRequester, projForLevel, m_view);
    initializeLevelData(m_level, m_staticObjsData, m_dynObjsData, m_texturesLevel);

    m_levelFinisherObjsData.clear();
    m_texturesLevelFinisher.clear();
    float x, y, z;
    m_level->getLevelFinisherCenter(x, y, z);
    m_levelFinisher = m_levelGroupFcns.getFinisherFcn(m_gameRequester, projForLevel, m_view, x, y, z);

    saveLevelData();
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

bool Graphics::testDepthTexture() {
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