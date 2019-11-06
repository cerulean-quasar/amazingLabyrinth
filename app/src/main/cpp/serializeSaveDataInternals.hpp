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
#ifndef AMAZING_LABYRINTH_SERIALIZE_SAVE_DATA_INTERNALS_HPP
#define AMAZING_LABYRINTH_SERIALIZE_SAVE_DATA_INTERNALS_HPP
#include <memory>
#include <json.hpp>
#include "saveData.hpp"

void to_json(nlohmann::json &j, Point<uint32_t> const &val);
void from_json(nlohmann::json const &j, Point<uint32_t> &val);
void to_json(nlohmann::json &j, Point<float> const &val);
void from_json(nlohmann::json const &j, Point<float> &val);
void to_json(nlohmann::json &j, LevelSaveData const &val);
void from_json(nlohmann::json const &j, LevelSaveData &val);


char constexpr const *GameSaveDataVersion = "GameSaveDataVersion";
char constexpr const *GameSaveDataLevelName = "LevelName";
char constexpr const *GameSaveDataScreenSize = "ScreenSize";
char constexpr const *GameSaveDataLevel = "LevelSaveData";

template <typename LevelSaveDataType>
std::vector<uint8_t> saveGameData(
        std::shared_ptr<GameSaveData> const &gameData,
std::shared_ptr<LevelSaveDataType> const &levelData)
{
    nlohmann::json j;
    j[GameSaveDataVersion] = gameData->version;
    j[GameSaveDataScreenSize] = gameData->screenSize;
    j[GameSaveDataLevelName] = gameData->levelName;

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