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

#ifndef AMAZING_LABYRINTH_MOVING_SAFE_AREAS_LOAD_DATA_HPP
#define AMAZING_LABYRINTH_MOVING_SAFE_AREAS_LOAD_DATA_HPP

#include <vector>
#include "../basic/loadData.hpp"

namespace movingSafeAreas {
    int constexpr levelSaveDataVersion = 1;

    struct QuadRowSaveData {
        std::vector<Point < float>> positions;
        float speed;
        Point<float> scale;

        QuadRowSaveData()
                : positions{},
                  speed{0.0f},
                  scale{0.0f, 0.0f} {}

        QuadRowSaveData(
            std::vector<Point < float>> &&positions_,
            float speed_,
            Point<float> &&scale_)
        : positions{ std::move(positions_) },
          speed{ speed_ },
          scale{ std::move(scale_) }
        {}

        QuadRowSaveData(
            std::vector<Point<float>> const &positions_,
            float speed_,
            Point<float> const &scale_)
        : positions{ positions_ },
          speed{ speed_ },
          scale{ scale_ }
        {}

        QuadRowSaveData(QuadRowSaveData &&other) noexcept = default;

        QuadRowSaveData(QuadRowSaveData const &other) noexcept = default;

        QuadRowSaveData &operator=(QuadRowSaveData const &other) noexcept = default;

    };

    struct LevelSaveData : public basic::LevelSaveData {
        Point<float> ball;
        std::vector<QuadRowSaveData> quadRows;

        LevelSaveData(LevelSaveData &&other) noexcept = default;

        LevelSaveData(LevelSaveData const &other) noexcept = default;

        LevelSaveData &operator=(LevelSaveData const &other) noexcept = default;

        LevelSaveData()
                : basic::LevelSaveData{levelSaveDataVersion},
                  ball{0.0f, 0.0f},
                  quadRows{} {
        }

        LevelSaveData(
                Point<float> &&ball_,
                std::vector<QuadRowSaveData> &&quadRows_)
                : basic::LevelSaveData{levelSaveDataVersion},
                  ball{ball_},
                  quadRows{std::move(quadRows_)} {
        }
    };
} // namespace movingSafeAreas

#endif // AMAZING_LABYRINTH_MOVING_SAFE_AREAS_LOAD_DATA_HPP