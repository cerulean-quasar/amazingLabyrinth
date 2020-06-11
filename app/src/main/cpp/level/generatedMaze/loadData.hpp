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

#ifndef AMAZING_LABYRINTH_GENERATED_MAZE_LOAD_DATA_HPP
#define AMAZING_LABYRINTH_GENERATED_MAZE_LOAD_DATA_HPP

#include <vector>

#include "../../saveData.hpp"

#include "../basic/loadData.hpp"
namespace generatedMaze {
    int constexpr levelSaveDataVersion = 1;
    struct LevelSaveData : public basic::LevelSaveData {
        uint32_t ballRow;
        uint32_t ballCol;
        Point<float> ballPos;
        uint32_t rowEnd;
        uint32_t colEnd;
        std::vector<uint32_t> wallTextures;
        std::vector<uint8_t> mazeWallsVector;

        LevelSaveData(LevelSaveData &&other) noexcept
                : basic::LevelSaveData{std::move(other)},
                  ballRow{other.ballRow},
                  ballCol{other.ballCol},
                  ballPos{other.ballPos},
                  rowEnd{other.rowEnd},
                  colEnd{other.colEnd},
                  wallTextures{other.wallTextures},
                  mazeWallsVector{std::move(other.mazeWallsVector)} {
        }

        LevelSaveData()
                : basic::LevelSaveData{levelSaveDataVersion},
                  ballRow{0},
                  ballCol{0},
                  ballPos{0.0f, 0.0f},
                  rowEnd{0},
                  colEnd{0},
                  wallTextures{},
                  mazeWallsVector{} {
        }

        LevelSaveData(
                uint32_t nbrRows_,
                uint32_t ballRow_,
                uint32_t ballCol_,
                Point<float> &&ballPos_,
                uint32_t rowEnd_,
                uint32_t colEnd_,
                std::vector<uint32_t> wallTextures_,
                std::vector<uint8_t> mazeWallsVector_)
                : basic::LevelSaveData{levelSaveDataVersion},
                  ballRow{ballRow_},
                  ballCol{ballCol_},
                  ballPos{ballPos_},
                  rowEnd{rowEnd_},
                  colEnd{colEnd_},
                  wallTextures{std::move(wallTextures_)},
                  mazeWallsVector{std::move(mazeWallsVector_)} {
        }
    };

    struct LevelConfigData {
        std::vector<std::string> m_wallTextureNames;
        std::string m_mazeFloorTexture;
        std::string m_holeTexture;
        uint32_t m_numberRows;
        bool m_dfsSearch;

        LevelConfigData(LevelConfigData &&other) noexcept
            : basic::LevelConfigData{std::move(other)},
              m_wallTextureNames{std::move(other.m_wallTextureNames)},
              m_mazeFloorTexture{std::move(other.m_mazeFloorTexture)},
              m_holeTexture{std::move(other.m_holeTexture)},
              m_numberRows{other.m_numberRows},
              m_dfsSearch{other.m_dfsSearch}
        {}

        LevelConfigData()
                : basic::LevelConfigData{},
                  m_wallTextureNames{},
                  m_mazeFloorTexture{},
                  m_holeTexture{},
                  m_numberRows{0},
                  m_dfsSearch{false}
        {}

        LevelConfigData(
                std::vector<std::string> wallTextureNames_,
                std::string mazeFloorTexture_,
                std::string holeTexture_,
                uint32_t numberRows_,
                bool dfsSearch_)
                : basic::LevelConfigData{},
                  m_wallTextureNames{std::move(wallTextureNames_)},
                  m_mazeFloorTexture{std::move(mazeFloorTexture_)},
                  m_holeTexture{std::move(holeTexture_)},
                  m_numberRows{numberRows_},
                  m_dfsSearch{dfsSearch_}
        {}
    };
}
#endif // AMAZING_LABYRINTH_GENERATED_MAZE_LOAD_DATA_HPP
