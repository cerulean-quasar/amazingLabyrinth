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
#ifndef AMAZING_LABYRINTH_MAZE_OPEN_AREA_HPP
#define AMAZING_LABYRINTH_MAZE_OPEN_AREA_HPP

#include "maze.hpp"

class MazeOpenArea : public Maze {
protected:
    virtual bool checkFinishCondition(float timeDiff) = 0;

    float leftWall(int col) {
        return m_width / (numberColumns * numberBlocksPerCell+1) * (col*numberBlocksPerCell+0.5f) - m_width/2;
    }

    float rightWall(int col) {
        return m_width / (numberColumns * numberBlocksPerCell+1) * ((col+1)*numberBlocksPerCell+0.5f) - m_width/2;
    }

    float topWall(int row) {
        return m_height / (numberRows * numberBlocksPerCell+1) * (row*numberBlocksPerCell+0.5f) - m_height/2;
    }

    float bottomWall(int row) {
        return m_height / (numberRows * numberBlocksPerCell+1) * ((row+1)*numberBlocksPerCell+0.5f) - m_height/2;
    }

public:
    MazeOpenArea(std::shared_ptr<GameRequester> inGameRequester, std::shared_ptr<MazeSaveData> sd,
                 float width, float height, float maxZ)
            : Maze(std::move(inGameRequester), std::move(sd), width, height, maxZ, getMazeWallModelMatricesGenerator())
    {}

    MazeOpenArea(std::shared_ptr<GameRequester> inGameRequester, Maze::CreateParameters const &parameters,
                 float width, float height, float maxZ)
            : Maze(std::move(inGameRequester), parameters, width, height, maxZ, getMazeWallModelMatricesGenerator())
    {}

    bool updateData() override;
    SaveLevelDataFcn getSaveLevelDataFcn() override = 0;

private:
    static Maze::MazeWallModelMatrixGeneratorFcn getMazeWallModelMatricesGenerator();
};

#endif /* AMAZING_LABYRINTH_MAZE_OPEN_AREA_HPP */