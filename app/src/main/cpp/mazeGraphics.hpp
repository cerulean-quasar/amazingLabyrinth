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
#ifndef AMAZING_LABYRINTH_MAZE_GRAPHICS_HPP
#define AMAZING_LABYRINTH_MAZE_GRAPHICS_HPP

#include <memory>
#include <boost/implicit_cast.hpp>

#include "level/levelTracker.hpp"
#include "common.hpp"
#include "serializeSaveData.hpp"
#include "saveData.hpp"

class LevelSequence {
public:
    inline uint32_t surfaceWidth() { return m_surfaceWidth; }
    inline uint32_t surfaceHeight() { return m_surfaceHeight; }
    inline DrawObjectTable const &levelStaticObjsData() { return m_staticObjsData; }
    inline DrawObjectTable const &levelDynObjsData() { return m_dynObjsData; }
    inline DrawObjectTable const &finisherObjsData() { return m_levelFinisherObjsData; }
    inline DrawObjectTable const &starterStaticObjsData() { return m_levelStarterStaticObjsData; }
    inline DrawObjectTable const &starterDynObjsData() { return m_levelStarterDynObjsData; }

    inline TextureMap const &starterTextures() { return m_texturesLevelStarter; }
    inline TextureMap const &levelTextures() { return m_texturesLevel; }
    inline TextureMap const &finisherTextures() { return m_texturesLevelFinisher; }

    virtual glm::mat4 getPerspectiveMatrixForLevel(uint32_t surfaceWidth, uint32_t surfaceHeight) = 0;

    glm::mat4 projectionMatrix() {
        if (!m_projInitialized) {
            updatePerspectiveMatrix(m_surfaceWidth, m_surfaceHeight);
            m_projInitialized = true;
        }
        return m_proj;
    }

    glm::mat4 viewMatrix() {
        if (!m_viewInitialized) {
            setView();
            m_viewInitialized = true;
        }
        return m_view;
    }

    glm::vec3 lightingSource() { return m_lightingSource; }
    glm::mat4 viewLightSource() { return m_viewLightingSource; }
    glm::vec4 backgroundColor() { return m_level->getBackgroundColor(); }
    bool needFinisherObjs() { return m_level->isFinished() || m_levelFinisher->isUnveiling(); }

    void saveLevelData() {
        Point<uint32_t> screenSize{m_surfaceWidth, m_surfaceHeight};
        auto gd = std::make_shared<GameSaveData>(screenSize, m_levelTracker->getLevelName(),
                m_levelStarter != nullptr);
        if (m_level != nullptr) {
            auto saveFcn = m_level->getSaveLevelDataFcn();
            auto saveData = saveFcn(gd);
            m_gameRequester->sendSaveData(saveData);
        } else {
            auto saveFcn = Level::getBasicSaveLevelDataFcn();
            auto saveData = saveFcn(gd);
            m_gameRequester->sendSaveData(saveData);
        }
    }

    bool updateData();
    void updateAcceleration(float x, float y, float z);
    void changeLevel(size_t level);

    LevelSequence(std::shared_ptr<GameRequester> inRequester,
                  uint32_t surfaceWidth,
                  uint32_t surfaceHeight,
                  bool isGL)
            : m_isGL{isGL},
              m_surfaceWidth{surfaceWidth},
              m_surfaceHeight{surfaceHeight},
              m_gameRequester{std::move(inRequester)},
              m_projInitialized{false},
              m_viewInitialized{false},
              m_proj{},
              m_view{},
              m_viewLightingSource{},
              m_lightingSource{},
              m_levelTracker{},
              m_texturesLevel{},
              m_texturesLevelStarter{},
              m_texturesLevelFinisher{},
              m_texturesChanged{false},
              m_staticObjsData{},
              m_dynObjsData{},
              m_levelFinisherObjsData{},
              m_levelStarterStaticObjsData{},
              m_levelStarterDynObjsData{},
              m_level{},
              m_levelFinisher{},
              m_levelStarter{}

    {
        Point<uint32_t> screenSize = {surfaceWidth, surfaceHeight};
        RestoreData saveRestore = m_gameRequester->getSaveData(screenSize);
        m_levelName = saveRestore.levelName;
        m_levelGroupFcns = saveRestore.levelGroupFcns;

        /* leave initialization of the level tracker and levels for the first render operation.
         * This is needed because they involve virtual functions.
         */
    }

private:
    bool m_isGL;
    uint32_t m_surfaceWidth;
    uint32_t m_surfaceHeight;
    std::shared_ptr<GameRequester> m_gameRequester;

    bool m_viewInitialized;
    bool m_projInitialized;

protected:
    static float constexpr m_perspectiveViewAngle = 3.1415926f/4.0f;
    static float constexpr m_perspectiveNearPlane = 0.5f;
    static float constexpr m_perspectiveFarPlane = 5.0f;
    glm::mat4 m_proj;
    glm::mat4 m_view;
    glm::mat4 m_viewLightingSource;
    glm::vec3 m_lightingSource;
    std::shared_ptr<LevelTracker> m_levelTracker;
    std::string m_levelName;
    LevelGroup m_levelGroupFcns;

    TextureMap m_texturesLevel;
    TextureMap m_texturesLevelStarter;
    TextureMap m_texturesLevelFinisher;

    bool m_texturesChanged;

    DrawObjectTable m_staticObjsData;
    DrawObjectTable m_dynObjsData;
    DrawObjectTable m_levelFinisherObjsData;
    DrawObjectTable m_levelStarterStaticObjsData;
    DrawObjectTable m_levelStarterDynObjsData;

    std::shared_ptr<Level> m_level;
    std::shared_ptr<LevelFinish> m_levelFinisher;
    std::shared_ptr<LevelStarter> m_levelStarter;

    /* initialize the level tracker and all the level data */
    void initializeLevelTracker();

    void setView();
    virtual void updatePerspectiveMatrix(uint32_t surfaceWidth, uint32_t surfaceHeight) = 0;
    void setLightingSource();
    virtual void setupCommonBuffers() { /* work only needed in Vulkan */ }
    void setViewLightingSource();
    void addObjects(DrawObjectTable &objs, TextureMap &textures);
    void addTextures(TextureMap &textures);
    void initializeLevelData(std::shared_ptr<Level> const &level, DrawObjectTable &staticObjsData,
                             DrawObjectTable &dynObjsData, TextureMap &textures);

    virtual std::shared_ptr<TextureData> createTexture(std::shared_ptr<TextureDescription> const &textureDescription) = 0;
    virtual std::shared_ptr<DrawObjectData> createObject(std::shared_ptr<DrawObject> const &obj, TextureMap &textures) = 0;
    virtual void updateLevelData(DrawObjectTable &objsData, TextureMap &textures) = 0;

private:
    glm::mat4 getViewMatrix() {
        return glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f),
                glm::vec3(0.0f, 0.0f, LevelTracker::m_maxZLevel), glm::vec3(0.0f, 1.0f, 0.0f));
    }
};

class Graphics {
public:
    virtual void initThread()=0;

    void updateAcceleration(float x, float y, float z) {
        m_levelSequence->updateAcceleration(x, y, z);
    }

    void changeLevel(size_t level) {
        m_levelSequence->changeLevel(level);
    }

    virtual void drawFrame()=0;

    virtual bool updateData()=0;

    virtual void recreateSwapChain(uint32_t width, uint32_t height)=0;

    virtual GraphicsDescription graphicsDescription() = 0;

    void sendGraphicsDescription(bool hasAccelerometer, std::string vulkanError) {
        GraphicsDescription info = graphicsDescription();
        if (!vulkanError.empty()) {
            info.m_extraInfo.push_back(vulkanError);
        }
        m_gameRequester->sendGraphicsDescription(info, hasAccelerometer);
    }

    void saveLevelData() {
        return m_levelSequence->saveLevelData();
    }

    virtual void cleanupThread()=0;

    // some levels use this function to get a depth texture.
    virtual std::shared_ptr<TextureData> getDepthTexture(
            DrawObjectTable const &objsData,
            float width,
            float height,
            uint32_t nbrWidthSamples,
            float farthestDepth,
            float nearestDepth,
            std::vector<float> &depthValues,
            std::vector<glm::vec3> &normalMap) = 0;

    explicit Graphics(GameRequesterCreator inRequesterCreator)
        : m_gameRequester{inRequesterCreator(this)},
        m_levelSequence{}
    {}

    virtual ~Graphics() = default;

protected:
    std::shared_ptr<GameRequester> m_gameRequester;
    std::shared_ptr<LevelSequence> m_levelSequence;

    static float constexpr m_depthTextureNearPlane = 0.1f;
    static float constexpr m_depthTextureFarPlane = 10.0f;

    bool testDepthTexture();
};

#endif // AMAZING_LABYRINTH_MAZE_GRAPHICS_HPP