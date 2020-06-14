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
#ifndef AMAZING_LABYRINTH_SAVE_DATA_HPP
#define AMAZING_LABYRINTH_SAVE_DATA_HPP

#include <vector>
#include <string>
#include <memory>
#include <glm/glm.hpp>

template <typename CoordType>
struct Point {
    CoordType x;
    CoordType y;
    Point<CoordType>() : x{0}, y{0} {}
    Point<CoordType>(CoordType x_, CoordType y_) : x{x_}, y{y_} {}
    Point<CoordType>(Point<CoordType> const &other) : x{other.x}, y{other.y} {}
    bool operator==(Point<CoordType> const &other) const {
        return x == other.x && y == other.y;
    }
    bool operator!=(Point<CoordType> const &other) const {
        return x != other.x || y != other.y;
    }
};


class LevelTracker;
class LevelStarter;
class Level;
class LevelFinish;
using GetStarterFcn = std::function<std::shared_ptr<LevelStarter>(LevelTracker &levelTracker,
        glm::mat4 const &proj, glm::mat4 const &view)>;
using GetLevelFcn = std::function<std::shared_ptr<Level>(LevelTracker &levelTracker,
        glm::mat4 const &proj, glm::mat4 const &view)>;
using GetFinisherFcn = std::function<std::shared_ptr<LevelFinish>(
        LevelTracker &levelTracker, float centerX, float centerY, glm::mat4 const &proj, glm::mat4 const &view)>;

struct RestoreData {
    std::string levelName;
    LevelGroup levelGroupFcns;
};

#endif
