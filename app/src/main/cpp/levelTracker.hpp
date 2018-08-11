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
#ifndef AMAZING_LABYRINTH_LEVEL_TRACKER_HPP
#define AMAZING_LABYRINTH_LEVEL_TRACKER_HPP
#include <array>
#include "levelFinish.hpp"
#include "level.hpp"
#include "levelStarter.hpp"

typedef std::shared_ptr<Level> (*getLevel)(uint32_t width, uint32_t height);
typedef std::shared_ptr<LevelFinish> (*getLevelFinisher)(float centerX, float centerY);
typedef std::shared_ptr<LevelStarter> (*getLevelStarter)();

struct LevelEntry {
    getLevelStarter starter;
    getLevel level;
    getLevelFinisher finisher;
};

class LevelTracker {
private:
    uint32_t currentLevel;
    uint32_t width;
    uint32_t height;
    std::array<LevelEntry, 3> levelTable;
public:
    LevelTracker();

    void setParameters(uint32_t inWidth, uint32_t inHeight) {
        width = inWidth;
        height = inHeight;
    }

    std::shared_ptr<LevelStarter> getLevelStarter();
    std::shared_ptr<Level> getLevel();
    std::shared_ptr<LevelFinish> getLevelFinisher(float centerX, float centerY);
    void gotoNextLevel();
};
#endif