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

#include "levelTracker/levelTracker.hpp"
#include "levels/basic/level.hpp"
#include "levels/finisher/types.hpp"
#include "common.hpp"

class LevelSequence {
public:
    inline uint32_t surfaceWidth() { return m_surfaceWidth; }
    inline uint32_t surfaceHeight() { return m_surfaceHeight; }
    bool needFinisherObjs() { return m_level->isFinished() || m_levelFinisher->isUnveiling(); }

    void saveLevelData() {
        levelTracker::saveGameData(m_gameRequester, m_surfaceWidth, m_surfaceHeight,
                m_levelTracker->levelName(), m_level, (m_levelStarter != nullptr));
    }

    void updateAcceleration(float x, float y, float z) {
        if (m_levelStarter) {
            m_levelStarter->updateAcceleration(x, y, z);
        } else if (m_level) {
            m_level->updateAcceleration(x, y, z);
        }
    }

    bool drag(float startX, float startY, float distanceX, float distanceY) {
        if (!m_level || m_levelStarter) {
            return false;
        }

        auto start = getCoordsAtDepth(startX, startY, true);
        auto distance = getCoordsAtDepth(distanceX, distanceY, false);
        return m_level->drag(start.first, start.second, distance.first, distance.second);
    }

    bool dragEnded(float x, float y) {
        if (!m_level || m_levelStarter) {
            return false;
        }

        auto endPosition = getCoordsAtDepth(x, y, true);
        return m_level->dragEnded(endPosition.first, endPosition.second);
    }

    bool tap(float x, float y) {
        if (!m_level || m_levelStarter) {
            return false;
        }

        auto position = getCoordsAtDepth(x, y, true);
        return m_level->tap(position.first, position.second);
    }

    bool updateData(bool alwaysUpdateDynObjs);

    void changeLevel(std::string const &level);

    // Called in preparation to calling notifySurfaceChanged
    void cleanupLevelData() {
        m_levelStarter.reset();
        m_level.reset();
        m_levelFinisher.reset();

        m_levelDrawer->clearDrawObjectTable(levelDrawer::STARTER);
        m_levelDrawer->clearDrawObjectTable(levelDrawer::LEVEL);
        m_levelDrawer->clearDrawObjectTable(levelDrawer::FINISHER);
    }

    void notifySurfaceChanged(uint32_t surfaceWidth, uint32_t surfaceHeight);

    LevelSequence(
            std::shared_ptr<GameRequester> inGameRequester,
            std::shared_ptr<levelDrawer::LevelDrawer> inLevelDrawer,
            uint32_t surfaceWidth,
            uint32_t surfaceHeight)
            : m_surfaceWidth{surfaceWidth},
              m_surfaceHeight{surfaceHeight},
              m_gameRequester{std::move(inGameRequester)},
              m_levelDrawer{std::move(inLevelDrawer)},
              m_levelTracker{std::make_shared<levelTracker::Loader>(m_gameRequester)},
              m_levelGroupFcns{m_levelTracker->getLevelGroupFcns(m_surfaceWidth, m_surfaceHeight)},
              m_level{m_levelGroupFcns.getLevelFcn(levelDrawer::Adaptor(levelDrawer::LEVEL, m_levelDrawer))},
              m_levelFinisher{},
              m_levelStarter{m_levelGroupFcns.getStarterFcn(levelDrawer::Adaptor(levelDrawer::STARTER, m_levelDrawer))}
    {
        float x, y, z;
        m_level->getLevelFinisherCenter(x, y, z);
        m_levelFinisher = m_levelGroupFcns.getFinisherFcn(levelDrawer::Adaptor(levelDrawer::FINISHER, m_levelDrawer), x, y, z);
    }

private:
    uint32_t m_surfaceWidth;
    uint32_t m_surfaceHeight;
    std::shared_ptr<GameRequester> m_gameRequester;
    std::shared_ptr<levelDrawer::LevelDrawer> m_levelDrawer;

protected:
    std::shared_ptr<levelTracker::Loader> m_levelTracker;
    levelTracker::LevelGroup m_levelGroupFcns;

    std::shared_ptr<basic::Level> m_level;
    std::shared_ptr<finisher::LevelFinisher> m_levelFinisher;
    std::shared_ptr<basic::Level> m_levelStarter;

private:
    // x and y are in pixels off set from the bottom left corner
    inline std::pair<float, float> getCoordsAtDepth(float x, float y, bool isAbsoluteMove) {
        if (isAbsoluteMove) {
            return std::make_pair(x/m_surfaceWidth*2.0f - 1.0f, 1.0f - y / m_surfaceHeight * 2.0f);
        } else {
            return std::make_pair(x/m_surfaceWidth*2.0f, -y / m_surfaceHeight * 2.0f);
        }
    }
};

class Graphics {
public:
    virtual void initThread()=0;

    void changeRotationAngle(float rotationAngle) {
        m_rotationAngle = rotationAngle;
    }

    void updateAcceleration(float x, float y, float z) {
        glm::vec4 acceleration{x, y, z, 1.0f};
        glm::mat4 rotation = glm::rotate(glm::mat4{1.0f}, glm::radians(m_rotationAngle), glm::vec3{0.0f, 0.0f, 1.0f});
        acceleration = rotation * acceleration;
        m_levelSequence->updateAcceleration(
                acceleration.x/acceleration.w,
                acceleration.y/acceleration.w,
                acceleration.z/acceleration.w);
    }

    void changeLevel(std::string const &level) {
        m_levelSequence->changeLevel(level);
    }

    virtual void drawFrame()=0;

    virtual bool updateData(bool alwaysUpdateDynObjs)=0;

    virtual void recreateSwapChain(uint32_t width, uint32_t height)=0;

    virtual GraphicsDescription graphicsDescription() = 0;

    virtual std::shared_ptr<renderDetails::Parameters> getParametersForRenderDetailsName(
            char const *renderDetailsName) = 0;

    void sendGraphicsDescription(bool hasAccelerometer, std::string vulkanError) {
        GraphicsDescription info = graphicsDescription();
        if (!vulkanError.empty()) {
            info.m_extraInfo.push_back(vulkanError);
        }
        m_gameRequester->sendGraphicsDescription(info, hasAccelerometer);
    }

    void sendKeepAliveEnabled(bool keepAliveEnabled) {
        m_gameRequester->sendKeepAliveEnabled(keepAliveEnabled);
    }

    void saveLevelData() {
        return m_levelSequence->saveLevelData();
    }

    bool drag(float startX, float startY, float distanceX, float distanceY) {
        return m_levelSequence->drag(startX, startY, distanceX, distanceY);
    }

    bool dragEnded(float x, float y) {
        return m_levelSequence->dragEnded(x, y);
    }

    bool tap(float x, float y) {
        return m_levelSequence->tap(x, y);
    }

    virtual void cleanupThread()=0;

    explicit Graphics(GameRequesterCreator inRequesterCreator,
            float inRotationAngle)
        : m_gameRequester{inRequesterCreator(this)},
        m_levelSequence{},
        m_rotationAngle{inRotationAngle}
    {}

    virtual ~Graphics() = default;

protected:
    std::shared_ptr<GameRequester> m_gameRequester;
    std::shared_ptr<LevelSequence> m_levelSequence;
    float m_rotationAngle;

    static float constexpr m_depthTextureNearPlane = 0.1f;
    static float constexpr m_depthTextureFarPlane = 10.0f;

    bool testDepthTexture(levelDrawer::Adaptor inLevelDrawer);
};

#endif // AMAZING_LABYRINTH_MAZE_GRAPHICS_HPP