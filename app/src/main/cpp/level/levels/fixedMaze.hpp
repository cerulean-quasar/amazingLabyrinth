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

#ifndef AMAZING_LABYRINTH_FIXED_MAZE_HPP
#define AMAZING_LABYRINTH_FIXED_MAZE_HPP

#include <chrono>
#include <functional>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../common.hpp"
#include "../../saveData.hpp"
#include "../level.hpp"

struct FixedMazeSaveData : public LevelSaveData {
    static int constexpr m_fixedMazeVersion = 1;
    FixedMazeSaveData() : LevelSaveData{m_fixedMazeVersion} {}
};

class FixedMaze : public Level {
public:
    static float constexpr MODEL_WIDTH = 2.0f;
    static float constexpr MODEL_HEIGHT = 2.0f;
    static float constexpr MODEL_MAXZ = 2.0f;
    glm::vec4 getBackgroundColor() override;
    void updateAcceleration(float x, float y, float z) override;
    bool updateData() override;
    bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) override;
    bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged) override;
    void start() override;
    void getLevelFinisherCenter(float &x, float &y) override;
    SaveLevelDataFcn getSaveLevelDataFcn() override;

    FixedMaze(std::shared_ptr<GameRequester> inGameRequester, float width, float height, float maxZ);

    FixedMaze(std::shared_ptr<GameRequester> inGameRequester,
              std::shared_ptr<FixedMazeSaveData> sd,
              float width, float height, float maxZ);

    void initSetBallTexture(std::string const &texture) { m_ballTextureName = texture; }

    ~FixedMaze() override = default;

private:
    static float constexpr m_viscosity = 0.005f;

    std::chrono::high_resolution_clock::time_point m_prevTime;
    std::vector<Vertex> m_ballVertices;
    std::vector<uint32_t> m_ballIndices;
    //bool m_initialized;
    // data on where the ball is, how fast it is moving, etc.
    struct {
        glm::vec3 prevPosition;
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec3 acceleration;
        glm::quat totalRotated;
    } m_ball;

    float m_scaleBall;
    uint32_t m_rowWidth;
    uint32_t m_rowHeight;
    std::vector<float> m_depthMap;
    DrawObjectTable m_worldMap;
    std::shared_ptr<DrawObject> m_testObj;
    std::shared_ptr<TextureData> m_testTexture;
    std::string m_ballTextureName;

    void loadModels();
    void init();
    void setBallZPos();
};
#endif /* AMAZING_LABYRINTH_FIXED_MAZE_HPP */
