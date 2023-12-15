/**
 * Copyright 2023 Cerulean Quasar. All Rights Reserved.
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
#include "../../levelTracker/internals.hpp"
#include "../generatedMaze/serializer.hpp"
#include "level.hpp"
#include "serializer.hpp"

namespace collectMaze {
    char constexpr const *CollectionObjLocation = "CollectionObjLocation";
    char constexpr const *ItemsCollected = "ItemsCollected";
    char constexpr const *PreviousCells = "PreviousCells";

    void to_json(nlohmann::json &j, LevelSaveData const &val) {
        to_json(j, boost::implicit_cast<generatedMaze::LevelSaveData const &>(val));
        j[CollectionObjLocation] = val.collectionObjLocations;
        j[ItemsCollected] = val.itemsCollected;
        j[PreviousCells] = val.previousCells;
    }

    void from_json(nlohmann::json const &j, LevelSaveData &val) {
        from_json(j, boost::implicit_cast<generatedMaze::LevelSaveData &>(val));
        val.collectionObjLocations = j[CollectionObjLocation].get<std::vector<Point<float>>>();
        val.itemsCollected = j[ItemsCollected].get<std::vector<bool>>();
        val.previousCells = j[PreviousCells].get<std::vector<Point<uint32_t>>>();
    }

    char constexpr const *NumberCollectObjects = "NumberCollectObjects";
    void to_json(nlohmann::json &j, LevelConfigData const &val) {
        to_json(j, boost::implicit_cast<generatedMaze::LevelConfigData const &>(val));
        j[NumberCollectObjects] = val.numberCollectObjects;
    }

    void from_json(nlohmann::json const &j, LevelConfigData &val) {
        from_json(j, boost::implicit_cast<generatedMaze::LevelConfigData &>(val));
        val.numberCollectObjects = j[NumberCollectObjects].get<uint32_t>();
    }

    std::vector<uint8_t> Level::saveData(levelTracker::GameSaveData const &gsd,
                                         char const *saveLevelDataKey)
    {
        std::vector<Point<float>> collectionObjLocations;
        std::vector<bool> itemsCollected;
        collectionObjLocations.reserve(m_collectionObjectLocations.size());
        itemsCollected.reserve(m_collectionObjectLocations.size());
        for (auto const &collectionObj : m_collectionObjectLocations) {
            collectionObjLocations.emplace_back(collectionObj.second.x, collectionObj.second.y);
            itemsCollected.push_back(collectionObj.first);
        }

        std::vector<Point<uint32_t>> previousCells;
        previousCells.reserve(m_prevCells.size());
        for (auto const &prevCell : m_prevCells) {
            previousCells.emplace_back(prevCell.first, prevCell.second);
        }
        LevelSaveData sd(
                m_ballCell.row,
                m_ballCell.col,
                Point<float>{m_ball.position.x, m_ball.position.y},
                m_mazeBoard.rowEnd(),
                m_mazeBoard.colEnd(),
                std::vector<uint32_t>{m_wallTextureIndices},
                getSerializedMazeWallVector(),
                std::move(collectionObjLocations),
                std::move(itemsCollected),
                std::move(previousCells));

        nlohmann::json j;
        to_json(j, gsd);
        j[saveLevelDataKey] = sd;
        return nlohmann::json::to_cbor(j);
    }

    levelTracker::Register<levelTracker::LevelMapTable, levelTracker::levelTable, LevelConfigData, LevelSaveData, Level> registerLevel;
} // namespace collectMaze