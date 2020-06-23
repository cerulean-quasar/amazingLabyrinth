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

#ifndef AMAZING_LABYRINTH_AVOID_VORTEX_OPEN_AREA_LOAD_DATA_HPP
#define AMAZING_LABYRINTH_AVOID_VORTEX_OPEN_AREA_LOAD_DATA_HPP

#include <vector>
#include <memory>
#include <utility>
#include "../../levelTracker/types.hpp"

#include "../basic/loadData.hpp"

namespace avoidVortexOpenArea {
    int constexpr saveDataVersion = 1;

    struct LevelSaveData : public basic::LevelSaveData {
        Point<float> ball;
        Point<float> hole;
        Point<float> startPos;
        std::vector<Point<float>> vortexes;

        LevelSaveData(LevelSaveData &&other) noexcept = default;

        LevelSaveData(LevelSaveData &other) noexcept = default;

        LevelSaveData &operator=(LevelSaveData const &other) noexcept = default;

        LevelSaveData()
                : basic::LevelSaveData{saveDataVersion},
                  ball{0.0f, 0.0f},
                  hole{0.0f, 0.0f},
                  startPos{0.0f, 0.0f},
                  vortexes{} {
        }

        LevelSaveData(
                Point<float> &&ball_,
                Point<float> &&hole_,
                Point<float> &&startPos_,
                std::vector<Point<float>> &&vortexes_)
                : basic::LevelSaveData{saveDataVersion},
                  ball{ball_},
                  hole{hole_},
                  startPos{startPos_},
                  vortexes{std::move(vortexes_)} {
        }
    };

    struct LevelConfigData : public basic::LevelConfigData{
        std::string holeTexture;
        std::string vortexTexture;
        std::string startVortexTexture;

        LevelConfigData()
            : basic::LevelConfigData{},
            holeTexture{},
            vortexTexture{},
            startVortexTexture{}
        {
        }

        LevelConfigData(LevelConfigData const &other) noexcept = default;

        LevelConfigData(LevelConfigData &&other) noexcept = default;

        LevelConfigData &operator=(LevelConfigData const &other) noexcept = default;
    };
} // namespace avoidVortexOpenArea

#endif // AMAZING_LABYRINTH_AVOID_VORTEX_OPEN_AREA_LOAD_DATA_HPP
