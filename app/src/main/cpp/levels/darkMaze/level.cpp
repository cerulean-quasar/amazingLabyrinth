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
#include <vector>
#include "../generatedMaze/level.hpp"
#include "level.hpp"

namespace darkMaze {
    bool Level::updateDrawObjects() {
        m_parameters.lightingSources[0] = m_ball.position;
        m_levelDrawer.updateCommonObjectData(m_objRefsWalls[0], m_parameters);
        return openAreaMaze::Level::updateDrawObjects();
    }

    bool Level::checkFinishCondition(float) {
        // we finished the level if all the items are collected and the ball is in proximity to the hole.
        return ballInProximity(getColumnCenterPosition(m_mazeBoard.colEnd()),
                               getRowCenterPosition(m_mazeBoard.rowEnd()));
    }

} // namespace darkMaze
