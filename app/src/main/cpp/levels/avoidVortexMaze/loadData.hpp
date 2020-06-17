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

#ifndef AMAZING_LABYRINTH_AVOID_VORTEX_MAZE_LOAD_DATA_HPP
#define AMAZING_LABYRINTH_AVOID_VORTEX_MAZE_LOAD_DATA_HPP

#include <vector>
#include "../generatedMaze/loadData.hpp"

namespace avoidVortexMaze {

    struct LevelSaveData : public generatedMaze::LevelSaveData {
        std::vector<Point<float>> avoidObjLocations;
        Point<uint32_t> startRowCol;

        // Intentionally slices other to pass part into the MazeSaveData constructor.
        LevelSaveData(LevelSaveData &&other) noexcept
                : generatedMaze::LevelSaveData(std::move(other)),
                  avoidObjLocations{std::move(other.avoidObjLocations)} {
        }

        LevelSaveData()
                : generatedMaze::LevelSaveData{},
                  avoidObjLocations{},
                  startRowCol{} {
        }

        LevelSaveData(
                uint32_t ballRow_,
                uint32_t ballCol_,
                Point<float> ballPos_,
                uint32_t rowEnd_,
                uint32_t colEnd_,
                std::vector<uint32_t> wallTextures_,
                std::vector<uint8_t> mazeWallsVector_,
                std::vector<Point<float>> avoidObjLocations_,
                Point<uint32_t> startRowCol_)
                : generatedMaze::LevelSaveData{ballRow_, ballCol_, std::move(ballPos_),
                                               rowEnd_, colEnd_,
                                               std::move(wallTextures_),
                                               std::move(mazeWallsVector_)},
                  avoidObjLocations{std::move(avoidObjLocations_)},
                  startRowCol{std::move(startRowCol_)}
        {}
    };

    struct LevelConfigData : public generatedMaze::LevelConfigData {
        std::string avoidTexture;
        uint32_t numberAvoidObjects;

        LevelConfigData()
            : generatedMaze::LevelConfigData(),
              avoidTexture(),
              numberAvoidObjects()
        {}

        LevelConfigData(LevelConfigData &&other) noexcept
            : generatedMaze::LevelConfigData{std::move(other)},
            avoidTexture{std::move(other.avoidTexture)},
            numberAvoidObjects{other.numberAvoidObjects}
        {}
    };
} // namespace avoidVortexMaze

#endif // AMAZING_LABYRINTH_AVOID_VORTEX_MAZE_LOAD_DATA_HPP