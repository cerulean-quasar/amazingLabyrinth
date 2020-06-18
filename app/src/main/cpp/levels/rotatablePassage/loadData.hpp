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

#ifndef AMAZING_LABYRINTH_ROTATABLE_PASSAGE_LOAD_DATA_HPP
#define AMAZING_LABYRINTH_ROTATABLE_PASSAGE_LOAD_DATA_HPP

#include "../basic/loadData.hpp"

namespace rotatablePassage {
    static int constexpr levelSaveDataVersion = 1;
    struct LevelSaveData : public basic::LevelSaveData {
        LevelSaveData() : basic::LevelSaveData{levelSaveDataVersion} {}
    };

    struct ComponentConfig {
        std::string model;
        std::string texture;
        std::string lockedInPlaceTexture;
    };

    struct LevelConfigData : public basic::LevelConfigData {
        std::string holeModel;
        std::string holeTexture;
        uint32_t numberRows;
        bool dfsSearch;
        std::vector<std::string> borderTextures;
        ComponentConfig straight;
        ComponentConfig turn;
        ComponentConfig crossJunction;
        ComponentConfig tJunction;
        ComponentConfig deadEnd;

        LevelConfigData()
            : basic::LevelConfigData{},
            numberRows{0},
            dfsSearch{false}
        {}

        LevelConfigData(LevelConfigData const &other) noexcept = default;

        LevelConfigData(LevelConfigData &&other) noexcept = default;

        LevelConfigData &operator=(LevelConfigData const &other) noexcept = default;
    };
} // name rotatablePassage
#endif // AMAZING_LABYRINTH_ROTATABLE_PASSAGE_LOAD_DATA_HPP
