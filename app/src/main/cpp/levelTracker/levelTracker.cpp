/**
 * Copyright 2022 Cerulean Quasar. All Rights Reserved.
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
#include <array>
#include <functional>
#include <boost/optional.hpp>
#include <vector>
#include <boost/none.hpp>

#include "../levels/basic/level.hpp"
#include "types.hpp"
#include "levelTracker.hpp"
#include "internals.hpp"

char constexpr const *PointXKey = "X";
char constexpr const *PointYKey = "Y";

void to_json(nlohmann::json &j, Point<uint32_t> const &val) {
    j[PointXKey] = val.x;
    j[PointYKey] = val.y;
}

void from_json(nlohmann::json const &j, Point<uint32_t> &val) {
    val.x = j[PointXKey].get<int>();
    val.y = j[PointYKey].get<int>();
}

void to_json(nlohmann::json &j, Point<float> const &val) {
    j[PointXKey] = val.x;
    j[PointYKey] = val.y;
}

void from_json(nlohmann::json const &j, Point<float> &val) {
    val.x = j[PointXKey].get<float>();
    val.y = j[PointYKey].get<float>();
}

namespace levelTracker {
    namespace DataVariables {
        char constexpr const *Levels = "Levels";
        char constexpr const *Name = "Name";
        char constexpr const *File = "File";
        char constexpr const *Components = "Components";
        char constexpr const *Starter = "Starter";
        char constexpr const *Level = "Level";
        char constexpr const *Finisher = "Finisher";
    }

    LevelMapTable &starterTable() {
        static LevelMapTable map;
        return map;
    }

    LevelMapTable &levelTable() {
        static LevelMapTable map;
        return map;
    }

    FinisherMapTable &finisherTable() {
        static FinisherMapTable map;
        return map;
    }

    void from_json(nlohmann::json const &j, LevelTableEntry &val) {
        val.levelName = j[DataVariables::Name].get<std::string>();
        val.fileName = j[DataVariables::File].get<std::string>();
    }

    struct Components {
        std::string starter;
        std::string level;
        std::string finisher;

        Components() = default;

        Components(Components &&other) noexcept
            : starter{std::move(other.starter)},
              level{std::move(other.level)},
              finisher{std::move(other.finisher)}
        {}
    };

    void from_json(nlohmann::json const &j, Components &val) {
        val.starter = j[DataVariables::Starter].get<std::string>();
        val.level = j[DataVariables::Level].get<std::string>();
        val.finisher = j[DataVariables::Finisher].get<std::string>();
    }

    void from_json(nlohmann::json const &j, GameSaveData &val) {
        val.version = j[DataVariables::GameSaveDataVersion].get<int>();
        val.screenSize = j[DataVariables::GameSaveDataScreenSize].get<Point<uint32_t>>();
        val.levelName = j[DataVariables::GameSaveDataLevelName].get<std::string>();
        val.needsStarter = j[DataVariables::GameSaveDataNeedsStarter].get<bool>();
    }

    void to_json(nlohmann::json &j, GameSaveData const &val) {
        j[DataVariables::GameSaveDataVersion] = val.version;
        j[DataVariables::GameSaveDataScreenSize] = val.screenSize;
        j[DataVariables::GameSaveDataLevelName] = val.levelName;
        j[DataVariables::GameSaveDataNeedsStarter] = val.needsStarter;
    }

    void saveGameData(
            std::shared_ptr<FileRequester> const &requester,
            uint32_t screenWidth,
            uint32_t screenHeight,
            std::string const &levelName,
            std::shared_ptr<basic::Level> const &level,
            bool needsStarter)
    {
        Point<uint32_t> screenSize{screenWidth, screenHeight};
        GameSaveData gsd(GameSaveDataVersionValue, screenSize, levelName, needsStarter);

        std::vector<uint8_t> vec;
        if (level) {
             vec = level->saveData(gsd, DataVariables::GameSaveDataLevel);
        } else {
            nlohmann::json j;
            to_json(j, gsd);
            vec = nlohmann::json::to_cbor(j);
        }

        std::ofstream saveDataStream(requester->getSaveDataFileName());

        if (!saveDataStream.fail()) {
            saveDataStream.write(reinterpret_cast<char const *>(vec.data()), vec.size());
            saveDataStream.close();
        }
    }

    void Loader::gotoNextLevel() {
        if (m_currentLevel == boost::none) {
            m_currentLevel.reset(0);
        } else {
            m_currentLevel.reset((m_currentLevel.get() + 1) % m_levelTable.size());
        }
    }

    LevelGroup Loader::getLevelGroupFcns(uint32_t screenWidth, uint32_t screenHeight) {
        nlohmann::json *pjsdLevel = nullptr;
        nlohmann::json jsdLevel;
        nlohmann::json jgb;
        bool needsLevelStarter = true;
        if (m_currentLevel == boost::none) {
            auto cborSaveData = getDataFromFile(m_gameRequester->getSaveDataFileName());
            if (cborSaveData.empty()) {
                m_currentLevel.reset(0);
            } else {
                jgb = nlohmann::json::from_cbor(cborSaveData);

                auto gb = jgb.get<GameSaveData>();

                m_currentLevel = getLevelNumber(gb.levelName);

                if (m_currentLevel == boost::none) {
                    m_currentLevel.reset(0);
                } else {
                    Point<uint32_t> screenSize{screenWidth, screenHeight};
                    if (screenSize == gb.screenSize) {
                        auto it = jgb.find(DataVariables::GameSaveDataLevel);
                        if (it != jgb.end()) {
                            jsdLevel = *it;
                            pjsdLevel = &jsdLevel;
                            needsLevelStarter = gb.needsStarter;
                        }
                    }
                }
            }
        }
        nlohmann::json j = nlohmann::json::from_cbor(getDataFromFile(m_gameRequester->getAssetStream(m_levelTable[m_currentLevel.get()].fileName)));
        Components components = j[DataVariables::Components].get<Components>();

        LevelGroup group;
        auto starterIt = starterTable().find(components.starter);
        if (starterIt == starterTable().end()) {
            throw std::runtime_error("Invalid level starter.");
        }

        if (needsLevelStarter) {
            group.getStarterFcn = starterIt->second(j[DataVariables::Starter], nullptr,
                                                    m_maxZLevelStarter);
        } else {
            group.getStarterFcn = GenerateLevelFcn(
                    [](levelDrawer::Adaptor) -> std::shared_ptr<basic::Level> {
                        return nullptr;
                    });
        }

        auto levelIt = levelTable().find(components.level);
        if (levelIt == levelTable().end()) {
            throw std::runtime_error("Invalid level");
        }

        group.getLevelFcn = levelIt->second(j[DataVariables::Level], pjsdLevel, m_maxZLevel);

        auto finisherIt = finisherTable().find(components.finisher);
        if (finisherIt == finisherTable().end()) {
            throw std::runtime_error("Invalid finisher");
        }

        group.getFinisherFcn = finisherIt->second(j[DataVariables::Finisher], m_maxZLevelFinisher);

        return group;
    }

    Loader::Loader(
        std::shared_ptr<GameRequester> inGameRequester)
        : m_gameRequester{std::move(inGameRequester)},
          m_currentLevel{boost::none}
    {
        auto streambuf = m_gameRequester->getLevelTableAssetStream();
        std::istream stream(streambuf.get());
        nlohmann::json j;
        stream >> j;
        m_levelTable = std::move(j[DataVariables::Levels].get<std::vector<LevelTableEntry>>());
    }
}
