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
#ifndef AMAZING_LABYRINTH_LEVEL_TRACKER_INTERNALS_HPP
#define AMAZING_LABYRINTH_LEVEL_TRACKER_INTERNALS_HPP

#include <functional>
#include <unordered_map>

#include <json.hpp>

#include "../levels/basic/level.hpp"
#include "../levels/levelFinish.hpp"
#include "levelTracker.hpp"

void to_json(nlohmann::json &j, Point<uint32_t> const &val);
void from_json(nlohmann::json const &j, Point<uint32_t> &val);
void to_json(nlohmann::json &j, Point<float> const &val);
void from_json(nlohmann::json const &j, Point<float> &val);

namespace levelTracker {
    namespace DataVariables {
        char constexpr const *GameSaveDataVersion = "GameSaveDataVersion";
        char constexpr const *GameSaveDataLevelName = "LevelName";
        char constexpr const *GameSaveDataScreenSize = "ScreenSize";
        char constexpr const *GameSaveDataLevel = "LevelSaveData";
        char constexpr const *GameSaveDataNeedsStarter = "LevelNeedsStarter";
    }
    using GenerateLevelGeneratorFcn = std::function<GenerateLevelFcn(nlohmann::json const &,
            nlohmann::json const *, float)>;
    using LevelMapEntry = std::pair<std::string, GenerateLevelGeneratorFcn>;
    using LevelMapTable = std::unordered_map<std::string, GenerateLevelGeneratorFcn>;

    using GenerateFinisherGeneratorFcn = std::function<GenerateFinisherFcn(nlohmann::json const &, float)>;
    using FinisherMapEntry = std::pair<std::string, GenerateFinisherGeneratorFcn>;
    using FinisherMapTable = std::unordered_map<std::string, GenerateFinisherGeneratorFcn>;

    LevelMapTable &starterTable();

    LevelMapTable &levelTable();

    FinisherMapTable &finisherTable();

    template<typename TableEntry, typename Table, Table (*table)()>
    class Register {
    public:
        Register(TableEntry entry) {
            table().insert(entry);
        }
    };

    using RegisterStarter = Register<LevelMapEntry, LevelMapTable, starterTable>;
    using RegisterLevel = Register<LevelMapEntry, LevelMapTable, levelTable>;
    using RegisterFinisher = Register<FinisherMapEntry, FinisherMapTable, finisherTable>;

    int constexpr GameSaveDataVersionValue = 1;
    struct GameSaveData {
        int version;
        Point<uint32_t> screenSize;
        std::string levelName;
        bool needsStarter;
        GameSaveData(
                Point<uint32_t> const &screenSize_,
                std::string const &levelName_,
                bool needsStarter_) :
                version(GameSaveDataVersionValue),
                screenSize(screenSize_),
                levelName(levelName_),
                needsStarter(needsStarter_)
        {}

        GameSaveData(
                int version_,
                Point<uint32_t> const &screenSize_,
                std::string const &levelName_,
                bool needsStarter_) :
                version(version_),
                screenSize(screenSize_),
                levelName(levelName_),
                needsStarter(needsStarter_)
        {}
    };

    template <typename LevelSaveDataType>
    std::vector<uint8_t> saveGameData(
            std::shared_ptr<GameSaveData> const &gameData,
            std::shared_ptr<LevelSaveDataType> const &levelData)
    {
        nlohmann::json j;
        j[DataVariables::GameSaveDataVersion] = gameData->version;
        j[DataVariables::GameSaveDataScreenSize] = gameData->screenSize;
        j[DataVariables::GameSaveDataLevelName] = gameData->levelName;
        j[DataVariables::GameSaveDataNeedsStarter] = gameData->needsStarter;

        if (levelData != nullptr) {
            j[DataVariables::GameSaveDataLevel] = *levelData;
        }

        std::vector<uint8_t> vec = nlohmann::json::to_cbor(j);

        return vec;
    }
}

#endif //AMAZING_LABYRINTH_LEVEL_TRACKER_INTERNALS_HPP
