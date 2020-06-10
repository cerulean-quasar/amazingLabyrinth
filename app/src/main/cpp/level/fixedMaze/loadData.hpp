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
        LevelSaveData() : basic::LevelSaveData{LevelSaveDataVersion} {}
    };

    struct LevelConfigData : public basic::LevelConfigData {
        std::string m_mazeFloorModel;
        std::string m_mazeFloorTexture;
        float m_extraBounce;
        float m_minSpeedOnBounce;

        LevelConfigData()
                : basic::LevelConfigData(),
                m_mazeFloorModel{},
                m_mazeFloorTexture{},
                m_extraBounce{0.0f},
                m_minSpeedOnBounce{0.0f}
        {
        }

        LevelConfigData(LevelConfigData &&other) noexcept
                : basic::LevelConfigData(),
                  m_mazeFloorModel{std::move(other.m_mazeFloorModel)},
                  m_mazeFloorTexture{std::move(other.m_mazeFloorTexture)},
                  m_extraBounce{other.m_extraBounce},
                  m_minSpeedOnBounce{other.m_minSpeedOnBounce}
        {
        }

        LevelConfigData(
                std::string ballModel,
                std::string ballTexture,
                float ballSizeDiagonalRatio,
                std::string mazeFloorModel,
                std::string mazeFloorTexture,
                bool bounceEnabled,
                float extraBounce,
                float minSpeedOnBounce) noexcept
                : basic::LevelConfigData(ballModel, ballTexture, bounceEnabled, ballSizeDiagonalRatio),
                  m_mazeFloorModel{std::move(mazeFloorModel)},
                  m_mazeFloorTexture{std::move(mazeFloorTexture)},
                  m_extraBounce{extraBounce},
                  m_minSpeedOnBounce{minSpeedOnBounce}
        {
        }

    };
} // namespace fixedMaze

#endif // AMAZING_LABYRINTH_FIXED_MAZE_LOAD_DATA_HPP