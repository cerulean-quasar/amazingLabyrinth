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
#ifndef AMAZING_LABYRINTH_STARTER_LOAD_DATA_HPP
#define AMAZING_LABYRINTH_STARTER_LOAD_DATA_HPP

#include <string>
#include <vector>

#include "../basic/loadData.hpp"
#include "../../saveData.hpp"

namespace starter {
    int constexpr levelSaveDataVersion = 1;

    struct LevelSaveData : public basic::LevelSaveData {
        LevelSaveData(LevelSaveData &&other) noexcept
                : basic::LevelSaveData{levelSaveDataVersion}
        {}

        LevelSaveData()
                : basic::LevelSaveData{levelSaveDataVersion}
        {}
    };

    struct LevelConfigData : public basic::LevelConfigData {
        std::vector<std::string> startupMessages;

        LevelConfigData()
                : basic::LevelConfigData{},
                  startupMessages{}
        {}

        LevelConfigData(LevelConfigData &&other)
                : basic::LevelConfigData{std::move(other)},
                  startupMessages{std::move(other.startupMessages)}
        {}
    };
} // namespace starter

#endif // AMAZING_LABYRINTH_STARTER_LOAD_DATA_HPP
