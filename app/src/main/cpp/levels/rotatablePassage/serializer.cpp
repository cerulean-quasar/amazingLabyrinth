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
#include "level.hpp"
#include "../basic/serializer.hpp"
#include "../basic/level.hpp"
#include "../../serializeSaveDataInternals.hpp"

#include "loadData.hpp"
#include "serializer.hpp"

namespace rotatablePassage {
    void to_json(nlohmann::json &j, LevelSaveData const &val) {
        to_json(j, boost::implicit_cast<basic::LevelSaveData const &>(val));
    }

    void from_json(nlohmann::json const &j, LevelSaveData &val) {
        from_json(j, boost::implicit_cast<basic::LevelSaveData &>(val));
    }

    char constexpr const *Model = "Model";
    char constexpr const *Texture = "Texture";
    char constexpr const *LockedInPlaceTexture = "LockedInPlaceTexture";
    void to_json(nlohmann::json &j, ComponentConfig const &val) {
        j[Model] = val.model;
        j[Texture] = val.texture;
        j[LockedInPlaceTexture] = val.lockedInPlaceTexture;
    }

    void from_json(nlohmann::json const &j, ComponentConfig &val) {
        val.model = j[Model].get<std::string>();
        val.texture = j[Texture].get<std::string>();
        val.lockedInPlaceTexture = j[LockedInPlaceTexture].get<std::string>();
    }

    char constexpr const *HoleModel = "HoleModel";
    char constexpr const *HoleTexture ="HoleTexture";
    char constexpr const *NumberRows = "NumberRows";
    char constexpr const *DfsSearch = "DfsSearch";
    char constexpr const *BorderTextures ="BorderTextures";
    char constexpr const *Straight = "Straight";
    char constexpr const *Turn = "Turn";
    char constexpr const *CrossJunction = "CrossJunction";
    char constexpr const *TJunction = "TJunction";
    char constexpr const *DeadEnd = "DeadEnd";
    void to_json(nlohmann::json &j, LevelConfigData const &val) {
        to_json(j, boost::implicit_cast<basic::LevelConfigData const &>(val));
        j[HoleModel] = val.holeModel;
        j[HoleTexture] = val.holeTexture;
        j[NumberRows] = val.numberRows;
        j[DfsSearch] = val.dfsSearch;
        j[BorderTextures] = val.borderTextures;
        j[Straight] = val.straight;
        j[Turn] = val.turn;
        j[CrossJunction] = val.crossJunction;
        j[TJunction] = val.tJunction;
        j[DeadEnd] = val.deadEnd;
    }

    void from_json(nlohmann::json const &j, LevelConfigData &val) {
        from_json(j, boost::implicit_cast<basic::LevelConfigData &>(val));
        val.holeModel = j[HoleModel].get<std::string>();
        val.holeTexture = j[HoleTexture].get<std::string>();
        val.numberRows = j[NumberRows].get<uint32_t>();
        val.dfsSearch = j[DfsSearch].get<bool>();
        val.borderTextures = j[BorderTextures].get<std::vector<std::string>>();
        val.straight = j[Straight].get<ComponentConfig>();
        val.turn = j[Turn].get<ComponentConfig>();
        val.crossJunction = j[CrossJunction].get<ComponentConfig>();
        val.tJunction = j[TJunction].get<ComponentConfig>();
        val.deadEnd = j[DeadEnd].get<ComponentConfig>();
    }

    basic::Level::SaveLevelDataFcn Level::getSaveLevelDataFcn() {
        auto sd = std::make_shared<LevelSaveData>();
        return {[sd](std::shared_ptr<GameSaveData> gsd) -> std::vector<uint8_t> {
            return saveGameData(gsd, sd);
        }};
    }
} // namespace rotatablePassage