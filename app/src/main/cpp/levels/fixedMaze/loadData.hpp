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

#ifndef AMAZING_LABYRINTH_FIXED_MAZE_LOAD_DATA_HPP
#define AMAZING_LABYRINTH_FIXED_MAZE_LOAD_DATA_HPP

#include "../basic/loadData.hpp"

namespace fixedMaze {
    static int constexpr LevelSaveDataVersion = 1;
    struct LevelSaveData : public basic::LevelSaveData {
        LevelSaveData()
            : basic::LevelSaveData{LevelSaveDataVersion}
        {}

        LevelSaveData(LevelSaveData &&other) noexcept = default;

        LevelSaveData(LevelSaveData const &other) noexcept = default;

        LevelSaveData &operator=(LevelSaveData const &other) noexcept = default;
    };

    struct LevelConfigData : public basic::LevelConfigData {
        std::string mazeFloorModel;
        std::string mazeFloorTexture;
        float extraBounce;
        float minSpeedOnBounce;

        LevelConfigData()
                : basic::LevelConfigData(),
                mazeFloorModel{},
                mazeFloorTexture{},
                extraBounce{0.0f},
                minSpeedOnBounce{0.0f}
        {
        }

        LevelConfigData(LevelConfigData const &other) noexcept = default;

        LevelConfigData(LevelConfigData &&other) noexcept = default;

        LevelConfigData &operator=(LevelConfigData const &other) noexcept = default;
    };
} // namespace fixedMaze

#endif // AMAZING_LABYRINTH_FIXED_MAZE_LOAD_DATA_HPP