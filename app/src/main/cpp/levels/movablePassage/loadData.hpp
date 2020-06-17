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

#ifndef AMAZING_LABYRINTH_MOVABLE_PASSAGE_LOAD_DATA_HPP
#define AMAZING_LABYRINTH_MOVABLE_PASSAGE_LOAD_DATA_HPP

#include "../basic/loadData.hpp"

namespace movablePassage {
    struct LevelSaveData : public basic::LevelSaveData {
        static int constexpr m_movablePassageVersion = 1;

        LevelSaveData() : basic::LevelSaveData{m_movablePassageVersion} {}
    };

    struct ComponentConfig {
        std::string model;
        std::string texture;
        uint32_t numberPlacements;

        ComponentConfig()
            : model{},
              texture{},
              numberPlacements{0}
        {}

        ComponentConfig(ComponentConfig &&other) noexcept
            : model{std::move(other.model)},
              texture{std::move(other.texture)},
              numberPlacements{other.numberPlacements}
        {}

        ComponentConfig &operator=(ComponentConfig &&other) noexcept
        {
            model = std::move(other.model);
            texture = std::move(other.texture);
            numberPlacements = other.numberPlacements;
            return *this;
        }
    };

    struct RockPlacement {
        uint32_t row;
        uint32_t col;

        RockPlacement()
            : row{}, col{}
        {}

        RockPlacement(RockPlacement &&other) noexcept
            : row{other.row}, col{other.col}
        {}

        RockPlacement &operator=(RockPlacement &&other) noexcept {
            row = other.row;
            col = other.col;

            return *this;
        }
    };

    struct LevelConfigData : public basic::LevelConfigData {
        uint32_t numberTilesX;
        uint32_t numberTilesY;
        uint32_t startColumn;
        uint32_t endColumn;
        std::string endTexture;
        std::string endOffBoardTexture;
        std::string placementLockedInPlaceTexture;
        std::vector<std::string> rockModels;
        std::vector<std::string> rockTextures;
        std::vector<std::string> dirtModels;
        std::vector<std::string> dirtTextures;
        std::vector<std::string> beginningSideModels;
        std::vector<std::string> beginningSideTextures;
        std::vector<std::string> beginningOpenModels;
        std::vector<std::string> beginningOpenTextures;
        std::vector<std::string> beginningCornerModels;
        std::vector<std::string> beginningCornerTextures;
        ComponentConfig straight;
        ComponentConfig turn;
        ComponentConfig crossjunction;
        ComponentConfig tjunction;
        std::vector<RockPlacement> rockPlacements;

        LevelConfigData() {}

        LevelConfigData(LevelConfigData &&other) noexcept
            : endTexture{std::move(other.endTexture)},
              endOffBoardTexture{std::move(other.endOffBoardTexture)},
              placementLockedInPlaceTexture{std::move(other.placementLockedInPlaceTexture)},
              rockModels{std::move(other.rockModels)},
              rockTextures{std::move(other.rockTextures)},
              dirtModels{std::move(other.dirtModels)},
              dirtTextures{std::move(other.dirtTextures)},
              beginningSideModels{std::move(other.beginningSideModels)},
              beginningSideTextures{std::move(other.beginningSideTextures)},
              beginningOpenModels{std::move(other.beginningOpenModels)},
              beginningOpenTextures{std::move(other.beginningOpenTextures)},
              beginningCornerModels{std::move(other.beginningCornerModels)},
              beginningCornerTextures{std::move(other.beginningCornerTextures)},
              straight{std::move(other.straight)},
              turn{std::move(other.turn)},
              crossjunction{std::move(other.crossjunction)},
              tjunction{std::move(other.tjunction)},
              rockPlacements{std::move(other.rockPlacements)}
        {}
    };
} // namespace movablePassage

#endif // AMAZING_LABYRINTH_MOVABLE_PASSAGE_LOAD_DATA_HPP
