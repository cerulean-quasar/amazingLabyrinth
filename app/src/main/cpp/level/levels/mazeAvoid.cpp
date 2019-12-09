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

#include "mazeAvoid.hpp"
#include "maze.hpp"

bool MazeAvoid::checkFinishCondition(float timeDiff) {
    for (auto avoidObj : m_avoidObjectLocations) {
        if (ballInProximity(avoidObj.x, avoidObj.y)) {
            // send the ball back to the start because it hit one of the objects it was supposed
            // to avoid.
            ball.row = m_ballStartR;
            ball.col = m_ballStartC;
            ball.position = getCellCenterPosition(m_ballStartR, m_ballStartC);
        }
    }

    // we finished the level if all the items are collected and the ball is in proximity to the hole.
    return ballInProximity(getColumnCenterPosition(m_colEnd), getRowCenterPosition(m_rowEnd));
}

void MazeAvoid::generateAvoidModelMatrices() {
    // generate the items to collect
    while (m_avoidObjectLocations.size() < nbrItemsToAvoid) {
        uint32_t row = random.getUInt(0, numberRows-1);
        uint32_t col = random.getUInt(0, numberColumns-1);

        glm::vec3 pos = getCellCenterPosition(row, col);
        int wall = random.getUInt(0, 3);
        bool selected = false;
        while (!selected) {
            switch (wall) {
                case 0:
                    if (cells[row][col].leftWallExists()) {
                        pos.x = leftWall(col) + scale;
                        selected = true;
                        break;
                    }
                    // fall through and just try to see if the next wall exists and put the object
                    // there if it does.
                case 1:
                    if (cells[row][col].rightWallExists()) {
                        pos.x = rightWall(col) - scale;
                        selected = true;
                        break;
                    }
                case 2:
                    if (cells[row][col].topWallExists()) {
                        pos.y = topWall(row) + scale ;
                        selected = true;
                        break;
                    }
                case 3:
                default:
                    if (cells[row][col].bottomWallExists()) {
                        pos.y = bottomWall(row) - scale;
                        selected = true;
                        break;
                    }
                    wall = 0;
            }
        }
        /*
        glm::vec3 pos{random.getFloat(leftWall(col)+scale/2.0f, rightWall(col)-scale/2.0f),
                      random.getFloat(topWall(row)+scale/2.0f, bottomWall(row)-scale/2.0f),
                      getBallZPosition()}; */
        bool tooClose = false;
        for (auto const &avoidObj : m_avoidObjectLocations) {
            if (glm::length(pos - avoidObj) < (m_width+m_height)/32) {
                tooClose = true;
            }
        }

        if (!tooClose) {
            m_avoidObjectLocations.push_back(pos);
        }
    }
}

bool MazeAvoid::updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) {
    if (objs.empty()) {
        // the maze static objects.
        bool isUpdated = Maze::updateStaticDrawObjects(objs, textures);

        // the objects to avoid
        std::shared_ptr<DrawObject> aviodObj(new DrawObject());
        aviodObj->indices = holeIndices;
        aviodObj->vertices = holeVertices;
        aviodObj->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester, m_avoidObjTexture);
        textures.insert(std::make_pair(aviodObj->texture, std::shared_ptr<TextureData>()));

        for (auto const &item : m_avoidObjectLocations) {
            aviodObj->modelMatrices.push_back(glm::translate(item) * scaleBall);
        }
        objs.push_back(std::make_pair(aviodObj, std::shared_ptr<DrawObjectData>()));

        return true;
    } else {
        return false;
    }
}
