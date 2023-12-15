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

#include "../basic/level.hpp"
#include "../generatedMaze/level.hpp"
#include "level.hpp"

namespace collectMaze {
    bool Level::checkFinishCondition(float timeDiff) {
        uint32_t nbrItemsCollected = 0;

        // m_prevCells always has nbrItemsToCollect+1 items in it. The one at the back is the last
        // location of the user's ball.  If this is not true, then we need to add the new location of
        // user's ball to the back of the queue of previous user ball positions.
        if (m_ballCell.row != m_prevCells[m_numberCollectObjects].first ||
            m_ballCell.col != m_prevCells[m_numberCollectObjects].second) {
            m_prevCells.pop_front();
            m_prevCells.emplace_back(m_ballCell.row, m_ballCell.col);
        }

        uint32_t j = m_numberCollectObjects - 1;
        for (uint32_t i = 0; i < m_numberCollectObjects; i++) {
            auto &item = m_collectionObjectLocations[i];
            if (!item.first && ballInProximity(item.second.x, item.second.y)) {
                item.first = true;
            }

            // move the collected balls to follow the user's ball.
            if (item.first) {
                nbrItemsCollected++;
                float length = glm::length(getCellCenterPosition(m_prevCells[j].first,
                                                                 m_prevCells[j].second) -
                                           item.second);
                if (length > ballRadius()) {
                    float timeTotal = 1.0f;
                    if (glm::length(m_ball.velocity) > 0.01f) {
                        timeTotal = length / glm::length(m_ball.velocity);
                    }
                    item.second +=
                            (getCellCenterPosition(m_prevCells[j].first, m_prevCells[j].second) -
                             item.second) / timeTotal * timeDiff;
                }
                j--;
            }
        }

        // we finished the level if all the items are collected and the ball is in proximity to the hole.
        return nbrItemsCollected == m_collectionObjectLocations.size() &&
               ballInProximity(getColumnCenterPosition(m_mazeBoard.colEnd()),
                               getRowCenterPosition(m_mazeBoard.rowEnd()));
    }

    void Level::generateCollectBallModelMatrices() {
        // generate the items to collect
        std::vector<std::pair<uint32_t, uint32_t>> itemsRC;
        itemsRC.emplace_back(m_mazeBoard.rowEnd(), m_mazeBoard.colEnd());
        itemsRC.emplace_back(m_ballCell.row, m_ballCell.col);
        while (m_collectionObjectLocations.size() < m_numberCollectObjects) {
            uint32_t row = random.getUInt(0, m_mazeBoard.numberRows() - 1);
            uint32_t col = random.getUInt(0, m_mazeBoard.numberColumns() - 1);

            // move it away from the wall by at least the ball diameter so that it can be easily
            // collected.
            glm::vec3 pos{
                    random.getFloat(leftWall(col) + ballRadius(), rightWall(col) - ballRadius()),
                    random.getFloat(topWall(row) + ballRadius(), bottomWall(row) - ballRadius()),
                    getBallZPosition()};
            bool tooClose = false;
            for (auto const &itemRC : itemsRC) {
                if (std::abs(static_cast<int32_t>(itemRC.first) - static_cast<int32_t>(row)) +
                    std::abs(static_cast<int32_t>(itemRC.second) - static_cast<int32_t>(col)) < 3) {
                    tooClose = true;
                }
            }

            if (!tooClose) {
                m_collectionObjectLocations.emplace_back(false, pos);
                itemsRC.emplace_back(row, col);
            }
        }

        // generate the previous locations of the ball (just set all previous locations to the ball starting cell).
        // this is used to cause the items to collect to follow the user's ball.
        for (uint32_t i = 0; i < m_numberCollectObjects + 1; i++) {
            m_prevCells.emplace_back(m_ballCell.row, m_ballCell.col);
        }
    }

    bool Level::updateDrawObjects() {
        generatedMaze::Level::updateDrawObjects();

        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f),
                                           glm::vec3{collectBallScaleFactor, collectBallScaleFactor,
                                                     collectBallScaleFactor});

        if (m_collectionObjectLocations.size() != m_objDataRefCollect.size()) {
            throw std::runtime_error("Invalid number of collection objects");
        }

        for (size_t i = 0; i < m_collectionObjectLocations.size(); i++) {
            m_levelDrawer.updateModelMatrixForObject(m_objRefCollect, m_objDataRefCollect[i],
                    glm::translate(glm::mat4(1.0f), m_collectionObjectLocations[i].second) *
                    glm::mat4_cast(m_ball.totalRotated) *
                    scaleMatrix);
        }

        return true;
    }
} // namespace collectMaze