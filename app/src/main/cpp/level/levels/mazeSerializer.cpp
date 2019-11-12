/**
 * Copyright 2019 Cerulean Quasar. All Rights Reserved.
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
#include "maze.hpp"
#include "../../serializeSaveDataInternals.hpp"

char constexpr const *NbrRows = "NbrRows";
char constexpr const *BallRow = "BallRow";
char constexpr const *BallCol = "BallCol";
char constexpr const *EndRow = "EndRow";
char constexpr const *EndCol = "EndCol";
char constexpr const *MazeWalls = "MazeWalls";
char constexpr const *WallTextures = "WallTextures";
char constexpr const *BallPos = "BallPos";
void to_json(nlohmann::json &j, MazeSaveData const &val) {
    to_json(j, boost::implicit_cast<LevelSaveData const &>(val));
    j[NbrRows] = val.nbrRows;
    j[BallRow] = val.ballRow;
    j[BallCol] = val.ballCol;
    j[BallPos] = val.ballPos;
    j[EndRow] = val.rowEnd;
    j[EndCol] = val.colEnd;
    j[WallTextures] = val.wallTextures;
    j[MazeWalls] = val.mazeWallsVector;
}

void from_json(nlohmann::json const &j, MazeSaveData &val) {
    from_json(j, boost::implicit_cast<LevelSaveData&>(val));
    val.nbrRows = j[NbrRows].get<uint32_t>();
    val.ballRow = j[BallRow].get<uint32_t>();
    val.ballCol = j[BallCol].get<uint32_t>();
    val.ballPos = j[BallPos].get<Point<float>>();
    val.rowEnd = j[EndRow].get<uint32_t>();
    val.colEnd = j[EndCol].get<uint32_t>();
    val.wallTextures = j[WallTextures].get<std::vector<uint32_t>>();
    val.mazeWallsVector = j[MazeWalls].get<std::vector<uint8_t>>();
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

std::vector<uint8_t> Maze::getSerializedMazeWallVector() {
    std::vector<uint8_t> mazeWallVector;
    size_t vectorSize = (numberRows * numberColumns)/2;
    bool odd = (numberRows * numberColumns) % 2 == 1;
    vectorSize += odd ? 1 : 0;

    mazeWallVector.reserve(vectorSize);
    uint8_t high = 0;
    uint8_t low = 0;
    bool upper = true;
    for (auto const &row : cells) {
        for (auto const &cell : row) {
            if (upper) {
                high = encodeCell(cell.mTopWallExists, cell.mRightWallExists,
                                  cell.mBottomWallExists, cell.mLeftWallExists);
                upper = false;
            } else {
                low = encodeCell(cell.mTopWallExists, cell.mRightWallExists,
                                 cell.mBottomWallExists, cell.mLeftWallExists);
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

void initializeCell(uint8_t nibble, bool &top, bool &right, bool &bottom, bool &left) {
    top = (nibble & 0x8U) != 0;
    right = (nibble & 0x4U) != 0;
    bottom = (nibble & 0x2U) != 0;
    left = (nibble & 0x1U) != 0;
}

void incrementRowCol(size_t nbrCols, size_t &i, size_t &j) {
    j++;
    if (j >= nbrCols) {
        j = 0;
        i++;
    }
}

void Maze::generateCellsFromMazeVector(std::vector<uint8_t> const &mazeVector) {
    size_t  i = 0;
    size_t  j = 0;
    for (auto word : mazeVector) {
        initializeCell(word >> 0x4U,
                cells[i][j].mTopWallExists,
                cells[i][j].mRightWallExists,
                cells[i][j].mBottomWallExists,
                cells[i][j].mLeftWallExists);
        incrementRowCol(numberColumns, i, j);
        if (i < numberRows && j < numberColumns) {
            initializeCell(word & 0xFU,
                           cells[i][j].mTopWallExists,
                           cells[i][j].mRightWallExists,
                           cells[i][j].mBottomWallExists,
                           cells[i][j].mLeftWallExists);
            incrementRowCol(numberColumns, i, j);
        } else {
            break;
        }
    }
}

Level::SaveLevelDataFcn Maze::getSaveLevelDataFcn() {
    auto sd = std::make_shared<MazeSaveData>(
            numberRows,
            ball.row,
            ball.col,
            Point<float>{ball.position.x, ball.position.y},
            m_rowEnd,
            m_colEnd,
            std::vector<uint32_t>(m_wallTextureIndices),
            getSerializedMazeWallVector());
    return {[sd](std::shared_ptr<GameSaveData> gsd) -> std::vector<uint8_t> {
        return saveGameData(gsd, sd);
    }};
}
