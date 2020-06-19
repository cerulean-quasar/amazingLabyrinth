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

#ifndef AMAZING_LABYRINTH_LEVEL_FINISH_HPP
#define AMAZING_LABYRINTH_LEVEL_FINISH_HPP

#include <cstdint>
#include <vector>
#include <list>
#include <chrono>
#include "../../graphics.hpp"
#include "../../mathGraphics.hpp"
#include "../../random.hpp"
#include "../../common.hpp"
#include "loadData.hpp"

namespace finisher {
    class LevelFinisher {
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
        LevelFinisher(std::shared_ptr<GameRequester> inGameRequester, glm::mat4 const &proj,
                    glm::mat4 const &view, float centerX, float centerY, float centerZ, float maxZ)
                : m_gameRequester{std::move(inGameRequester)},
                  m_maxZ(maxZ),
                  m_centerX(centerX),
                  m_centerY(centerY),
                  shouldUnveil(false),
                  finished(false)
        {
            auto wh = getWidthHeight(maxZ, proj, view);
            m_width = wh.first;
            m_height = wh.second;

            auto center = getCenter(proj, view, centerX, centerY, centerZ, maxZ);
            m_centerX = center.first;
            m_centerY = center.second;
        }

        void unveilNewLevel() {
            shouldUnveil = true;
            finished = false;
        }

        std::pair<float, float> getCenter(glm::mat4 const &proj, glm::mat4 const &view, float x, float y, float z, float atZ) {
            glm::vec4 locationScreen = proj * view * glm::vec4{x, y, z, 1.0f};
            glm::vec4 zVec = proj * view * glm::vec4{0.0f, 0.0f, atZ, 1.0f};
            locationScreen = glm::vec4{locationScreen.x * zVec.w, locationScreen.y * zVec.w,
                                       zVec.z * locationScreen.w, locationScreen.w * zVec.w};
            glm::vec4 locationWorld = glm::inverse(view) * glm::inverse(proj) * locationScreen;

            return std::make_pair(locationWorld.x/locationWorld.w, locationWorld.y/locationWorld.w);
        }

        bool isUnveiling() { return shouldUnveil; }

        bool isDone() { return finished; }

        virtual bool updateDrawObjects(DrawObjectTable &drawObjects, TextureMap &textures,
                                       bool &texturesUpdated) = 0;

        virtual ~LevelFinisher() = default;
    };
} // finisher


namespace manyQuadsCoverUp {
    class LevelFinisher : public finisher::LevelFinisher {
    private:
        static uint32_t constexpr totalNumberObjectsForSide = 5;
        static uint32_t constexpr totalNumberObjects =
                totalNumberObjectsForSide * totalNumberObjectsForSide;
        std::list<glm::vec3> translateVectors;

        // every timeThreshold, a new image appears, covering up the maze.
        static float constexpr timeThreshold = 0.05f;

        Random random;
        uint32_t totalNumberReturned;

        std::chrono::high_resolution_clock::time_point prevTime;
        std::vector<std::string> imagePaths;
    public:
        static char constexpr const *m_name = "manyQuadsCoverUp";
        bool updateDrawObjects(DrawObjectTable &drawObjects, TextureMap &textures,
                                       bool &texturesUpdated) override;

        LevelFinisher(std::shared_ptr<GameRequester> inGameRequester, std::shared_ptr<LevelConfigData> const &lcd,
                glm::mat4 const &proj, glm::mat4 const &view, float centerX, float centerY, float centerZ, float maxZ);

        ~LevelFinisher() override = default;
    };
} // namespace manyQuadCoverUp

namespace growingQuad {
    class LevelFinisher : public finisher::LevelFinisher {
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
        static char constexpr const *m_name = "growingQuad";
        bool updateDrawObjects(DrawObjectTable &drawObjects, TextureMap &textures,
                               bool &texturesUpdated) override;

        LevelFinisher(
            std::shared_ptr<GameRequester> inGameRequester,
            std::shared_ptr<LevelConfigData> const &lcd,
            glm::mat4 const &proj,
            glm::mat4 const &view,
            float centerX,
            float centerY,
            float centerZ,
            float maxZ);

        ~LevelFinisher() override = default;
    };
} // namespace growingQuad
#endif