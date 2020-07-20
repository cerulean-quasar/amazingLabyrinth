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

#include <vector>

#include "level.hpp"

namespace avoidVortexMaze {
    char constexpr const *Level::m_name;

    bool Level::checkFinishCondition(float) {
        for (auto avoidObj : m_avoidObjectLocations) {
            if (ballInProximity(avoidObj.x, avoidObj.y)) {
                // send the ball back to the start because it hit one of the objects it was supposed
                // to avoid.
                m_ballCell.row = m_mazeBoard.rowStart();
                m_ballCell.col = m_mazeBoard.colStart();
                m_ball.position = getCellCenterPosition(m_ballCell.row, m_ballCell.col);
            }
        }

        // we finished the level if all the items are collected and the ball is in proximity to the hole.
        return ballInProximity(getColumnCenterPosition(m_mazeBoard.colEnd()),
                               getRowCenterPosition(m_mazeBoard.rowEnd()));
    }

    void Level::generateAvoidModelMatrices(uint32_t numberAvoidObjects) {
        // generate the items to avoid
        while (m_avoidObjectLocations.size() < numberAvoidObjects) {
            uint32_t row = random.getUInt(0, m_mazeBoard.numberRows() - 1);
            uint32_t col = random.getUInt(0, m_mazeBoard.numberColumns() - 1);

            glm::vec3 pos = getCellCenterPosition(row, col);
            int wall = random.getUInt(0, 3);
            bool selected = false;

            std::vector<std::pair<uint32_t, uint32_t>> itemsRC;
            itemsRC.emplace_back(m_mazeBoard.rowEnd(), m_mazeBoard.colEnd());
            itemsRC.emplace_back(m_mazeBoard.rowStart(), m_mazeBoard.colStart());
            while (!selected) {
                switch (wall) {
                    case 0:
                        if (m_mazeBoard.wallExists(row, col,
                                                   GeneratedMazeBoard::WallType::leftWall)) {
                            pos.x = leftWall(col) + ballRadius();
                            selected = true;
                            break;
                        }
                        // fall through and just try to see if the next wall exists and put the object
                        // there if it does.
                    case 1:
                        if (m_mazeBoard.wallExists(row, col,
                                                   GeneratedMazeBoard::WallType::rightWall)) {
                            pos.x = rightWall(col) - ballRadius();
                            selected = true;
                            break;
                        }
                    case 2:
                        if (m_mazeBoard.wallExists(row, col,
                                                   GeneratedMazeBoard::WallType::topWall)) {
                            pos.y = topWall(row) + ballRadius();
                            selected = true;
                            break;
                        }
                    case 3:
                    default:
                        if (m_mazeBoard.wallExists(row, col,
                                                   GeneratedMazeBoard::WallType::bottomWall)) {
                            pos.y = bottomWall(row) - ballRadius();
                            selected = true;
                            break;
                        }
                        wall = 0;
                }
            }

            bool tooClose = false;
            for (auto const &itemRC : itemsRC) {
                if (std::abs(static_cast<int32_t>(itemRC.first) - static_cast<int32_t>(row)) +
                    std::abs(static_cast<int32_t>(itemRC.second) - static_cast<int32_t>(col)) < 3) {
                    tooClose = true;
                }
            }

            if (!tooClose) {
                m_avoidObjectLocations.push_back(pos);
                itemsRC.emplace_back(row, col);
            }
        }
    }
} // namespace avoidVortexMaze