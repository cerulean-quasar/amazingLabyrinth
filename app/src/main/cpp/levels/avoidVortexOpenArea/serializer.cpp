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
#include <boost/implicit_cast.hpp>
#include <json.hpp>
#include "loadData.hpp"
#include "../basic/level.hpp"
#include "../basic/serializer.hpp"
#include "../../levelTracker/internals.hpp"
#include "level.hpp"
#include "serializer.hpp"

namespace avoidVortexOpenArea {
    char constexpr const *BallLocation = "BallLocation";
    char constexpr const *HoleLocation = "HoleLocation";
    char constexpr const *StartPosition = "StartPosition";
    char constexpr const *Vortexes = "Vortexes";

    void to_json(nlohmann::json &j, LevelSaveData const &val) {
        to_json(j, boost::implicit_cast<basic::LevelSaveData const &>(val));
        j[BallLocation] = val.ball;
        j[HoleLocation] = val.hole;
        j[StartPosition] = val.startPos;
        j[Vortexes] = val.vortexes;
    }

    void from_json(nlohmann::json const &j, LevelSaveData &val) {
        from_json(j, boost::implicit_cast<basic::LevelSaveData &>(val));
        val.ball = j[BallLocation].get<Point<float>>();
        val.hole = j[HoleLocation].get<Point<float>>();
        val.startPos = j[StartPosition].get<Point<float>>();
        val.vortexes = j[Vortexes].get<std::vector<Point<float>>>();
    }

    char constexpr const *HoleTexture = "HoleTexture";
    char constexpr const *VortexTexture = "VortexTexture";
    char constexpr const *StartVortexTexture = "StartVortexTexture";
    void to_json(nlohmann::json &j, LevelConfigData const &val) {
        to_json(j, boost::implicit_cast<basic::LevelConfigData const &>(val));
        j[HoleTexture] = val.m_holeTexture;
        j[VortexTexture] = val.m_vortexTexture;
        j[StartVortexTexture] = val.m_startVortexTexture;
    }

    void from_json(nlohmann::json const &j, LevelConfigData &val) {
        from_json(j, boost::implicit_cast<basic::LevelConfigData &>(val));
        val.m_holeTexture = j[HoleTexture].get<std::string>();
        val.m_vortexTexture = j[VortexTexture].get<std::string>();
        val.m_startVortexTexture = j[StartVortexTexture].get<std::string>();
    }

    std::vector<uint8_t> Level::saveData(levelTracker::GameSaveData const &gsd,
                                         char const *saveLevelDataKey) {
        std::vector<Point<float>> vortexes;
        vortexes.reserve(vortexPositions.size());
        for (auto const &vortexPosition : vortexPositions) {
            vortexes.emplace_back(vortexPosition.x, vortexPosition.y);
        }
        LevelSaveData sd(
                Point<float>{m_ball.position.x, m_ball.position.y},
                Point<float>{holePosition.x, holePosition.y},
                Point<float>{startPosition.x, startPosition.y},
                std::move(vortexes));
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
} // namespace avoidVortexOpenArea