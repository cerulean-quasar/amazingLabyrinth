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
#ifndef AMAZING_LABYRINTH_MAZE_COLLECT_LOAD_DATA_HPP
#define AMAZING_LABYRINTH_MAZE_COLLECT_LOAD_DATA_HPP

#include <vector>
#include <string>
#include "../../levelTracker/internals.hpp"
#include "../generatedMaze/loadData.hpp"

namespace collectMaze {
    struct LevelSaveData : public generatedMaze::LevelSaveData {
        std::vector <Point<float>> collectionObjLocations;
        std::vector<bool> itemsCollected;
        std::vector <Point<uint32_t>> previousCells;

        LevelSaveData(LevelSaveData &&other) noexcept = default;

        LevelSaveData(LevelSaveData const &other) noexcept = default;

        LevelSaveData &operator=(LevelSaveData const &other) noexcept = default;

        LevelSaveData()
                : generatedMaze::LevelSaveData{},
                  collectionObjLocations{},
                  itemsCollected{},
                  previousCells{} {
        }

        LevelSaveData(
                uint32_t ballRow_,
                uint32_t ballCol_,
                Point<float> &&ballPos_,
                uint32_t rowEnd_,
                uint32_t colEnd_,
                std::vector <uint32_t> wallTextures_,
                std::vector <uint8_t> mazeWallsVector_,
                std::vector <Point<float>> collectionObjLocations_,
                std::vector<bool> itemsCollected_,
                std::vector <Point<uint32_t>> previousCells_)
                : generatedMaze::LevelSaveData{ballRow_, ballCol_, std::move(ballPos_), rowEnd_, colEnd_,
                               std::move(wallTextures_), std::move(mazeWallsVector_)},
                  collectionObjLocations{std::move(collectionObjLocations_)},
                  itemsCollected{std::move(itemsCollected_)},
                  previousCells{std::move(previousCells_)} {
        }
    };

    struct LevelConfigData : public generatedMaze::LevelConfigData {
        uint32_t numberCollectObjects;

        LevelConfigData()
            : generatedMaze::LevelConfigData(),
              numberCollectObjects{0}
        {}

        LevelConfigData(LevelConfigData const &other) noexcept = default;

        LevelConfigData(LevelConfigData &&other) noexcept = default;

        LevelConfigData &operator=(LevelConfigData const &other) noexcept = default;
    };
} // namespace collectMaze
#endif // AMAZING_LABYRINTH_MAZE_COLLECT_LOAD_DATA_HPP