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

void LevelSequence::notifySurfaceChanged(
        uint32_t surfaceWidth,
        uint32_t surfaceHeight,
        glm::mat4 preTransform)
{
    updatePretransform(preTransform);
    m_surfaceWidth = surfaceWidth;
    m_surfaceHeight = surfaceHeight;

    cleanupLevelData();
    m_levelGroupFcns = m_levelTracker->getLevelGroupFcns(m_surfaceWidth, m_surfaceHeight);

    // need to regenerate the maze if the width/height of the surface changed.
    if (m_levelStarter) {
        m_levelStarter = m_levelGroupFcns.getStarterFcn(
                levelDrawer::Adaptor(levelDrawer::STARTER, m_levelDrawer));
    }

    m_level = m_levelGroupFcns.getLevelFcn(
            levelDrawer::Adaptor(levelDrawer::LEVEL, m_levelDrawer));

    float x, y, z;
    m_level->getLevelFinisherCenter(x, y, z);
    m_levelFinisher = m_levelGroupFcns.getFinisherFcn(
            levelDrawer::Adaptor(levelDrawer::FINISHER, m_levelDrawer), x, y, z);

    saveLevelData();
}

void LevelSequence::changeLevel(std::string const &level) {
    m_levelTracker->setLevel(level);
    m_levelGroupFcns = m_levelTracker->getLevelGroupFcns(m_surfaceWidth, m_surfaceHeight);

    cleanupLevelData();
    m_level = m_levelGroupFcns.getLevelFcn(
            levelDrawer::Adaptor(levelDrawer::LEVEL, m_levelDrawer));
    m_levelStarter = m_levelGroupFcns.getStarterFcn(
            levelDrawer::Adaptor(levelDrawer::STARTER, m_levelDrawer));

    float x, y, z;
    m_level->getLevelFinisherCenter(x, y, z);
    m_levelFinisher = m_levelGroupFcns.getFinisherFcn(
            levelDrawer::Adaptor(levelDrawer::STARTER, m_levelDrawer), x, y, z);
}

bool LevelSequence::updateData(bool alwaysUpdateDynObjs) {

    bool drawingNecessary = false;

    if (m_level->isFinished() || m_levelFinisher->isUnveiling()) {
        if (m_levelFinisher->isUnveiling()) {
            if (m_levelFinisher->isDone()) {
                m_levelDrawer->clearDrawObjectTable(levelDrawer::FINISHER);
                float x, y, z;
                m_level->getLevelFinisherCenter(x, y, z);

                m_levelDrawer->clearDrawObjectTable(levelDrawer::FINISHER);
                m_levelFinisher = m_levelGroupFcns.getFinisherFcn(
                        levelDrawer::Adaptor(levelDrawer::FINISHER, m_levelDrawer), x, y, z);

                m_levelStarter->start();
                return false;
            }
        } else {
            if (m_levelFinisher->isDone()) {

                m_levelTracker->gotoNextLevel();
                m_levelGroupFcns = m_levelTracker->getLevelGroupFcns(m_surfaceWidth, m_surfaceHeight);

                m_levelDrawer->clearDrawObjectTable(levelDrawer::STARTER);
                m_levelStarter = m_levelGroupFcns.getStarterFcn(
                        levelDrawer::Adaptor(levelDrawer::STARTER, m_levelDrawer));

                m_levelDrawer->clearDrawObjectTable(levelDrawer::LEVEL);
                m_level = m_levelGroupFcns.getLevelFcn(
                        levelDrawer::Adaptor(levelDrawer::LEVEL, m_levelDrawer));

                saveLevelData();

                m_levelFinisher->unveilNewLevel();
                return false;
            }
        }

        drawingNecessary = m_levelFinisher->updateDrawObjects();
        if (!drawingNecessary && !alwaysUpdateDynObjs) {
            return false;
        }
    } else if (m_levelStarter != nullptr) {
        drawingNecessary = m_levelStarter->updateData();

        if (m_levelStarter->isFinished()) {
            m_levelStarter.reset();
            m_levelDrawer->clearDrawObjectTable(levelDrawer::STARTER);
            m_level->start();
            return false;
        }

        if (drawingNecessary || alwaysUpdateDynObjs) {
            m_levelStarter->updateDrawObjects();
        }
    } else {
        drawingNecessary = m_level->updateData();

        if (drawingNecessary || alwaysUpdateDynObjs) {
            m_level->updateDrawObjects();
        }

        if (m_level->isFinished()) {
            m_levelFinisher->start();
        }
    }

    return drawingNecessary || alwaysUpdateDynObjs;
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

bool Graphics::testDepthTexture(levelDrawer::Adaptor inLevelDrawer) {
    levelDrawer::ModelsTextures modelsTextures{std::make_pair(std::make_shared<levelDrawer::ModelDescriptionQuad>(),
            std::shared_ptr<levelDrawer::TextureDescription>())};

    // just request something to be drawn that is definitely bigger than the screen
    glm::mat4 modelMatrix =
            glm::scale(glm::mat4(1.0f), glm::vec3{10.0f, 10.f, 1.0f});

    std::vector<float> depthMap;
    inLevelDrawer.drawToBuffer(depthMapRenderDetailsName, modelsTextures, {modelMatrix},
            2.0f, 2.0f, 200, -1.0f, 1.0f, depthMap);

    /*
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
     */

    float errVal = 0.01f;
    auto cmpDepth = std::function<bool(float, float)>([errVal](float v1, float v2) -> bool {
        return v1 < v2 + errVal && v1 > v2 - errVal;
    });

    /*
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
            */

    return testMap<float>(0.0f, depthMap, cmpDepth);
}