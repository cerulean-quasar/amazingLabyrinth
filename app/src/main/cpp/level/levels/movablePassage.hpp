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

#ifndef AMAZING_LABYRINTH_MOVABLE_PASSAGE_HPP
#define AMAZING_LABYRINTH_MOVABLE_PASSAGE_HPP

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

struct MovablePassageSaveData : public LevelSaveData {
    static int constexpr m_movablePassageVersion = 1;
    MovablePassageSaveData() : LevelSaveData{m_movablePassageVersion} {}
};

class MovablePassage;
class Component {
    friend MovablePassage;
public:
    enum ComponentType {
        straight = 1,
        tjunction = 2,
        crossjunction = 3,
        turn = 4,
        uturn = 5
    };

    struct Placement {
        glm::vec3 m_position;
        float m_rotationAngle; /*radians */
        bool m_lockedIntoPlace;

        Placement(glm::vec3 const &position,
                float rotationAngle = 0.0f, /* radians */
                bool lockedIntoPlace = false)
            : m_position{position},
            m_rotationAngle{rotationAngle},
            m_lockedIntoPlace{lockedIntoPlace}
        {
        }
    };

    void add(glm::vec3 position) {
        m_placements.emplace_back(position);
    }

    Component(ComponentType inType)
        : m_componentType{inType}
    {
    }

    bool operator==(Component const &other) { return other.m_componentType == m_componentType; }
    bool operator<(Component const &other) { return m_componentType < other.m_componentType; }

private:
    ComponentType m_componentType;
    std::vector<Placement> m_placements;
};

class MovablePassage : public Level {
public:
    static float constexpr MODEL_MAXZ = 1.0f;
    glm::vec4 getBackgroundColor() override;
    bool updateData() override;
    bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) override;
    bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged) override;
    void start() override;
    void getLevelFinisherCenter(float &x, float &y) override;
    SaveLevelDataFcn getSaveLevelDataFcn() override;

    MovablePassage(
        std::shared_ptr<GameRequester> inGameRequester,
        std::shared_ptr<MovablePassageSaveData> sd,
        float width, float height, float maxZ)
        : Level(inGameRequester, width, height, maxZ, true, 1/50.0f, false),
        m_components{},
        m_nbrComponents{0}
    {}

    void initSetBallInfo(
        std::string const &ballModel,
        std::string const &ballTexture)
    {
        m_ballModel = ballModel;
        m_ballTextureName = ballTexture;
    }

    void initSetFloorInfo(
        uint32_t nbrTilesX,
        uint32_t nbrTilesY,
        std::string const &floorTexture)
    {
        m_floorTexture = floorTexture;
    }

    void initAddType(
        Component::ComponentType inComponentType,
        uint32_t nbrComponents,
        std::string &texture)
    {
        auto insertResult = m_components.emplace(inComponentType);
        for (uint32_t i = 0; i < nbrComponents; i++) {
            insertResult.first->add(glm::vec3{0.0f, 0.0f, 0.0f});
            m_nbrComponents++;
        }
    }

    void initDone();

    ~MovablePassage() override = default;

private:
    static uint32_t constexpr m_nbrTileRowsForStart = 2;
    static uint32_t constexpr m_nbrTileRowsForEnd = 2;
    std::chrono::high_resolution_clock::time_point m_prevTime;
    std::string m_ballModel;
    std::string m_ballTextureName;
    std::string m_floorTexture;
    std::set<Component> m_components;
    Component m_fixedComponents;
    float m_tileSize;
    uint32_t m_nbrComponents;
    uint32_t m_nbrTilesX;
    uint32_t m_nbrTilesY;
};
#endif /* AMAZING_LABYRINTH_MOVABLE_PASSAGE_HPP */
