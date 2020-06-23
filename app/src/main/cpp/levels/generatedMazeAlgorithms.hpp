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
#ifndef AMAZING_LABYRINTH_GENERATED_MAZE_ALGORITHMS_HPP
#define AMAZING_LABYRINTH_GENERATED_MAZE_ALGORITHMS_HPP

#include <vector>
#include "../random.hpp"

class GeneratedMazeBoard;

class Cell {
    friend GeneratedMazeBoard;
private:
    bool mVisited;
    bool mIsStart;
    bool mIsEnd;
    bool mTopWallExists;
    bool mBottomWallExists;
    bool mLeftWallExists;
    bool mRightWallExists;
public:
    Cell() : mVisited(false), mIsStart(false), mIsEnd(false), mTopWallExists(true),
             mBottomWallExists(true), mLeftWallExists(true), mRightWallExists(true) {}
    ~Cell() = default;

    bool visited() const { return mVisited; }
    bool isStart() const { return mIsStart; }
    bool isEnd() const { return mIsEnd; }
    bool topWallExists() const { return mTopWallExists; }
    bool bottomWallExists() const { return mBottomWallExists; }
    bool leftWallExists() const { return mLeftWallExists; }
    bool rightWallExists() const { return mRightWallExists; }
};

class GeneratedMazeBoard {
public:
    enum WallType {
        rightWall = 0,
        topWall,
        leftWall,
        bottomWall
    };

    enum Mode {
        none = 0,
        BFS = 1,
        DFS = 2
    };

    size_t rowEnd() { return m_rowEnd; }
    size_t colEnd() { return m_colEnd; }
    size_t rowStart() { return m_rowStart; }
    size_t colStart() { return m_colStart; }
    size_t numberRows() { return m_cells.size(); }
    size_t numberColumns() { return m_cells.size() > 0 ? m_cells[0].size() : 0; }
    Cell const &getCell(unsigned int row, unsigned int column);

    void setEnd(size_t row, size_t col) {
        m_cells[row][col].mIsEnd = true;
        m_rowEnd = row;
        m_colEnd = col;
    }

    void setStart(size_t row, size_t col) {
        m_cells[row][col].mIsStart = true;
        m_rowStart = row;
        m_colStart = col;
    }

    bool wallExists(size_t row, size_t col, WallType wall) {
        switch (wall) {
            case WallType::rightWall:
                return m_cells[row][col].mRightWallExists;
            case WallType::topWall:
                return m_cells[row][col].mTopWallExists;
            case WallType::leftWall:
                return m_cells[row][col].mLeftWallExists;
            case WallType::bottomWall:
            default:
                return m_cells[row][col].mBottomWallExists;
        }
    }

    void setWall(size_t row, size_t col, WallType wall, bool isSet) {
        switch (wall) {
            case WallType::rightWall:
                m_cells[row][col].mRightWallExists = isSet;
                break;
            case WallType::topWall:
                m_cells[row][col].mTopWallExists = isSet;
                break;
            case WallType::leftWall:
                m_cells[row][col].mLeftWallExists = isSet;
                break;
            case WallType::bottomWall:
            default:
                m_cells[row][col].mBottomWallExists = isSet;
                break;
        }
    }

    GeneratedMazeBoard(size_t numberRows, size_t numberColumns, Mode mode)
        : m_rowEnd{0},
          m_colEnd{0},
          m_rowStart{0},
          m_colStart{0}
    {
        m_cells.resize(numberRows);

        for (unsigned int i = 0; i < numberRows; i++) {
            m_cells[i].resize(numberColumns);
        }

        if (mode == BFS) {
            generateBFS();
        } else if (mode == DFS){
            generateDFS();
        }

    }
private:
    Random m_random;
    std::vector<std::vector<Cell>> m_cells;

    size_t m_rowEnd;
    size_t m_colEnd;
    size_t m_rowStart;
    size_t m_colStart;

    void generateBFS();
    void generateDFS();
    void addCellOption(unsigned int r, unsigned int c, std::vector<std::pair<unsigned int, unsigned int> > &options);
};

#endif // AMAZING_LABYRINTH_GENERATED_MAZE_ALGORITHMS_HPP
