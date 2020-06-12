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
#include <memory>
#include <json.hpp>
#include <boost/implicit_cast.hpp>
#include "../../serializeSaveDataInternals.hpp"
#include "../basic/serializer.hpp"

#include "serializer.hpp"
#include "level.hpp"
#include "loadData.hpp"

namespace movablePassage {
    void to_json(nlohmann::json &j, LevelSaveData const &val) {
        to_json(j, boost::implicit_cast<basic::LevelSaveData const &>(val));
    }

    void from_json(nlohmann::json const &j, LevelSaveData &val) {
        from_json(j, boost::implicit_cast<basic::LevelSaveData &>(val));
    }

    char constexpr const *Row = "Row";
    char constexpr const *Col = "Col";
    void to_json(nlohmann::json &j, RockPlacement const &val) {
        j[Row] = val.row;
        j[Col] = val.col;
    }

    void from_json(nlohmann::json const &j, RockPlacement &val) {
        val.row = j[Row].get<uint32_t>();
        val.col = j[Col].get<uint32_t>();
    }

    char constexpr const *Model = "Model";
    char constexpr const *Texture = "Texture";
    char constexpr const *NumberPlacements = "NumberPlacements";
    void to_json(nlohmann::json &j, ComponentConfig const &val) {
        j[Model] = val.model;
        j[Texture] = val.texture;
        j[NumberPlacements] = val.numberPlacements;
    }

    void from_json(nlohmann::json const &j, ComponentConfig &val) {
        val.model = j[Model].get<std::string>();
        val.texture = j[Texture].get<std::string>();
        val.numberPlacements = j[NumberPlacements].get<uint32_t>();
    }

    char constexpr const *NumberTilesX = "NumberTilesX";
    char constexpr const *NumberTilesY = "NumberTilesY";
    char constexpr const *StartColumn = "StartColumn";
    char constexpr const *EndColumn = "EndColumn";
    char constexpr const *EndTexture = "EndTexture";
    char constexpr const *EndOffBoardTexture = "EndOffBoardTexture";
    char constexpr const *PlacementLockedInPlaceTexture = "PlacementLockedInPlaceTexture";
    char constexpr const *RockModels = "RockModels";
    char constexpr const *RockTextures = "RockTextures";
    char constexpr const *DirtModels = "DirtModels";
    char constexpr const *DirtTextures = "DirtTextures";
    char constexpr const *BeginningSideModels = "BeginningSideModels";
    char constexpr const *BeginningSideTextures = "BeginningSideTextures";
    char constexpr const *BeginningOpenModels = "BeginningOpenModels";
    char constexpr const *BeginningOpenTextures = "BeginningOpenTextures";
    char constexpr const *BeginningCornerModels = "BeginningCornerModels";
    char constexpr const *BeginningCornerTextures = "BeginningCornerTextures";
    char constexpr const *Straight = "Straight";
    char constexpr const *Turn = "Turn";
    char constexpr const *CrossJunction = "CrossJunction";
    char constexpr const *TJunction = "TJunction";
    char constexpr const *RockPlacements = "RockPlacements";
    void to_json(nlohmann::json &j, LevelConfigData const &val) {
        to_json(j, boost::implicit_cast<basic::LevelConfigData const &>(val));

        j[NumberTilesX] = val.numberTilesX;
        j[NumberTilesY] = val.numberTilesY;
        j[StartColumn] = val.startColumn;
        j[EndColumn] = val.endColumn;
        j[EndTexture] = val.endTexture;
        j[EndOffBoardTexture] = val.endOffBoardTexture;
        j[PlacementLockedInPlaceTexture] = val.placementLockedInPlaceTexture;
        j[RockModels] = val.rockModels;
        j[RockTextures] = val.rockTextures;
        j[DirtModels] = val.dirtModels;
        j[DirtTextures] = val.dirtTextures;

        j[BeginningSideModels] = val.beginningSideModels;
        j[BeginningSideTextures] = val.beginningSideTextures;

        j[BeginningOpenModels] = val.beginningOpenModels;
        j[BeginningOpenTextures] = val.beginningOpenTextures;

        j[BeginningCornerModels] = val.beginningCornerModels;
        j[BeginningCornerTextures] = val.beginningCornerTextures;

        j[Straight] = val.straight;
        j[Turn] = val.turn;
        j[CrossJunction] = val.crossjunction;
        j[TJunction] = val.tjunction;
        j[RockPlacements] = val.rockPlacements;
    }

    void from_json(nlohmann::json const &j, LevelConfigData &val) {
        from_json(j, boost::implicit_cast<basic::LevelConfigData &>(val));

        val.numberTilesX = j[NumberTilesX].get<uint32_t>();
        val.numberTilesY = j[NumberTilesY].get<uint32_t>();
        val.startColumn = j[StartColumn].get<uint32_t>();
        val.endColumn = j[EndColumn].get<uint32_t>();
        val.endTexture = j[EndTexture].get<std::string>();
        val.endOffBoardTexture = j[EndOffBoardTexture].get<std::string>();
        val.placementLockedInPlaceTexture = j[PlacementLockedInPlaceTexture].get<std::string>;
        val.rockModels = j[RockModels].get<std::vector<std::string>>();
        val.rockTextures = j[RockTextures].get<std::vector<std::string>>();
        val.dirtModels = j[DirtModels].get<std::vector<std::string>>();
        val.dirtTextures = j[DirtTextures].get<std::vector<std::string>>();

        val.beginningSideModels = j[BeginningSideModels].get<std::vector<std::string>>();
        val.beginningSideTextures = j[BeginningSideTextures].get<std::vector<std::string>>();

        val.beginningOpenModels = j[BeginningOpenModels].get<std::vector<std::string>>();
        val.beginningOpenTextures = j[BeginningOpenTextures].get<std::vector<std::string>>();

        val.beginningCornerModels = j[BeginningCornerModels].get<std::vector<std::string>>();
        val.beginningCornerTextures = j[BeginningCornerTextures].get<std::vector<std::string>>();

        val.straight = j[Straight].get<ComponentConfig>();
        val.turn = j[Turn].get<ComponentConfig>();
        val.crossjunction = j[CrossJunction].get<ComponentConfig>();
        val.tjunction = j[TJunction].get<ComponentConfig>();
        val.rockPlacements = j[RockPlacements].get<ComponentConfig>();
    }

    basic::Level::SaveLevelDataFcn Level::getSaveLevelDataFcn() {
        auto sd = std::make_shared<LevelSaveData>();
        return {[sd](std::shared_ptr<GameSaveData> gsd) -> std::vector<uint8_t> {
            return saveGameData(gsd, sd);
        }};
    }

} // namespace movablePassage