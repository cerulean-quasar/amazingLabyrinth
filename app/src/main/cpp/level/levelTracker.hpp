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
#include <boost/optional.hpp>
#include "levelFinish.hpp"
#include "level.hpp"
#include "levelStarter.hpp"
#include "../common.hpp"

typedef std::shared_ptr<Level> (*getLevel)(std::shared_ptr<GameRequester> inGameRequester,
        boost::optional<GameBundle> const &saveData, float width,
        float height, float maxZ);
typedef std::shared_ptr<LevelFinish> (*getLevelFinisher)(
        std::shared_ptr<GameRequester> inGameRequester,
        float centerX, float centerY, float maxZ);
typedef std::shared_ptr<LevelStarter> (*getLevelStarter)(
        std::shared_ptr<GameRequester> inGameRequester, float width, float height, float maxZ);

struct LevelEntry {
    getLevelStarter starter;
    getLevel level;
    getLevelFinisher finisher;
    std::string levelName;  // must be unique for all levels.
    std::string levelDescription;
};

typedef std::array<LevelEntry, 7> LevelTable;

class LevelTracker {
public:
    static float constexpr m_maxZLevel = -1.0f;
    static float constexpr m_maxZLevelStarter = 0.0f;
    static float constexpr m_maxZLevelFinisher = 1.0f;

    std::pair<float, float> getWidthHeight(float maxZ, glm::mat4 const &proj, glm::mat4 const &view);
    std::shared_ptr<LevelStarter> getLevelStarter();
    std::shared_ptr<Level> getLevel(boost::optional<GameBundle> const &saveData);
    std::shared_ptr<LevelFinish> getLevelFinisher(float centerX, float centerY,
                                                  glm::mat4 const &proj, glm::mat4 const &view);
    void gotoNextLevel();
    static std::vector<std::string> getLevelDescriptions();
    static bool validLevel(uint32_t level) { return level < s_levelTable.size(); }

    static GameBundleSchema getSaveGameSchema(GameBundle const &initialBundle) {
        return GameBundleSchema{};
    }

    GameBundle saveLevelData() {
        GameBundle saveData;
        saveData.insert(std::make_pair(KeyLevelIdentifier,
                GameBundleValue(s_levelTable[m_currentLevel].levelName)));
        return saveData;
    }

    LevelTracker(
            std::shared_ptr<GameRequester> inGameRequester,
            boost::optional<GameBundle> const &saveData,
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

        if (saveData) {
            auto levelNameIt = saveData->find(KeyLevelIdentifier);
            if (levelNameIt != saveData->end()) {
                for (uint32_t i = 0; i < s_levelTable.size(); i++) {
                    if (boost::get<std::string>(levelNameIt->second) == s_levelTable[i].levelName) {
                        m_currentLevel = i;
                        break;
                    }
                }
            }
        }
    }

private:
    std::shared_ptr<GameRequester> m_gameRequester;
    uint32_t m_currentLevel;
    float m_widthLevel;
    float m_heightLevel;
    float m_widthLevelStarter;
    float m_heightLevelStarter;
    static LevelTable s_levelTable;
};
#endif