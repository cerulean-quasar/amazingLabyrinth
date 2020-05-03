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
#include <memory>
#include <json.hpp>
#include "level.hpp"
#include "../serializeSaveDataInternals.hpp"

template <>
std::vector<uint8_t> saveGameData<void>(
        std::shared_ptr<GameSaveData> const &gameData,
        std::shared_ptr<void> const &)
{
    nlohmann::json j;
    j[GameSaveDataVersion] = gameData->version;
    j[GameSaveDataScreenSize] = gameData->screenSize;
    j[GameSaveDataLevelName] = gameData->levelName;

    std::vector<uint8_t> vec = nlohmann::json::to_cbor(j);

    return vec;
}

Level::SaveLevelDataFcn Level::getSaveLevelDataFcn() {
    return getBasicSaveLevelDataFcn();
}

Level::SaveLevelDataFcn Level::getBasicSaveLevelDataFcn() {
    return {[](std::shared_ptr<GameSaveData> gsd) -> std::vector<uint8_t> {
        return saveGameData(gsd, std::shared_ptr<void>());
    }};
}
