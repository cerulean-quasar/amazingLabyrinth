/**
 * Copyright 2018 Cerulean Quasar. All Rights Reserved.
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
#ifndef AMAZING_LABYRINTH_MAZE_OPEN_AREA_HPP
#define AMAZING_LABYRINTH_MAZE_OPEN_AREA_HPP

#include "maze.hpp"

class MazeOpenArea : public Maze {
protected:
    uint32_t m_rowEnd;
    uint32_t m_colEnd;
    virtual void generateModelMatrices();
    virtual bool checkFinishCondition(float timeDiff) = 0;

    float leftWall(int col) {
        return m_width / (numberColumns * numberBlocksPerCell) * col*numberBlocksPerCell - m_width/2;
    }

    float rightWall(int col) {
        return m_width / (numberColumns * numberBlocksPerCell) * (col+1)*numberBlocksPerCell - m_width/2;
    }

    float topWall(int row) {
        return m_height / (numberRows * numberBlocksPerCell) * row*numberBlocksPerCell - m_height/2;
    }

    float bottomWall(int row) {
        return m_height / (numberRows * numberBlocksPerCell) * (row+1)*numberBlocksPerCell - m_height/2;
    }

public:
    MazeOpenArea(unsigned int inNumberRows, Mode inMode, uint32_t width, uint32_t height)
            : Maze(inNumberRows, inMode, width, height),
              m_rowEnd{},
              m_colEnd{}
    {}

    virtual bool updateData();
};

#endif /* AMAZING_LABYRINTH_MAZE_OPEN_AREA_HPP */