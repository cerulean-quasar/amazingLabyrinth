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

#ifndef AMAZING_LABYRINTH_FIXED_MAZE_LEVEL_HPP
#define AMAZING_LABYRINTH_FIXED_MAZE_LEVEL_HPP

#include <chrono>
#include <functional>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../common.hpp"
#include "../basic/level.hpp"
#include "../../random.hpp"

#include "loadData.hpp"

namespace fixedMaze {
    class Level : public basic::Level {
    public:
        static char constexpr const *m_name = "fixedMaze";
        static float constexpr MODEL_MAXZ = 1.0f;

        glm::vec4 getBackgroundColor() override;

        bool updateData() override;

        bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) override;

        bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures,
                                      bool &texturesChanged) override;

        void start() override;

        void getLevelFinisherCenter(float &x, float &y, float &z) override;

        char const *name() override { return m_name; }

        std::vector<uint8_t> saveData(levelTracker::GameSaveData const &gsd,
                                      char const *saveLevelDataKey) override;

        Level(std::shared_ptr<GameRequester> inGameRequester,
                  std::shared_ptr<LevelConfigData> const &lcd,
                  std::shared_ptr<LevelSaveData> const &sd,
                  glm::mat4 const &proj, glm::mat4 const &view, float maxZ);

        void init();

        ~Level() override = default;

    private:
        float const m_speedLimit;

        std::chrono::high_resolution_clock::time_point m_prevTime;
        std::vector<Vertex> m_ballVertices;
        std::vector<uint32_t> m_ballIndices;
        std::string m_floorModel;
        std::string m_floorTexture;

        size_t m_rowWidth;
        size_t m_rowHeight;
        std::vector<float> m_depthMap;
        std::vector<glm::vec3> m_normalMap;
        std::shared_ptr<DrawObject> m_floor;
        float m_extraBounce;
        float m_minSpeedOnObjBounce;
        Random m_randomNbrs;

        size_t getXCell(float x);

        size_t getYCell(float y);

        float getZPos(float x, float y);

        float getZPos(float x, float y, float extend);

        float getRawDepth(size_t xcell, size_t ycell);

        float getRawDepth(float x, float y);

        glm::vec3 getParallelAcceleration();

        glm::vec3 getNormalAtPosition(float x, float y);

        glm::vec3 getNormalAtPosition(float x, float y, float extend, glm::vec3 const &velocity);

        glm::vec3 getNormalAtPosition(float x, float y, glm::vec3 const &velocity);

        glm::vec3 getRawNormalAtPosition(size_t xcell, size_t ycell);

        glm::vec3 getRawNormalAtPosition(float x, float y);

        void moveBall(float timeDiff);

        void findModelViewPort(
                std::vector<float> const &depthMap,
                std::vector<glm::vec3> const &normalMap,
                size_t rowWidth,
                glm::mat4 &trans,
                std::vector<float> &outDepthMap,
                std::vector<glm::vec3> &outNormalMap,
                size_t &outRowWidth);
    };
}
#endif // AMAZING_LABYRINTH_FIXED_MAZE_LEVEL_HPP
