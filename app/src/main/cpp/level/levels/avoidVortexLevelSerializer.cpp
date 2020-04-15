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
#include "avoidVortexLevel.hpp"
#include "../../serializeSaveDataInternals.hpp"

char constexpr const *BallLocation = "BallLocation";
char constexpr const *HoleLocation = "HoleLocation";
char constexpr const *StartPosition = "StartPosition";
char constexpr const *Vortexes = "Vortexes";
void to_json(nlohmann::json &j, AvoidVortexLevelSaveData const &val) {
    to_json(j, boost::implicit_cast<LevelSaveData const &>(val));
    j[BallLocation] = val.ball;
    j[HoleLocation] = val.hole;
    j[StartPosition] = val.startPos;
    j[Vortexes] = val.vortexes;
}

void from_json(nlohmann::json const &j, AvoidVortexLevelSaveData &val) {
    from_json(j, boost::implicit_cast<LevelSaveData&>(val));
    val.ball = j[BallLocation].get<Point<float>>();
    val.hole = j[HoleLocation].get<Point<float>>();
    val.startPos = j[StartPosition].get<Point<float>>();
    val.vortexes = j[Vortexes].get<std::vector<Point<float>>>();
}

Level::SaveLevelDataFcn AvoidVortexLevel::getSaveLevelDataFcn() {
    std::vector<Point<float>> vortexes;
    vortexes.reserve(vortexPositions.size());
    for (auto const &vortexPosition : vortexPositions) {
        vortexes.emplace_back(vortexPosition.x, vortexPosition.y);
    }
    auto sd = std::make_shared<AvoidVortexLevelSaveData>(
            Point<float>{m_ball.position.x, m_ball.position.y},
            Point<float>{holePosition.x, holePosition.y},
            Point<float>{startPosition.x, startPosition.y},
            std::move(vortexes));
    return {[sd](std::shared_ptr<GameSaveData> gsd) -> std::vector<uint8_t> {
        return saveGameData(gsd, sd);
    }};
}
