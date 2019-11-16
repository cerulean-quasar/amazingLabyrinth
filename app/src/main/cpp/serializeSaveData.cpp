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

#include <string>
#include <memory>
#include <functional>
#include <json.hpp>

#include "serializeSaveDataInternals.hpp"
#include "serializeSaveData.hpp"
#include "saveData.hpp"
#include "level/levelTracker.hpp"

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

char constexpr const *LevelVersion = "LevelVersion";
void to_json(nlohmann::json &j, LevelSaveData const &val) {
    j[LevelVersion] = val.m_version;
}

void from_json(nlohmann::json const &j, LevelSaveData &val) {
    val.m_version = j[LevelVersion].get<int>();
}

using RestoreLevelFromDataFcn = std::function<RestoreData(nlohmann::json const *, std::string const&)>;

struct LevelRestoreTableEntry {
    template <typename LevelSaveDataType>
    LevelRestoreTableEntry(LevelGroup (*getLevelGroupFcn)(std::shared_ptr<LevelSaveDataType> const &),
                           std::string levelName_, std::string levelDescription_)
            : restoreFcn(
            [getLevelGroupFcn] (nlohmann::json const *j, std::string const& levelName) -> RestoreData {
                std::shared_ptr<LevelSaveDataType> levelSaveData;
                if (j != nullptr) {
                    auto jobj = j->find(GameSaveDataLevel);
                    if (jobj != j->end()) {
                        nlohmann::json j2 = jobj.value();
                        LevelSaveDataType lsd = j2.get<LevelSaveDataType>();
                        levelSaveData = std::make_shared<LevelSaveDataType>(std::move(lsd));
                    }
                }

                return { levelName, getLevelGroupFcn(levelSaveData)};
            }),
              levelName(std::move(levelName_)),
              levelDescription(std::move(levelDescription_))
    {}

    // TODO: override should not be necessary after full implementation completed
    template <>
    LevelRestoreTableEntry(LevelGroup (*getLevelGroupFcn)(std::shared_ptr<void> const &),
                           std::string levelName_, std::string levelDescription_)
            : restoreFcn(
            [getLevelGroupFcn] (nlohmann::json const *j, std::string const& levelName) -> RestoreData {
                std::shared_ptr<void> levelSaveData;
                return { levelName, getLevelGroupFcn(levelSaveData)};
            }),
              levelName(std::move(levelName_)),
              levelDescription(std::move(levelDescription_))
    {}

    RestoreLevelFromDataFcn restoreFcn;
    std::string levelName;
    std::string levelDescription;
};

using LevelRestoreTable = std::array<LevelRestoreTableEntry, 8>;

LevelRestoreTable const &getRestoreLevelTable() {
    static LevelRestoreTable const levelRestoreTable{
            // levelName must be unique for all levels.
            LevelRestoreTableEntry{LevelTracker::getLevelGroupBeginning, LevelTracker::beginning, "The beginning"},
            LevelRestoreTableEntry{LevelTracker::getLevelGroupLonelyPlanet, LevelTracker::lonelyPlanet, "The lonely planet"},
            LevelRestoreTableEntry{LevelTracker::getLevelGroupPufferFish, LevelTracker::pufferFish, "The puffer fish"},
            LevelRestoreTableEntry{LevelTracker::getLevelGroupRolarBear, LevelTracker::rolarBear, "The rolar bear"},
            LevelRestoreTableEntry{LevelTracker::getLevelGroupBee1, LevelTracker::bee1, "The roller bee"},
            LevelRestoreTableEntry{LevelTracker::getLevelGroupBee2, LevelTracker::bee2, "The search"},
            LevelRestoreTableEntry{LevelTracker::getLevelGroupCat, LevelTracker::cat, "The cat"},
            LevelRestoreTableEntry{LevelTracker::getLevelGroupBunny, LevelTracker::bunny, "The bunny"}};
    return levelRestoreTable;
}

using LevelMap = std::map<std::string, RestoreLevelFromDataFcn>;

LevelTable const &getLevelTable() {
    struct Make {
        LevelTable levelTable;
        Make() {
            auto& restoreTable = getRestoreLevelTable();
            levelTable.reserve(restoreTable.size());
            for (auto const &levelRestoreEntry : restoreTable) {
                levelTable.push_back(LevelEntry{levelRestoreEntry.levelName,
                                      levelRestoreEntry.levelDescription,
                                      levelRestoreEntry.restoreFcn(nullptr, levelRestoreEntry.levelName).levelGroupFcns});
            }
        }
    };
    static Make make;
    return make.levelTable;
}

LevelMap const &getLevelMap() {
    struct Make {
        LevelMap levelMap;

        Make() {
            for (auto const &levelRestoreEntry : getRestoreLevelTable()) {
                levelMap.insert(
                        std::make_pair(levelRestoreEntry.levelName, levelRestoreEntry.restoreFcn));
            }
        }
    };
    static Make make;
    return make.levelMap;
}

RestoreData createLevelFromRestore() {
    return getRestoreLevelTable()[0].restoreFcn(nullptr, getRestoreLevelTable()[0].levelName);
}

RestoreData createLevelFromRestore(std::vector<uint8_t> const &saveData, Point<uint32_t> const &screenSize) {
    try {
        nlohmann::json j = nlohmann::json::from_cbor(saveData);

        auto gb = std::make_shared<GameSaveData>(j[GameSaveDataVersion].get<int>(),
            j[GameSaveDataScreenSize].get<Point<uint32_t>>(),
            j[GameSaveDataLevelName].get<std::string>());

        auto jobj = j.find(GameSaveDataLevel);
        bool ignoreLevelData = false;
        if (screenSize != gb->screenSize) {
            ignoreLevelData = true;
        }

        auto it = getLevelMap().find(gb->levelName);
        if (it != getLevelMap().end()) {
            if (ignoreLevelData) {
                return it->second(nullptr, it->first);
            } else {
                return it->second(&j, it->first);
            }
        } else {
            return createLevelFromRestore();
        }
    } catch (...) {
    }

    return createLevelFromRestore();
}
