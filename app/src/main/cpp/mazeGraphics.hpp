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
#ifndef AMAZING_LABYRINTH_MAZE_GRAPHICS_HPP
#define AMAZING_LABYRINTH_MAZE_GRAPHICS_HPP

#include "level/levelTracker.hpp"
#include "common.hpp"

class LevelSequence {
public:
    glm::mat4 projectionMatrix() { return m_proj; }
    glm::mat4 viewMatrix() { return m_view; }
    glm::vec3 lightingSource() { return m_lightingSource; }
    glm::mat4 viewLightSource() { return m_viewLightingSource; }

    GameBundle saveLevelData() {
        GameBundle saveData = m_levelTracker.saveLevelData();
        saveData.insert(std::make_pair(KeyLevelIsAtStart, GameBundleValue(true)));
        saveData.insert(std::make_pair(KeyVersionIdentifier, GameBundleValue(1)));
        return saveData;
    }

protected:
    glm::mat4 m_proj;
    glm::mat4 m_view;
    glm::mat4 m_viewLightingSource;
    glm::vec3 m_lightingSource;
    LevelTracker m_levelTracker;

    LevelSequence(std::shared_ptr<GameRequester> inRequester,
            boost::optional<GameBundle> const &bundle,
            uint32_t surfaceWidth,
            uint32_t surfaceHeight)
            : m_proj{},
              m_view{},
              m_viewLightingSource{},
              m_lightingSource{},
              m_levelTracker{inRequester, bundle, getPerspectiveMatrix(surfaceWidth, surfaceHeight),
                             getViewMatrix()}
    {
        setView();
        updatePerspectiveMatrix(surfaceWidth, surfaceHeight);
        setLightingSource();
        setViewLightingSource();
    }

    virtual void setView();
    virtual void updatePerspectiveMatrix(uint32_t surfaceWidth, uint32_t surfaceHeight);
    virtual void setLightingSource();
    virtual void setViewLightingSource();
private:
    glm::mat4 getPerspectiveMatrix(uint32_t surfaceWidth, uint32_t surfaceHeight) {
        return glm::perspective(glm::radians(45.0f), surfaceWidth / (float) surfaceHeight,
                0.1f, 100.0f);
    }

    glm::mat4 getViewMatrix() {
        return glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
                glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
};

class Graphics {
public:
    virtual void initThread()=0;

    virtual void updateAcceleration(float x, float y, float z)=0;

    virtual void drawFrame()=0;

    virtual bool updateData()=0;

    virtual void recreateSwapChain(uint32_t width, uint32_t height)=0;

    virtual GraphicsDescription graphicsDescription() = 0;

    virtual GameBundle saveLevelData() = 0;

    virtual void cleanupThread()=0;

    explicit Graphics(std::shared_ptr<GameRequester> inRequester)
        : m_gameRequester{std::move(inRequester)}
    {}

    virtual ~Graphics() = default;

protected:
    std::shared_ptr<GameRequester> m_gameRequester;
};
#endif // AMAZING_LABYRINTH_MAZE_GRAPHICS_HPP