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
#ifndef AMAZING_LABYRINTH_SERIALIZE_SAVE_DATA_INTERNALS_HPP
#define AMAZING_LABYRINTH_SERIALIZE_SAVE_DATA_INTERNALS_HPP
#include <memory>
#include <json.hpp>
#include "saveData.hpp"
#include "level/levels/maze.hpp"
#include "level/levels/mazeCollect.hpp"
#include "level/levels/movingQuadsLevel.hpp"
#include "level/levels/openAreaLevel.hpp"
#include "level/levels/avoidVortexLevel.hpp"
#include "level/levels/mazeAvoid.hpp"
#include "level/levels/fixedMaze.hpp"
#include "level/levels/movablePassage.hpp"

void to_json(nlohmann::json &j, Point<uint32_t> const &val);
void from_json(nlohmann::json const &j, Point<uint32_t> &val);
void to_json(nlohmann::json &j, Point<float> const &val);
void from_json(nlohmann::json const &j, Point<float> &val);
void to_json(nlohmann::json &j, LevelSaveData const &val);
void from_json(nlohmann::json const &j, LevelSaveData &val);

void from_json(nlohmann::json const &j, OpenAreaLevelSaveData &val);
void to_json(nlohmann::json &j, OpenAreaLevelSaveData const &val);
void to_json(nlohmann::json &j, AvoidVortexLevelSaveData const &val);
void from_json(nlohmann::json const &j, AvoidVortexLevelSaveData &val);
void to_json(nlohmann::json &j, MovingQuadsLevelSaveData const &val);
void from_json(nlohmann::json const &j, MovingQuadsLevelSaveData &val);
void to_json(nlohmann::json &j, MazeSaveData const &val);
void from_json(nlohmann::json const &j, MazeSaveData &val);
void to_json(nlohmann::json &j, MazeCollectSaveData const &val);
void from_json(nlohmann::json const &j, MazeCollectSaveData &val);
void to_json(nlohmann::json &j, MazeAvoidSaveData const &val);
void from_json(nlohmann::json const &j, MazeAvoidSaveData &val);
void to_json(nlohmann::json &j, FixedMazeSaveData const &val);
void from_json(nlohmann::json const &j, FixedMazeSaveData &val);
void to_json(nlohmann::json &j, MovablePassageSaveData const &val);
void from_json(nlohmann::json const &j, MovablePassageSaveData &val);

char constexpr const *GameSaveDataVersion = "GameSaveDataVersion";
char constexpr const *GameSaveDataLevelName = "LevelName";
char constexpr const *GameSaveDataScreenSize = "ScreenSize";
char constexpr const *GameSaveDataLevel = "LevelSaveData";
char constexpr const *GameSaveDataNeedsStarter = "LevelNeedsStarter";

template <typename LevelSaveDataType>
std::vector<uint8_t> saveGameData(
        std::shared_ptr<GameSaveData> const &gameData,
        std::shared_ptr<LevelSaveDataType> const &levelData)
{
    nlohmann::json j;
    j[GameSaveDataVersion] = gameData->version;
    j[GameSaveDataScreenSize] = gameData->screenSize;
    j[GameSaveDataLevelName] = gameData->levelName;
    j[GameSaveDataNeedsStarter] = gameData->needsStarter;

    if (levelData != nullptr) {
        j[GameSaveDataLevel] = *levelData;
    }

    std::vector<uint8_t> vec = nlohmann::json::to_cbor(j);

    return vec;
}

template <>
std::vector<uint8_t> saveGameData<void>(
        std::shared_ptr<GameSaveData> const &gameData,
        std::shared_ptr<void> const &levelData);

#endif