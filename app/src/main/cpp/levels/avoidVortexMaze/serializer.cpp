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
#include <boost/optional.hpp>
#include "level.hpp"
#include "loadData.hpp"
#include "../generatedMaze/serializer.hpp"
#include "serializer.hpp"
#include "../../levelTracker/internals.hpp"

namespace avoidVortexMaze {
    char constexpr const *AvoidObjLocation = "AvoidObjLocation";
    char constexpr const *BallStart = "BallStart";

    void to_json(nlohmann::json &j, LevelSaveData const &val) {
        to_json(j, boost::implicit_cast<generatedMaze::LevelSaveData const &>(val));
        j[AvoidObjLocation] = val.avoidObjLocations;
        j[BallStart] = val.startRowCol;
    }

    void from_json(nlohmann::json const &j, LevelSaveData &val) {
        from_json(j, boost::implicit_cast<generatedMaze::LevelSaveData &>(val));
        val.avoidObjLocations = j[AvoidObjLocation].get<std::vector<Point<float>>>();
        val.startRowCol = j[BallStart].get<Point<uint32_t>>();
    }

    char constexpr const *AvoidTexture = "AvoidTexture";
    char constexpr const *NumberAvoidObjects = "NumberAvoidObjects";
    void to_json(nlohmann::json &j, LevelConfigData const &val) {
        to_json(j, boost::implicit_cast<generatedMaze::LevelConfigData  const &>(val));
        j[AvoidTexture] = val.avoidTexture;
        j[NumberAvoidObjects] = val.numberAvoidObjects;
    }

    void from_json(nlohmann::json const &j, LevelConfigData  &val) {
        from_json(j, boost::implicit_cast<generatedMaze::LevelConfigData  &>(val));
        val.avoidTexture = j[AvoidTexture].get<std::string>();
        val.numberAvoidObjects = j[NumberAvoidObjects].get<uint32_t>();
    }

    std::vector<uint8_t> Level::saveData(levelTracker::GameSaveData const &gsd, char const *saveLevelDataKey) {
        std::vector<Point<float>> avoidObjLocations;
        avoidObjLocations.reserve(m_avoidObjectLocations.size());
        for (auto const &avoidObj : m_avoidObjectLocations) {
            avoidObjLocations.emplace_back(avoidObj.x, avoidObj.y);
        }

        LevelSaveData sd(
                m_ballCell.row,
                m_ballCell.col,
                Point<float>{m_ball.position.x, m_ball.position.y},
                m_mazeBoard.rowEnd(),
                m_mazeBoard.colEnd(),
                std::vector<uint32_t>{m_wallTextureIndices},
                getSerializedMazeWallVector(),
                std::move(avoidObjLocations),
                Point<uint32_t>{static_cast<uint32_t>(m_mazeBoard.rowStart()),
                                static_cast<uint32_t>(m_mazeBoard.colStart())});

        nlohmann::json j;
        to_json(j, gsd);
        j[saveLevelDataKey] = sd;
        return nlohmann::json::to_cbor(j);
    }

    levelTracker::Register<levelTracker::LevelMapTable, levelTracker::levelTable, LevelConfigData, LevelSaveData, Level> registerLevel;
} // namespace avoidVortexMaze