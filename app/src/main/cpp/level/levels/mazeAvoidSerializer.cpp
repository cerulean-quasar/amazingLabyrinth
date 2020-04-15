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
#include "mazeAvoid.hpp"

char constexpr const *AvoidObjLocation = "AvoidObjLocation";
char constexpr const *BallStart = "BallStart";
void to_json(nlohmann::json &j, MazeAvoidSaveData const &val) {
    to_json(j, boost::implicit_cast<MazeSaveData const &>(val));
    j[AvoidObjLocation] = val.avoidObjLocations;
    j[BallStart] = val.startRowCol;
}

void from_json(nlohmann::json const &j, MazeAvoidSaveData &val) {
    from_json(j, boost::implicit_cast<MazeSaveData&>(val));
    val.avoidObjLocations = j[AvoidObjLocation].get<std::vector<Point<float>>>();
    val.startRowCol = j[BallStart].get<Point<uint32_t>>();
}

Level::SaveLevelDataFcn MazeAvoid::getSaveLevelDataFcn() {
    std::vector<Point<float>> avoidObjLocations;
    avoidObjLocations.reserve(m_avoidObjectLocations.size());
    for (auto const &avoidObj : m_avoidObjectLocations) {
        avoidObjLocations.emplace_back(avoidObj.x, avoidObj.y);
    }

    auto sd = std::make_shared<MazeAvoidSaveData>(
            numberRows,
            m_ballCell.row,
            m_ballCell.col,
            Point<float>{m_ball.position.x, m_ball.position.y},
            m_rowEnd,
            m_colEnd,
            std::vector<uint32_t>{m_wallTextureIndices},
            getSerializedMazeWallVector(),
            std::move(avoidObjLocations),
            Point<uint32_t>{m_ballStartR, m_ballStartC});

    return {[sd{move(sd)}](std::shared_ptr<GameSaveData> gsd) -> std::vector<uint8_t> {
        return saveGameData(gsd, sd);
    }};
}
