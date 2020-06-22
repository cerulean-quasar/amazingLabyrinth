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
#ifndef AMAZING_LABYRINTH_LEVEL_TRACKER_TYPES_HPP
#define AMAZING_LABYRINTH_LEVEL_TRACKER_TYPES_HPP

template <typename CoordType>
struct Point {
    union {
        CoordType x;
        CoordType row;
    };
    union {
        CoordType y;
        CoordType col;
    };

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

namespace levelTracker {
    int constexpr GameSaveDataVersionValue = 1;
    struct GameSaveData {
        int version;
        Point<uint32_t> screenSize;
        std::string levelName;
        bool needsStarter;
        GameSaveData(
                Point<uint32_t> const &screenSize_,
                std::string const &levelName_,
                bool needsStarter_) :
                version(GameSaveDataVersionValue),
                screenSize(screenSize_),
                levelName(levelName_),
                needsStarter(needsStarter_)
        {}

        GameSaveData(
                int version_,
                Point<uint32_t> const &screenSize_,
                std::string const &levelName_,
                bool needsStarter_) :
                version(version_),
                screenSize(screenSize_),
                levelName(levelName_),
                needsStarter(needsStarter_)
        {}

        GameSaveData()
            : version{GameSaveDataVersionValue},
              screenSize{0,0},
              levelName{},
              needsStarter{true}
        {}
    };

}

#endif // AMAZING_LABYRINTH_LEVEL_TRACKER_TYPES_HPP