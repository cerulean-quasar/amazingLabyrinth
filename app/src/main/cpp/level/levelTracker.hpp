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
#ifndef AMAZING_LABYRINTH_LEVEL_TRACKER_HPP
#define AMAZING_LABYRINTH_LEVEL_TRACKER_HPP
#include <array>
#include <functional>
#include <boost/optional.hpp>
#include "levelFinish.hpp"
#include "level.hpp"
#include "levelStarter.hpp"
#include "../common.hpp"
#include "../saveData.hpp"
#include "../serializeSaveData.hpp"
#include "levels/maze.hpp"
#include "levels/openAreaLevel.hpp"
#include "levels/mazeCollect.hpp"
#include "levels/avoidVortexLevel.hpp"
#include "levels/movingQuadsLevel.hpp"
#include "levels/mazeAvoid.hpp"

class LevelTracker {
public:
    static float constexpr m_maxZLevel = -1.0f;
    static float constexpr m_maxZLevelStarter = 0.0f;
    static float constexpr m_maxZLevelFinisher = 0.5f;

    // the following strings must be unique with respect to each other
    static char constexpr const *beginning = "beginning";
    static char constexpr const *lonelyPlanet = "lonelyPlanet";
    static char constexpr const *pufferFish = "pufferFish";
    static char constexpr const *rolarBear = "rolarBear";
    static char constexpr const *bee1 = "bee1";
    static char constexpr const *bee2 = "bee2";
    static char constexpr const *cat = "cat";
    static char constexpr const *bunny = "bunny";

    std::pair<float, float> getWidthHeight(float maxZ, glm::mat4 const &proj, glm::mat4 const &view);
    void gotoNextLevel();
    static std::vector<std::string> getLevelDescriptions();
    static bool validLevel(uint32_t level) { return level < getLevelTable().size(); }

    static LevelGroup getLevelGroupBeginning(std::shared_ptr<OpenAreaLevelSaveData> const &levelSaveData);
    static LevelGroup getLevelGroupLonelyPlanet(std::shared_ptr<AvoidVortexLevelSaveData> const &levelSaveData);
    static LevelGroup getLevelGroupPufferFish(std::shared_ptr<MazeSaveData> const &levelSaveData);
    static LevelGroup getLevelGroupRolarBear(std::shared_ptr<MovingQuadsLevelSaveData> const &levelSaveData);
    static LevelGroup getLevelGroupBee1(std::shared_ptr<MazeSaveData> const &levelSaveData);
    static LevelGroup getLevelGroupBee2(std::shared_ptr<MazeSaveData> const &levelSaveData);
    static LevelGroup getLevelGroupCat(std::shared_ptr<MazeCollectSaveData> const &levelSaveData);
    static LevelGroup getLevelGroupBunny(std::shared_ptr<MazeAvoidSaveData> const &levelBundle);

    void setLevel(std::string const &levelName) {
        size_t i = 0;
        for (auto entry : getLevelTable()) {
            if (levelName == entry.levelName) {
                setLevel(i);
                return;
            }
            i++;
        }
    }

    void setLevel(size_t level) {
        if (level < getLevelTable().size()) {
            m_currentLevel = level;
        }
    }

    std::string getLevelName() {
        return getLevelTable()[m_currentLevel].levelName;
    }

    LevelGroup getLevelGroupFcns() {
        return getLevelTable()[m_currentLevel].levelGroup;
    }

    LevelTracker(
            std::shared_ptr<GameRequester> inGameRequester,
            glm::mat4 const &proj,
            glm::mat4 const &view)
            : m_gameRequester{std::move(inGameRequester)},
              m_currentLevel{}
    {
        auto wh = getWidthHeight(m_maxZLevel, proj, view);
        m_widthLevel = wh.first;
        m_heightLevel = wh.second;

        wh = getWidthHeight(m_maxZLevelStarter, proj, view);
        m_widthLevelStarter = wh.first;
        m_heightLevelStarter = wh.second;

        wh = getWidthHeight(m_maxZLevelFinisher, proj, view);
        m_widthLevelFinisher = wh.first;
        m_heightLevelFinisher = wh.second;
    }

private:
    // if there is no levelSaveData, then we are at the beginning of the level and need a level starter
    // otherwise we are in the level and the level starter function should return nullptr.
    static GetStarterFcn getStarterFcn(bool required, std::vector<std::string> &&levelBeginStrings) {
        if (!required) {
            return GetStarterFcn([] (LevelTracker &tracker) {
                return nullptr;
            });
        }

        return GetStarterFcn( [levelBeginStrings{move(levelBeginStrings)}] (LevelTracker &tracker) {
            auto levelStarter = tracker.getStarter();
            for (auto levelBeginString : levelBeginStrings) {
                levelStarter->addTextString(levelBeginString);
            }
            return levelStarter;
        });
    }

    std::shared_ptr<LevelStarter> getStarter() {
        return std::make_shared<LevelStarter>(m_gameRequester, m_widthLevelStarter,
                                              m_heightLevelStarter, m_maxZLevelStarter);
    }

    template <typename LevelType, typename LevelDataType>
    std::shared_ptr<LevelType> getLevel(LevelDataType const &levelSaveData) {
        return std::make_shared<LevelType>(
                m_gameRequester, levelSaveData, m_widthLevel, m_heightLevel, m_maxZLevel);
    }

    template <typename FinisherType>
    std::shared_ptr<FinisherType> getFinisher(float centerX, float centerY, glm::mat4 const &proj, glm::mat4 const &view) {
        glm::vec4 locationScreen = proj * view * glm::vec4{centerX, centerY, m_maxZLevel, 1.0f};
        glm::vec4 z = proj * view * glm::vec4{0.0f, 0.0f, m_maxZLevelFinisher, 1.0f};
        locationScreen = glm::vec4{locationScreen.x * z.w, locationScreen.y * z.w,
                                   z.z * locationScreen.w, locationScreen.w * z.w};
        glm::vec4 locationWorld = glm::inverse(view) * glm::inverse(proj) * locationScreen;

        return std::make_shared<FinisherType>(m_gameRequester, m_widthLevelFinisher, m_heightLevelFinisher,
                locationWorld.x/locationWorld.w, locationWorld.y/locationWorld.w, m_maxZLevelFinisher);
    }

    std::shared_ptr<GameRequester> m_gameRequester;
    uint32_t m_currentLevel;
    float m_widthLevel;
    float m_heightLevel;
    float m_widthLevelStarter;
    float m_heightLevelStarter;
    float m_widthLevelFinisher;
    float m_heightLevelFinisher;
};
#endif
