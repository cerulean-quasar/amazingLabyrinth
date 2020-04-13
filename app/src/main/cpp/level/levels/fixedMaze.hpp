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
    static float constexpr MODEL_SIZE = 2.0f;
    static float constexpr MODEL_MAXZ = 1.0f;
    static float constexpr MODEL_BALL_SIZE = 2.0f;
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
    static float constexpr m_floatErrorAmount = 0.0001f;
    static float constexpr m_speedLimit = 2.0f;
    static float constexpr m_lengthToSmallToNormalize = 0.001f;

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
    size_t m_rowWidth;
    size_t m_rowHeight;
    std::vector<float> m_depthMap;
    std::vector<glm::vec3> m_normalMap;
    DrawObjectTable m_worldMap;
    std::shared_ptr<DrawObject> m_testObj;
    std::shared_ptr<TextureData> m_testTexture;
    std::string m_ballTextureName;
    bool m_bounce;
    float m_extraBounce;
    float m_minSpeedOnObjBounce;
    Random randomNbrs;

    void loadModels();
    void init();
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
    void ballOutOfBounds(glm::vec3 &pos);
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
#endif /* AMAZING_LABYRINTH_FIXED_MAZE_HPP */
