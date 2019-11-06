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
#include <boost/implicit_cast.hpp>
#include <json.hpp>

#include "openAreaLevel.hpp"
#include "../../serializeSaveDataInternals.hpp"

char constexpr const *BallLocation = "BallLocation";
char constexpr const *HoleLocation = "HoleLocation";
void to_json(nlohmann::json &j, OpenAreaLevelSaveData const &val) {
    to_json(j, boost::implicit_cast<LevelSaveData const &>(val));
    j[BallLocation] = val.ball;
    j[HoleLocation] = val.hole;
}

void from_json(nlohmann::json const &j, OpenAreaLevelSaveData &val) {
    from_json(j, boost::implicit_cast<LevelSaveData&>(val));
    val.ball = j[BallLocation].get<Point<float>>();
    val.hole = j[HoleLocation].get<Point<float>>();
}

Level::SaveLevelDataFcn OpenAreaLevel::getSaveLevelDataFcn() {
    auto sd = std::make_shared<OpenAreaLevelSaveData>(Point<float>{ball.position.x, ball.position.y}, Point<float>{holePosition.x, holePosition.y});
    return {[sd](std::shared_ptr<GameSaveData> gsd) -> std::vector<uint8_t> {
        nlohmann::json j;
        return saveGameData(gsd, sd);
    }};
}
