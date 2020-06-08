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

#include <list>

#include "generatedMazeAlgorithms.hpp"

void GeneratedMazeBoard::generateDFS() {
    std::vector<std::pair<unsigned int, unsigned int> > path;
    unsigned int rowStart, columnStart;
    unsigned int rowEnd, columnEnd;

    // the desired distance that they are apart is the average of the number of rows and the number
    // of columns divided by 2.
    unsigned int desiredDistanceSquared = (numberRows()+numberColumns())*(numberRows()+numberColumns())/16;
    unsigned int distanceSquared;
    do {
        rowStart = m_random.getUInt(0, numberRows()-1);
        columnStart = m_random.getUInt(0, numberColumns()-1);
        rowEnd = m_random.getUInt(0, numberRows()-1);
        columnEnd = m_random.getUInt(0, numberColumns()-1);
        distanceSquared = (rowEnd - rowStart)*(rowEnd - rowStart) +
                          (columnEnd - columnStart)*(columnEnd - columnStart);
    } while (distanceSquared < desiredDistanceSquared);

    m_cells[rowStart][columnStart].mIsStart = true;
    m_cells[rowEnd][columnEnd].mIsEnd = true;
    m_rowEnd = rowEnd;
    m_colEnd = columnEnd;
    m_rowStart = rowStart;
    m_colStart = columnStart;

    m_cells[rowStart][columnStart].mVisited = true;
    path.push_back(std::make_pair(rowStart, columnStart));

    while (!path.empty()) {
        std::pair<unsigned int, unsigned int> current = path.back();
        if (m_cells[current.first][current.second].isEnd()) {
            path.pop_back();
            continue;
        }

        std::vector<std::pair<unsigned int, unsigned int> > nextCellOptions;
        addCellOption(current.first-1, current.second, nextCellOptions);
        addCellOption(current.first, current.second-1, nextCellOptions);
        addCellOption(current.first+1, current.second, nextCellOptions);
        addCellOption(current.first, current.second+1, nextCellOptions);

        if (nextCellOptions.empty()) {
            path.pop_back();
            continue;
        }
        unsigned int i = m_random.getUInt(0, nextCellOptions.size() - 1);

        std::pair<unsigned int, unsigned int> next = nextCellOptions[i];

        if (current.first == next.first) {
            if (next.second < current.second) {
                // the new cell is on the left of the current one.
                m_cells[current.first][current.second].mLeftWallExists = false;
                m_cells[next.first][next.second].mRightWallExists = false;
            } else {
                m_cells[current.first][current.second].mRightWallExists = false;
                m_cells[next.first][next.second].mLeftWallExists = false;
            }
        } else {
            if (next.first < current.first) {
                // the new cell is on the top of the current one.
                m_cells[current.first][current.second].mTopWallExists = false;
                m_cells[next.first][next.second].mBottomWallExists = false;
            } else {
                m_cells[current.first][current.second].mBottomWallExists = false;
                m_cells[next.first][next.second].mTopWallExists = false;
            }
        }

        m_cells[next.first][next.second].mVisited = true;
        path.push_back(next);
    }
}

void GeneratedMazeBoard::generateBFS() {
    std::list<std::pair<unsigned int, unsigned int> > path;
    unsigned int rowStart, columnStart;
    unsigned int rowEnd, columnEnd;

    // the desired distance that they are apart is the average of the number of rows and the number
    // of columns divided by 2.
    unsigned int desiredDistanceSquared = (numberRows()+numberColumns())*(numberRows()+numberColumns())/16;
    unsigned int distanceSquared;
    do {
        rowStart = m_random.getUInt(0, numberRows()-1);
        columnStart = m_random.getUInt(0, numberColumns()-1);
        rowEnd = m_random.getUInt(0, numberRows()-1);
        columnEnd = m_random.getUInt(0, numberColumns()-1);
        distanceSquared = (rowEnd - rowStart)*(rowEnd - rowStart) +
                          (columnEnd - columnStart)*(columnEnd - columnStart);
    } while (distanceSquared < desiredDistanceSquared);

    m_cells[rowStart][columnStart].mIsStart = true;
    m_cells[rowEnd][columnEnd].mIsEnd = true;
    m_rowEnd = rowEnd;
    m_colEnd = columnEnd;
    m_rowStart = rowStart;
    m_colStart = columnStart;

    m_cells[rowStart][columnStart].mVisited = true;
    path.push_back(std::make_pair(rowStart, columnStart));

    while (!path.empty()) {
        std::pair<unsigned int, unsigned int> current = path.front();
        path.pop_front();
        if (m_cells[current.first][current.second].isEnd()) {
            continue;
        }

        std::vector<std::pair<unsigned int, unsigned int> > nextCellOptions;
        addCellOption(current.first - 1, current.second, nextCellOptions);
        addCellOption(current.first, current.second - 1, nextCellOptions);
        addCellOption(current.first + 1, current.second, nextCellOptions);
        addCellOption(current.first, current.second + 1, nextCellOptions);

        if (nextCellOptions.empty()) {
            continue;
        }

        for (auto const &next : nextCellOptions) {
            if (current.first == next.first) {
                if (next.second < current.second) {
                    // the new cell is on the left of the current one.
                    m_cells[current.first][current.second].mLeftWallExists = false;
                    m_cells[next.first][next.second].mRightWallExists = false;
                } else {
                    m_cells[current.first][current.second].mRightWallExists = false;
                    m_cells[next.first][next.second].mLeftWallExists = false;
                }
            } else {
                if (next.first < current.first) {
                    // the new cell is on the top of the current one.
                    m_cells[current.first][current.second].mTopWallExists = false;
                    m_cells[next.first][next.second].mBottomWallExists = false;
                } else {
                    m_cells[current.first][current.second].mBottomWallExists = false;
                    m_cells[next.first][next.second].mTopWallExists = false;
                }
            }

            m_cells[next.first][next.second].mVisited = true;
            size_t size = path.size();
            unsigned int i = size == 0 ? 0 : m_random.getUInt(0, path.size());
            auto it = path.begin();
            for (unsigned int j = 0; j < i; j++, it++)
                /* do nothing */;
            path.insert(it, next);
        }
    }
}

void GeneratedMazeBoard::addCellOption(unsigned int r, unsigned int c, std::vector<std::pair<unsigned int, unsigned int> > &options) {
    if (r < numberRows() && c < numberColumns() && !m_cells[r][c].visited()) {
        options.push_back(std::make_pair(r, c));
    }
}

Cell const &GeneratedMazeBoard::getCell(unsigned int row, unsigned int column) {
    return m_cells[row][column];
}
