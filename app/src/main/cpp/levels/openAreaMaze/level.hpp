/**
 * Copyright 2022 Cerulean Quasar. All Rights Reserved.
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
#ifndef AMAZING_LABYRINTH_OPEN_AREA_MAZE_LEVEL_HPP
#define AMAZING_LABYRINTH_OPEN_AREA_MAZE_LEVEL_HPP

#include "../generatedMaze/level.hpp"

namespace openAreaMaze {
    class Level : public generatedMaze::Level {
    protected:
        virtual bool checkFinishCondition(float timeDiff) = 0;

        float leftWall(int col) {
            return m_width / (m_mazeBoard.numberColumns() * numberBlocksPerCell + 1) *
                   (col * numberBlocksPerCell + 0.5f) - m_width / 2;
        }

        float rightWall(int col) {
            return m_width / (m_mazeBoard.numberColumns() * numberBlocksPerCell + 1) *
                   ((col + 1) * numberBlocksPerCell + 0.5f) - m_width / 2;
        }

        float topWall(int row) {
            return m_height / (m_mazeBoard.numberRows() * numberBlocksPerCell + 1) *
                   (row * numberBlocksPerCell + 0.5f) - m_height / 2;
        }

        float bottomWall(int row) {
            return m_height / (m_mazeBoard.numberRows() * numberBlocksPerCell + 1) *
                   ((row + 1) * numberBlocksPerCell + 0.5f) - m_height / 2;
        }

    public:
        Level(levelDrawer::Adaptor inLevelDrawer,
                std::shared_ptr<generatedMaze::LevelConfigData> const &lcd,
                std::shared_ptr<generatedMaze::LevelSaveData> const &sd,
                float maxZ,
                std::string const &renderDetailsName = shadowsChainingRenderDetailsName)
                : generatedMaze::Level(
                        std::move(inLevelDrawer), lcd, sd, maxZ,
                        renderDetailsName,
                        getMazeWallModelMatricesGenerator())
                {}

        bool updateData() override;

    private:
        static generatedMaze::Level::MazeWallModelMatrixGeneratorFcn getMazeWallModelMatricesGenerator();
    };
} // namespace openAreaMaze
#endif // AMAZING_LABYRINTH_OPEN_AREA_MAZE_LEVEL_HPP