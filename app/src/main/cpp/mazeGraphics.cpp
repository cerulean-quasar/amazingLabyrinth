/**
 * Copyright 2024 Cerulean Quasar. All Rights Reserved.
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
#include "levelDrawer/modelTable/modelLoader.hpp"

void LevelSequence::notifySurfaceChanged(
        uint32_t surfaceWidth,
        uint32_t surfaceHeight,
        bool levelStarterRequired)
{
    m_surfaceWidth = surfaceWidth;
    m_surfaceHeight = surfaceHeight;

    cleanupLevelData();
    m_levelGroupFcns = m_levelTracker->getLevelGroupFcns(m_surfaceWidth, m_surfaceHeight);

    // need to regenerate the maze if the width/height of the surface changed.
    if (levelStarterRequired) {
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
            levelDrawer::Adaptor(levelDrawer::FINISHER, m_levelDrawer), x, y, z);
}

bool LevelSequence::updateData(bool alwaysUpdateDynObjs) {

    bool drawingNecessary = false;

    if (m_level->isFinished() || m_levelFinisher->isUnveiling()) {
        if (m_levelFinisher->isUnveiling()) {
            if (m_levelFinisher->isDone()) {
                float x, y, z;
                m_level->getLevelFinisherCenter(x, y, z);

                m_levelFinisher.reset();
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

                m_levelStarter.reset();
                m_levelDrawer->clearDrawObjectTable(levelDrawer::STARTER);
                m_levelStarter = m_levelGroupFcns.getStarterFcn(
                        levelDrawer::Adaptor(levelDrawer::STARTER, m_levelDrawer));

                m_level.reset();
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
    auto checkResult = [&expected, cmp](data_type const &data) -> bool {
        return cmp(expected, data);
    };
    return std::all_of(valueMap.begin(), valueMap.end(), checkResult);
}

class ModelDescriptionTestQuad : public levelDrawer::ModelDescription {
private:
    uint8_t m_whichNormalsToLoad;
public:
    explicit ModelDescriptionTestQuad(uint8_t const &whichNormalsToLoad)
        : levelDrawer::ModelDescription(),
        m_whichNormalsToLoad{whichNormalsToLoad}
    {}

    uint8_t normalsToLoad() override { return m_whichNormalsToLoad; }

    std::pair<levelDrawer::ModelVertices, levelDrawer::ModelVertices> getData(std::shared_ptr<GameRequester> const &gameRequester) override
    {
        levelDrawer::ModelDescriptionQuad quad;
        auto obj = quad.getData(gameRequester);
        // set normal to bogus value that is different from the clear color.
        for (auto &vertex : obj.first.first) {
            vertex.normal = getNormal3Vec();
        }

        if (m_whichNormalsToLoad & LOAD_VERTEX_NORMALS) {
            obj.second = obj.first;
        }
        return std::move(obj);
    }

    static glm::vec3 getNormal3Vec() {
        return glm::vec3{0.5f, 0.5f, 0.5f};
    }

    static glm::vec4 getNormal4Vec() {
        return glm::vec4{getNormal3Vec(), 1.0f};
    }

    bool compareLess(ModelDescription *other) override {
        auto otherQuad = dynamic_cast<ModelDescriptionTestQuad *>(other);
        if (otherQuad == nullptr) {
            // should never happen
            throw std::runtime_error("Quad: Comparing incompatible pointers");
        }

        return false;
    }
};

bool Graphics::testDepthTexture(levelDrawer::Adaptor inLevelDrawer) {
    levelDrawer::ModelsTextures modelsTextures{
        std::make_pair<std::shared_ptr<levelDrawer::ModelDescription>, std::shared_ptr<levelDrawer::TextureDescription>>(
            std::make_shared<levelDrawer::ModelDescriptionQuad>(),
            std::shared_ptr<levelDrawer::TextureDescription>())};

    // just request something to be drawn that is definitely bigger than the screen
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    renderDetails::ParametersDepthMap depthParameters{};
    depthParameters.lookAt = gameConstants::lookAt;
    depthParameters.up = gameConstants::up;
    depthParameters.viewPoint = gameConstants::viewPoint;
    depthParameters.nearPlane = gameConstants::nearPlane;
    depthParameters.farPlane = gameConstants::farPlane;
    depthParameters.nearestDepth = 1.0f;
    depthParameters.farthestDepth = -1.0f;
    depthParameters.widthAtDepth = 2.0f;
    depthParameters.heightAtDepth = 2.0f;
    std::vector<float> depthMap;
    inLevelDrawer.drawToBuffer(
            {renderDetails::DrawingStyle::depthMap, renderDetails::FeatureList(), renderDetails::FeatureList()},
            modelsTextures, {modelMatrix},
            2.0f, 2.0f, 200,
            std::make_shared<renderDetails::ParametersDepthMap>(depthParameters),
            depthMap);

    levelDrawer::ModelsTextures modelsTextures1{
            std::make_pair<std::shared_ptr<levelDrawer::ModelDescription>, std::shared_ptr<levelDrawer::TextureDescription>>(
                    std::make_shared<ModelDescriptionTestQuad>(levelDrawer::ModelDescription::LOAD_BOTH),
                    std::shared_ptr<levelDrawer::TextureDescription>())};

    renderDetails::ParametersNormalMap normalParameters{};
    normalParameters.lookAt = gameConstants::lookAt;
    normalParameters.up = gameConstants::up;
    normalParameters.viewPoint = gameConstants::viewPoint;
    normalParameters.nearPlane = gameConstants::nearPlane;
    normalParameters.farPlane = gameConstants::farPlane;
    normalParameters.widthAtDepth = 2.0f;
    normalParameters.heightAtDepth = 2.0f;
    std::vector<float> rawNormalMap;
    inLevelDrawer.drawToBuffer(
        {renderDetails::DrawingStyle::normalMap, renderDetails::FeatureList(), renderDetails::FeatureList()},
        modelsTextures1, {modelMatrix},
        2.0f, 2.0f, 200,
        std::make_shared<renderDetails::ParametersNormalMap>(normalParameters),
        rawNormalMap);
    std::vector<glm::vec3> normalMap;
    unFlattenMap(rawNormalMap, normalMap);

    float errVal = 0.01f;
    auto cmpDepth = std::function<bool(float, float)>([errVal](float v1, float v2) -> bool {
        return v1 < v2 + errVal && v1 > v2 - errVal;
    });

    auto cmpNormal = std::function<bool(glm::vec3, glm::vec3)>([errVal](glm::vec3 v1, glm::vec3 v2) -> bool {
        return v1.x < v2.x + errVal && v1.x > v2.x - errVal &&
                v1.y < v2.y + errVal && v1.y > v2.y - errVal &&
                v1.z < v2.z + errVal && v1.z > v2.z - errVal;
    });

    glm::vec4 expectedNormal4 = glm::transpose(glm::inverse(modelMatrix)) * ModelDescriptionTestQuad::getNormal4Vec();
    glm::vec3 expectedNormal = glm::normalize(glm::vec3{expectedNormal4.x/expectedNormal4.w,
                                                        expectedNormal4.y/expectedNormal4.w,
                                                        expectedNormal4.z/expectedNormal4.w});

    return testMap<float>(0.0f, depthMap, cmpDepth) &&
            testMap<glm::vec3>(expectedNormal, normalMap, cmpNormal);

    //return testMap<float>(0.0f, depthMap, cmpDepth);
}