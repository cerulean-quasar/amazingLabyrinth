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
#include "movingQuadsLevel.hpp"
#include "../../serializeSaveDataInternals.hpp"

char constexpr const *BallLocation = "BallLocation";
char constexpr const *QuadRows = "QuadRows";
char constexpr const *Positions = "Positions";
char constexpr const *Speed = "Speed";
char constexpr const *Scale = "Scale";
void to_json(nlohmann::json &j, QuadRowSaveData const &val) {
    j[Positions] = val.positions;
    j[Speed] = val.speed;
    j[Scale] = val.scale;
}

void from_json(nlohmann::json const &j, QuadRowSaveData &val) {
    val.positions = j[Positions].get<std::vector<Point<float>>>();
    val.speed = j[Speed].get<float>();
    val.scale = j[Scale].get<Point<float>>();
}

void to_json(nlohmann::json &j, MovingQuadsLevelSaveData const &val) {
    to_json(j, boost::implicit_cast<LevelSaveData const &>(val));
    j[BallLocation] = val.ball;
    j[QuadRows] = val.quadRows;
}

void from_json(nlohmann::json const &j, MovingQuadsLevelSaveData &val) {
    from_json(j, boost::implicit_cast<LevelSaveData&>(val));
    val.ball = j[BallLocation].get<Point<float>>();
    val.quadRows = j[QuadRows].get<std::vector<QuadRowSaveData>>();
}

Level::SaveLevelDataFcn MovingQuadsLevel::getSaveLevelDataFcn() {
    std::vector<QuadRowSaveData> quadRows;
    quadRows.reserve(m_movingQuads.size());
    for (auto const &movingQuad : m_movingQuads) {
        std::vector<Point<float>> positions;
        for (auto const &position : movingQuad.positions) {
            positions.emplace_back(position.x, position.y);
        }
        quadRows.emplace_back(std::move(positions), movingQuad.speed, Point<float>{movingQuad.scale.x, movingQuad.scale.y});
    }
    auto sd = std::make_shared<MovingQuadsLevelSaveData>(
            Point<float>{m_ball.position.x, m_ball.position.y},
            std::move(quadRows));
    return {[sd](std::shared_ptr<GameSaveData> gsd) -> std::vector<uint8_t> {
        return saveGameData(gsd, sd);
    }};
}
