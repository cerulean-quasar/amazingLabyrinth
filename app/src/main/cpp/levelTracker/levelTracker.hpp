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
#ifndef AMAZING_LABYRINTH_LEVEL_TRACKER_HPP
#define AMAZING_LABYRINTH_LEVEL_TRACKER_HPP
#include <array>
#include <fstream>
#include <functional>
#include <boost/optional.hpp>
#include "levelFinish.hpp"
#include "basic/level.hpp"
#include "starter/level.hpp"
#include "../common.hpp"
#include "../saveData.hpp"
#include "../serializeSaveData.hpp"
#include "../mathGraphics.hpp"

namespace levelTracker {
    struct LevelGroup {
        GetStarterFcn getStarterFcn;
        GetLevelFcn getLevelFcn;
        GetFinisherFcn getFinisherFcn;
    };

    struct LevelTableEntry{
        std::string levelName;
        std::string fileName;

        LevelTableEntry() = default;
        LevelTableEntry(LevelTableEntry &&other)
                : levelName{std::move(other.levelName)},
                  fileName{std::move(other.fileName)}
        {}
    };

    class Loader {
    public:
        static float constexpr m_maxZLevel = -1.0f;
        static float constexpr m_maxZLevelStarter = 0.0f;
        static float constexpr m_maxZLevelFinisher = 0.5f;

        void gotoNextLevel();

        static bool validLevel(uint32_t level) { return level < getLevelTable().size(); }

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

        static std::string getLevelName(uint32_t level) {
            return getLevelTable()[level].levelName;
        }

        static LevelGroup getLevelGroupFcns(uint32_t level) {
            if (!validLevel(level)) {
                level = 0;
            }
            return getLevelTable()[level].levelGroup;
        }

        LevelGroup getLevelGroupFcns();

        Loader(std::shared_ptr<GameRequester> inGameRequester);

    private:
        std::vector<LevelTableEntry> m_levelTable;

        std::vector<uint8_t> getDataFromFile(std::string const &filename) {
            std::ifstream stream(filename);
            if (stream.good()) {
                stream.seekg(0, stream.end);
                size_t i = static_cast<size_t >(stream.tellg());
                stream.seekg(0, stream.beg);
                std::vector<uint8_t> vec;
                vec.resize(i);
                stream.read(reinterpret_cast<char *>(vec.data()), vec.size());

                if (!stream.fail()) {
                    return vec;
                }
            }
        }

        // if there is no levelSaveData, then we are at the beginning of the level and need a level starter
        // otherwise we are in the level and the level starter function should return nullptr.
        static GetStarterFcn
        getStarterFcn(bool required, std::vector<std::string> &&levelBeginStrings) {
            if (!required) {
                return GetStarterFcn([](LevelTracker &,
                                        glm::mat4 const &,
                                        glm::mat4 const &) -> std::shared_ptr<LevelStarter> {
                    return nullptr;
                });
            }

            return GetStarterFcn([levelBeginStrings{move(levelBeginStrings)}](LevelTracker &tracker,
                                                                              glm::mat4 const &proj,
                                                                              glm::mat4 const &view)
                                         -> std::shared_ptr<LevelStarter> {
                auto levelStarter = tracker.getStarter(proj, view);
                for (auto levelBeginString : levelBeginStrings) {
                    levelStarter->addTextString(levelBeginString);
                }
                return levelStarter;
            });
        }

        std::shared_ptr<LevelStarter> getStarter(glm::mat4 const &proj, glm::mat4 const &view) {
            auto wh = getWidthHeight(m_maxZLevelStarter, proj, view);
            return std::make_shared<LevelStarter>(m_gameRequester, wh.first, wh.second,
                                                  m_maxZLevelStarter);
        }

        template<typename LevelType, typename LevelDataType>
        std::shared_ptr<LevelType>
        getLevel(LevelDataType const &levelSaveData, glm::mat4 const &proj, glm::mat4 const &view) {
            auto wh = getWidthHeight(m_maxZLevel, proj, view);
            auto level = std::make_shared<LevelType>(
                    m_gameRequester, levelSaveData, wh.first, wh.second, m_maxZLevel);
            return level;
        }

        template<typename FinisherType>
        std::shared_ptr<FinisherType>
        getFinisher(float centerX, float centerY, glm::mat4 const &proj, glm::mat4 const &view) {
            glm::vec4 locationScreen = proj * view * glm::vec4{centerX, centerY, m_maxZLevel, 1.0f};
            glm::vec4 z = proj * view * glm::vec4{0.0f, 0.0f, m_maxZLevelFinisher, 1.0f};
            locationScreen = glm::vec4{locationScreen.x * z.w, locationScreen.y * z.w,
                                       z.z * locationScreen.w, locationScreen.w * z.w};
            glm::vec4 locationWorld = glm::inverse(view) * glm::inverse(proj) * locationScreen;

            auto wh = getWidthHeight(m_maxZLevelFinisher, proj, view);
            return std::make_shared<FinisherType>(m_gameRequester, wh.first, wh.second,
                                                  locationWorld.x / locationWorld.w,
                                                  locationWorld.y / locationWorld.w,
                                                  m_maxZLevelFinisher);
        }

        std::shared_ptr<GameRequester> m_gameRequester;
        uint32_t m_currentLevel;
    };
}
#endif
