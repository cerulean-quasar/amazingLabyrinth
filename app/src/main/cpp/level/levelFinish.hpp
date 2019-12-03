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

#ifndef AMAZING_LABYRINTH_LEVEL_FINISH_HPP
#define AMAZING_LABYRINTH_LEVEL_FINISH_HPP

#include <cstdint>
#include <vector>
#include <list>
#include <chrono>
#include "../graphics.hpp"
#include "../random.hpp"

class LevelFinish {
protected:
    std::shared_ptr<GameRequester> m_gameRequester;
    float m_maxZ;
    float m_width;
    float m_height;
    float m_centerX;
    float m_centerY;
    bool shouldUnveil;
    bool finished;
public:
    LevelFinish(std::shared_ptr<GameRequester> inGameRequester, float width, float height,
            float centerX, float centerY, float maxZ)
        : m_gameRequester{std::move(inGameRequester)},
        m_maxZ(maxZ),
        m_width(width),
        m_height(height),
        m_centerX(centerX),
        m_centerY(centerY),
        shouldUnveil(false),
        finished(false) { }
    void unveilNewLevel() { shouldUnveil = true; finished = false; }
    bool isUnveiling() { return shouldUnveil; }
    bool isDone() { return finished; }

    virtual bool updateDrawObjects(DrawObjectTable &drawObjects, TextureMap &textures, bool &texturesUpdated) = 0;
    virtual ~LevelFinish() = default;
};

class ManyQuadCoverUpLevelFinish : public LevelFinish {
private:
    static uint32_t constexpr totalNumberObjectsForSide = 5;
    static uint32_t constexpr totalNumberObjects = totalNumberObjectsForSide * totalNumberObjectsForSide;
    std::list<glm::vec3> translateVectors;

    // every timeThreshold, a new image appears, covering up the maze.
    static float constexpr timeThreshold = 0.05f;

    Random random;
    uint32_t totalNumberReturned;

    std::chrono::high_resolution_clock::time_point prevTime;
    std::vector<std::string> imagePaths;
public:
    virtual bool updateDrawObjects(DrawObjectTable &drawObjects, TextureMap &textures, bool &texturesUpdated);
    void initAddTexture(std::string const &inImagePath) {
        imagePaths.push_back(inImagePath);
    }
    ManyQuadCoverUpLevelFinish(std::shared_ptr<GameRequester> inGameRequester, float width, float height,
            float centerX, float centerY, float maxZ);
};

class GrowingQuadLevelFinish : public LevelFinish {
private:
    std::chrono::high_resolution_clock::time_point prevTime;
    float const timeThreshold = 0.1f; // seconds
    float const totalTime = 2.0f; // seconds
    float const finalSize;
    float const minSize;

    float timeSoFar;
    std::string imagePath;
    glm::vec3 scaleVector;

    glm::vec3 transVector;
public:
    virtual bool updateDrawObjects(DrawObjectTable &drawObjects, TextureMap &textures, bool &texturesUpdated);
    void initTexture(std::string const &inImagePath) {
        imagePath = inImagePath;
    }
    GrowingQuadLevelFinish(std::shared_ptr<GameRequester> inGameRequester, float width, float height,
            float x, float y, float maxZ);
};
#endif