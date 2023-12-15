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
#include <memory>
#include <json.hpp>
#include <boost/implicit_cast.hpp>
#include "level.hpp"
#include "../basic/serializer.hpp"
#include "../../levelTracker/internals.hpp"
#include "serializer.hpp"

namespace generatedMaze {
    char constexpr const *BallRow = "BallRow";
    char constexpr const *BallCol = "BallCol";
    char constexpr const *EndRow = "EndRow";
    char constexpr const *EndCol = "EndCol";
    char constexpr const *MazeWalls = "MazeWalls";
    char constexpr const *WallTextures = "WallTextures";
    char constexpr const *BallPos = "BallPos";

    void to_json(nlohmann::json &j, LevelSaveData const &val) {
        to_json(j, boost::implicit_cast<basic::LevelSaveData const &>(val));
        j[BallRow] = val.ballRow;
        j[BallCol] = val.ballCol;
        j[BallPos] = val.ballPos;
        j[EndRow] = val.rowEnd;
        j[EndCol] = val.colEnd;
        j[WallTextures] = val.wallTextures;
        j[MazeWalls] = val.mazeWallsVector;
    }

    void from_json(nlohmann::json const &j, LevelSaveData &val) {
        from_json(j, boost::implicit_cast<basic::LevelSaveData &>(val));
        val.ballRow = j[BallRow].get<uint32_t>();
        val.ballCol = j[BallCol].get<uint32_t>();
        val.ballPos = j[BallPos].get<Point<float>>();
        val.rowEnd = j[EndRow].get<uint32_t>();
        val.colEnd = j[EndCol].get<uint32_t>();
        val.wallTextures = j[WallTextures].get<std::vector<uint32_t>>();
        val.mazeWallsVector = j[MazeWalls].get<std::vector<uint8_t>>();
    }

    char constexpr const *NumberRows = "NumberRows";
    char constexpr const *DfsSearch = "DfsSearch";

    void to_json(nlohmann::json &j, LevelConfigData const &val) {
        to_json(j, boost::implicit_cast<basic::LevelConfigData const &>(val));
        j[NumberRows] = val.numberRows;
        j[DfsSearch] = val.dfsSearch;
    }

    void from_json(nlohmann::json const &j, LevelConfigData &val) {
        from_json(j, boost::implicit_cast<basic::LevelConfigData &>(val));
        val.numberRows = j[NumberRows].get<uint32_t>();
        val.dfsSearch = j[DfsSearch].get<bool>();
    }

    uint8_t encodeCell(bool top, bool right, bool bottom, bool left) {
        uint8_t word = 0;
        if (top) {
            word |= 0x8U;
        }
        if (right) {
            word |= 0x4U;
        }
        if (bottom) {
            word |= 0x2U;
        }
        if (left) {
            word |= 0x1U;
        }
        return word;
    }

    std::vector<uint8_t> Level::getSerializedMazeWallVector() {
        std::vector<uint8_t> mazeWallVector;
        size_t vectorSize = (m_mazeBoard.numberRows() * m_mazeBoard.numberColumns()) / 2;
        bool odd = (m_mazeBoard.numberRows() * m_mazeBoard.numberColumns()) % 2 == 1;
        vectorSize += odd ? 1 : 0;

        mazeWallVector.reserve(vectorSize);
        uint8_t high = 0;
        uint8_t low = 0;
        bool upper = true;
        for (size_t i = 0; i < m_mazeBoard.numberRows(); i++) {
            for (size_t j = 0; j < m_mazeBoard.numberColumns(); j++) {
                if (upper) {
                    high = encodeCell(
                            m_mazeBoard.wallExists(i, j, GeneratedMazeBoard::WallType::topWall),
                            m_mazeBoard.wallExists(i, j, GeneratedMazeBoard::WallType::rightWall),
                            m_mazeBoard.wallExists(i, j, GeneratedMazeBoard::WallType::bottomWall),
                            m_mazeBoard.wallExists(i, j, GeneratedMazeBoard::WallType::leftWall));
                    upper = false;
                } else {
                    low = encodeCell(
                            m_mazeBoard.wallExists(i, j, GeneratedMazeBoard::WallType::topWall),
                            m_mazeBoard.wallExists(i, j, GeneratedMazeBoard::WallType::rightWall),
                            m_mazeBoard.wallExists(i, j, GeneratedMazeBoard::WallType::bottomWall),
                            m_mazeBoard.wallExists(i, j, GeneratedMazeBoard::WallType::leftWall));
                    mazeWallVector.push_back((high << 0x4U) | low);
                    upper = true;
                }
            }
            if (odd && mazeWallVector.size() == vectorSize - 1) {
                mazeWallVector.push_back(high << 0x4U);
            }
        }
        return mazeWallVector;
    }

    void Level::initializeCell(uint8_t nibble, size_t row, size_t col) {
        m_mazeBoard.setWall(row, col, GeneratedMazeBoard::WallType::topWall, (nibble & 0x8U) != 0);
        m_mazeBoard.setWall(row, col, GeneratedMazeBoard::WallType::rightWall,
                            (nibble & 0x4U) != 0);
        m_mazeBoard.setWall(row, col, GeneratedMazeBoard::WallType::bottomWall,
                            (nibble & 0x2U) != 0);
        m_mazeBoard.setWall(row, col, GeneratedMazeBoard::WallType::leftWall, (nibble & 0x1U) != 0);
    }

    void incrementRowCol(size_t nbrCols, size_t &i, size_t &j) {
        j++;
        if (j >= nbrCols) {
            j = 0;
            i++;
        }
    }

    void Level::generateCellsFromMazeVector(std::vector<uint8_t> const &mazeVector) {
        size_t i = 0;
        size_t j = 0;
        size_t numberRows = m_mazeBoard.numberRows();
        size_t numberColumns = m_mazeBoard.numberColumns();
        for (auto word : mazeVector) {
            initializeCell(word >> 0x4U, i, j);
            incrementRowCol(numberColumns, i, j);
            if (i < numberRows && j < numberColumns) {
                initializeCell(word & 0xFU, i, j);
                incrementRowCol(numberColumns, i, j);
            } else {
                break;
            }
        }
    }

    std::vector<uint8_t> Level::saveData(levelTracker::GameSaveData const &gsd,
                                  char const *saveLevelDataKey) {
        LevelSaveData sd(
                m_ballCell.row,
                m_ballCell.col,
                Point<float>{m_ball.position.x, m_ball.position.y},
                m_mazeBoard.rowEnd(),
                m_mazeBoard.colEnd(),
                std::vector<uint32_t>(m_wallTextureIndices),
                getSerializedMazeWallVector());
        nlohmann::json j;
        to_json(j, gsd);
        j[saveLevelDataKey] = sd;
        return nlohmann::json::to_cbor(j);
    }

    levelTracker::Register<levelTracker::LevelMapTable, levelTracker::levelTable, LevelConfigData, LevelSaveData, Level> registerLevel;
}