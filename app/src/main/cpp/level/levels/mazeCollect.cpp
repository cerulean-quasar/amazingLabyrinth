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

#include "mazeCollect.hpp"
#include "maze.hpp"
#include "../level.hpp"

bool MazeCollect::checkFinishCondition(float timeDiff) {
    uint32_t nbrItemsCollected = 0;

    // m_prevCells always has nbrItemsToCollect+1 items in it. The one at the back is the last
    // location of the user's ball.  If this is not true, then we need to add the new location of
    // user's ball to the back of the queue of previous user ball positions.
    if (m_ballCell.row != m_prevCells[nbrItemsToCollect].first ||
        m_ballCell.col != m_prevCells[nbrItemsToCollect].second) {
        m_prevCells.pop_front();
        m_prevCells.emplace_back(m_ballCell.row, m_ballCell.col);
    }

    uint32_t j = nbrItemsToCollect - 1;
    for (uint32_t i = 0; i < nbrItemsToCollect; i++) {
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
                item.second += (getCellCenterPosition(m_prevCells[j].first,m_prevCells[j].second) -
                                item.second) / timeTotal * timeDiff;
            }
            j--;
        }
    }

    // we finished the level if all the items are collected and the ball is in proximity to the hole.
    return nbrItemsCollected == m_collectionObjectLocations.size() &&
           ballInProximity(getColumnCenterPosition(m_mazeBoard.colEnd()), getRowCenterPosition(m_mazeBoard.rowEnd()));
}

void MazeCollect::generateCollectBallModelMatrices() {
    // generate the items to collect
    std::vector<std::pair<uint32_t, uint32_t>> itemsRC;
    itemsRC.emplace_back(m_mazeBoard.rowEnd(), m_mazeBoard.colEnd());
    itemsRC.emplace_back(m_ballCell.row, m_ballCell.col);
    while (m_collectionObjectLocations.size() < nbrItemsToCollect) {
        uint32_t row = random.getUInt(0, m_mazeBoard.numberRows()-1);
        uint32_t col = random.getUInt(0, m_mazeBoard.numberColumns()-1);

        // move it away from the wall by at least the ball diameter so that it can be easily
        // collected.
        glm::vec3 pos{random.getFloat(leftWall(col)+ballRadius(), rightWall(col)-ballRadius()),
                      random.getFloat(topWall(row)+ballRadius(), bottomWall(row)-ballRadius()),
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
    for (uint32_t i = 0; i < nbrItemsToCollect+1; i++) {
        m_prevCells.emplace_back(m_ballCell.row, m_ballCell.col);
    }
}

bool MazeCollect::updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged) {
    bool isEmpty = objs.empty();
    bool isUpdated = MazeOpenArea::updateDynamicDrawObjects(objs, textures, texturesChanged);

    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f),
            glm::vec3{collectBallScaleFactor, collectBallScaleFactor, collectBallScaleFactor});
    if (isEmpty) {
        // the objects to collect - they are just smaller versions of the ball.  Just add them to
        // the ball model matrices.  The ball is always first in the table of draw objects.
        auto const &ballObj = objs[0].first;
        for (auto const &item : m_collectionObjectLocations) {
            ballObj->modelMatrices.push_back(glm::translate(glm::mat4(1.0f),item.second) *
                glm::mat4_cast(m_ball.totalRotated) * scaleMatrix);
        }
        isUpdated = true;
    } else {
        DrawObjectEntry &obj = objs[0];

        // ignore the first model matrix.  It is for the main ball that the user is rolling
        // not the following balls.
        uint32_t i = 1;
        for (auto const &item : m_collectionObjectLocations) {
            if (item.first) {
                obj.first->modelMatrices[i++] = glm::translate(glm::mat4(1.0f), item.second) *
                        glm::mat4_cast(m_ball.totalRotated) *scaleMatrix;
                isUpdated = true;
            } else {
                i++;
            }
        }
    }

    return isUpdated;
}
