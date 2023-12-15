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
#ifndef AMAZING_LABYRINTH_OPEN_AREA_LOAD_DATA_HPP
#define AMAZING_LABYRINTH_OPEN_AREA_LOAD_DATA_HPP

#include "../basic/loadData.hpp"

namespace openArea {
    int constexpr levelSaveDataVersion = 1;

    struct LevelSaveData : public basic::LevelSaveData {
        Point<float> ball;
        Point<float> hole;

        LevelSaveData(LevelSaveData &&other) noexcept = default;

        LevelSaveData(LevelSaveData const &other) noexcept = default;

        LevelSaveData &operator=(LevelSaveData const &other) noexcept = default;

        LevelSaveData()
                : basic::LevelSaveData{levelSaveDataVersion},
                  ball{0.0f, 0.0f},
                  hole{0.0f, 0.0f} {
        }

        LevelSaveData(Point<float> ball_, Point<float> hole_)
                : basic::LevelSaveData{levelSaveDataVersion},
                  ball{std::move(ball_)},
                  hole{std::move(hole_)} {
        }
    };
} // namespace openArea

#endif // AMAZING_LABYRINTH_OPEN_AREA_LOAD_DATA_HPP