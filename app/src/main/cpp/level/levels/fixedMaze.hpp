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
    glm::vec4 getBackgroundColor() override {
        return glm::vec4{0.0f, 0.0f, 0.0f, 0.0f};
    };
    void updateAcceleration(float x, float y, float z) override {
        ball.acceleration = glm::vec3{x, y, -z};
    }
    bool updateData() override {
        // need to postpone getting the depth texture till here because the functionality is
        // not available earlier on.
        // todo: fix this!
        if (!initialized) {
            init();
            initialized = true;
            return true;
        }
        return false;
    }

    bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) override {
        return true;
    }

    bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged) override {
        texturesChanged = false;
        if (objs.empty() && textures.empty() && initialized) {
            texturesChanged = true;
            objs.emplace_back(m_testObj, nullptr);
            //auto obj = m_worldMap.begin();
            //obj->first->texture = m_testObj->texture;
            //objs.emplace_back(obj->first, nullptr);
            textures.insert(
                    std::make_pair(std::make_shared<TextureDescriptionDummy>(m_gameRequester), m_testTexture));
        }
        return true;
    }

    void start() override {}

    void getLevelFinisherCenter(float &x, float &y) override {
        x = 0.0f;
        y = 0.0f;
    }

    SaveLevelDataFcn getSaveLevelDataFcn() override;

    FixedMaze(std::shared_ptr<GameRequester> inGameRequester, float width, float height, float maxZ)
            : Level{inGameRequester, width, height, maxZ},
              initialized{false}
    {
    }

    FixedMaze(std::shared_ptr<GameRequester> inGameRequester,
              std::shared_ptr<FixedMazeSaveData> sd,
              float width, float height, float maxZ)
            : Level{inGameRequester, width, height, maxZ},
            initialized{false}
    {
    }

    void init()
    {
        auto worldObj = std::make_shared<DrawObject>();
        loadModel(m_gameRequester->getAssetStream("models/mountainLandscape.obj"), worldObj->vertices, worldObj->indices);
        worldObj->modelMatrices.push_back(
                glm::translate(glm::mat4(1.0f), glm::vec3{0.0f, 0.0f, m_maxZ}) *
                glm::scale(glm::mat4(1.0f), glm::vec3{m_width/MODEL_WIDTH, m_height/MODEL_HEIGHT, 1.0f}) *
                glm::mat4_cast(glm::angleAxis(3.1415926f/2.0f, glm::vec3(1.0f, 0.0f, 0.0f))));
        worldObj->texture = nullptr;
        m_worldMap.emplace_back(worldObj, nullptr);

        m_testTexture = m_gameRequester->getDepthTexture(m_worldMap, m_width, m_height);
        m_testObj = std::make_shared<DrawObject>();
        m_testObj->texture = std::make_shared<TextureDescriptionDummy>(m_gameRequester);
        getQuad(m_testObj->vertices, m_testObj->indices);
        m_testObj->modelMatrices.push_back(glm::translate(glm::mat4(1.0f), glm::vec3{0.0f, 0.0f, m_maxZ}) *
            glm::scale(glm::mat4(1.0f), glm::vec3{m_width/2.0f, m_height/2.0f, 1.0f}));
    }

    ~FixedMaze() override = default;

private:
    bool initialized;
    // data on where the ball is, how fast it is moving, etc.
    struct {
        glm::vec3 prevPosition;
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec3 acceleration;
        glm::quat totalRotated;
    } ball;
    DrawObjectTable m_worldMap;
    std::shared_ptr<DrawObject> m_testObj;
    std::shared_ptr<TextureData> m_testTexture;
};
#endif /* AMAZING_LABYRINTH_FIXED_MAZE_HPP */
