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

#include "../basic/level.hpp"
#include "../../levelTracker/internals.hpp"

#include "loadData.hpp"
#include "level.hpp"
#include "serializer.hpp"

namespace fixedMaze {
    void to_json(nlohmann::json &j, LevelSaveData const &val) {
        to_json(j, boost::implicit_cast<basic::LevelSaveData const &>(val));
    }

    void from_json(nlohmann::json const &j, LevelSaveData &val) {
        from_json(j, boost::implicit_cast<basic::LevelSaveData &>(val));
    }

    char constexpr const *MazeFloorModel = "MazeFloorModel";
    char constexpr const *MazeFloorTexture = "MazeFloorTexture";
    char constexpr const *ExtraBounce = "ExtraBounce";
    char constexpr const *MinSpeedOnBounce = "MinSpeedOnBounce";
    void to_json(nlohmann::json &j, LevelConfigData const &val) {
        to_json(j, boost::implicit_cast<basic::LevelConfigData const &>(val));
        j[MazeFloorModel] = val.m_mazeFloorModel;
        j[MazeFloorTexture] = val.m_mazeFloorTexture;
        j[ExtraBounce] = val.m_extraBounce;
        j[MinSpeedOnBounce] = val.m_minSpeedOnBounce;
    }

    void from_json(nlohmann::json const &j, LevelConfigData &val) {
        from_json(j, boost::implicit_cast<basic::LevelConfigData &>(val));
        val.m_mazeFloorModel = j[MazeFloorModel].get<std::string>();
        val.m_mazeFloorTexture = j[MazeFloorTexture].get<std::string>();
        val.m_extraBounce = j[ExtraBounce].get<float>();
        val.m_minSpeedOnBounce = j[MinSpeedOnBounce].get<float>();
    }

    std::vector<uint8_t> Level::saveData(levelTracker::GameSaveData const &gsd,
                                  char const *saveLevelDataKey) {
        LevelSaveData sd();

        nlohmann::json j;
        to_json(j, gsd);
        j[saveLevelDataKey] = sd;
        return nlohmann::json::to_cbor(j);
    }

    levelTracker::RegisterLevel registerLevel(std::make_pair(Level::m_name,
         levelTracker::GenerateLevelFcn(
             [](nlohmann::json const &lcdjson, nlohmann::json const *sdjson, float z) -> levelTracker::GenerateLevelFcn
             {
                 LevelConfigData lcd = lcdjson.get<LevelConfigData>();
                 std::shared_ptr<LevelSaveData> sd;
                 if (sdjson) {
                     sd = std::make_shared(sdjson->get<LevelSaveData>());
                 }
                 return levelTracker::GenerateLevelFcn(
                     [lcd, sd, z](std::shared_ptr<GameRequester> gameRequester,
                                  glm::mat4 const &proj, glm::mat4 const &view) -> std::shared_ptr<basic::Level>
                     {
                         return std::make_shared<Level>(
                                 std::move(gameRequester), lcd, sd, proj, view, z);
                     });
             }))
    );
}