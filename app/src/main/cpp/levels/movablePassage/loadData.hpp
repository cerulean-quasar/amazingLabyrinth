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

#include <vector>
#include "../movablePassageAlgorithms.hpp"
#include "../basic/loadData.hpp"

namespace movablePassage {
    struct PlacementSaveData{
        Point<uint32_t> rowCol;
        uint32_t nbr90DegreeRotations;

        PlacementSaveData(uint32_t row, uint32_t col, uint32_t rotationNbr)
                : rowCol{row, col},
                  nbr90DegreeRotations{rotationNbr}
        {}

        PlacementSaveData()
            : rowCol{0, 0},
              nbr90DegreeRotations{0}
        {}

        PlacementSaveData(PlacementSaveData &&other) noexcept = default;

        PlacementSaveData(PlacementSaveData const &other) noexcept  = default;

        PlacementSaveData &operator=(PlacementSaveData const &other) noexcept = default;
    };

    struct LevelSaveData : public basic::LevelSaveData {
        static int constexpr m_movablePassageVersion = 1;

        std::vector<PlacementSaveData> straightPositions;
        std::vector<PlacementSaveData> tjunctionPositions;
        std::vector<PlacementSaveData> crossjunctionPositions;
        std::vector<PlacementSaveData> turnPositions;
        std::vector<Point<uint32_t>> pathLockedInPlace;
        std::vector<std::vector<ObjReference>> gameBoardObjReferences;
        Point<uint32_t> ballRC;
        Point<float> ballPosition;

        LevelSaveData() : basic::LevelSaveData{m_movablePassageVersion} {}

        LevelSaveData(LevelSaveData &&other) noexcept = default;

        LevelSaveData(LevelSaveData const &other) noexcept = default;

        LevelSaveData &operator=(LevelSaveData const &other) noexcept = default;
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

        ComponentConfig(ComponentConfig const &other) noexcept = default;

        ComponentConfig(ComponentConfig &&other) noexcept = default;

        ComponentConfig &operator=(ComponentConfig &&other) noexcept = default;

        ComponentConfig &operator=(ComponentConfig const &other) noexcept = default;
    };

    struct RockPlacement {
        uint32_t row;
        uint32_t col;

        RockPlacement()
            : row{0}, col{0}
        {}

        RockPlacement(RockPlacement const &other) noexcept = default;

        RockPlacement(RockPlacement &&other) noexcept = default;

        RockPlacement &operator=(RockPlacement &&other) noexcept = default;

        RockPlacement &operator=(RockPlacement const &other) noexcept = default;
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

        LevelConfigData()
            : basic::LevelConfigData{},
              numberTilesX{0},
              numberTilesY{0},
              startColumn{0},
              endColumn{0}
        {}

        LevelConfigData(LevelConfigData const &other) noexcept = default;

        LevelConfigData(LevelConfigData &&other) noexcept = default;

        LevelConfigData &operator=(LevelConfigData const &other) noexcept = default;
    };
} // namespace movablePassage

#endif // AMAZING_LABYRINTH_MOVABLE_PASSAGE_LOAD_DATA_HPP
