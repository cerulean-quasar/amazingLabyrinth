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
#include <array>
#include "levelTracker.hpp"
#include "maze.hpp"
#include "openAreaLevel.hpp"

std::shared_ptr<Level> getLevel0(uint32_t width, uint32_t height) {
    std::shared_ptr<OpenAreaLevel> openAreaLevel(new OpenAreaLevel());
    openAreaLevel->init(width, height);
    openAreaLevel->initSetBallTexture("textures/ball.png");
    openAreaLevel->initSetHoleTexture("textures/holeAnomaly.png");
    return openAreaLevel;
}

std::shared_ptr<Level> getLevel1(uint32_t width, uint32_t height) {
    std::shared_ptr<Maze> maze(new Maze(5, 5));
    maze->init(width, height);
    maze->initAddWallTexture("textures/wallFlower1.png");
    maze->initAddWallTexture("textures/wallFlower2.png");
    maze->initAddWallTexture("textures/wallFlower3.png");
    maze->initAddWallTexture("textures/wallFlower4.png");
    maze->initSetBallTexture("textures/ball.png");
    maze->initSetFloorTexture("textures/floor.png");
    maze->initSetHoleTexture("textures/hole.png");
    return maze;
}

std::shared_ptr<Level> getLevel2(uint32_t width, uint32_t height) {
    std::shared_ptr<Maze> maze(new Maze(10, 10));
    maze->init(width, height);
    maze->initAddWallTexture("textures/wallFlower1.png");
    maze->initAddWallTexture("textures/wallFlower2.png");
    maze->initAddWallTexture("textures/wallFlower3.png");
    maze->initAddWallTexture("textures/wallFlower4.png");
    maze->initSetBallTexture("textures/ball.png");
    maze->initSetFloorTexture("textures/floor.png");
    maze->initSetHoleTexture("textures/hole.png");
    return maze;
}

std::shared_ptr<LevelFinish> getLevelFinisher0() {
    std::shared_ptr<GrowingQuadLevelFinish> levelFinish(new GrowingQuadLevelFinish());
    return levelFinish;
}

std::shared_ptr<LevelFinish> getLevelFinisher1() {
    std::shared_ptr<ManyQuadCoverUpLevelFinish> levelFinish(new ManyQuadCoverUpLevelFinish());
    return levelFinish;
}

std::shared_ptr<LevelFinish> getLevelFinisher2() {
    std::shared_ptr<ManyQuadCoverUpLevelFinish> levelFinish(new ManyQuadCoverUpLevelFinish());
    return levelFinish;
}

LevelTracker::LevelTracker() :currentLevel(0), width(0), height(0) {
    levelTable[0] = {getLevel0, getLevelFinisher0};
    levelTable[1] = {getLevel1, getLevelFinisher1};
    levelTable[2] = {getLevel2, getLevelFinisher2};
}

void LevelTracker::gotoNextLevel() {
    currentLevel = (currentLevel + 1) % levelTable.size();
}

std::shared_ptr<Level> LevelTracker::getLevel() {
    return levelTable[currentLevel].first(width, height);
}

std::shared_ptr<LevelFinish> LevelTracker::getLevelFinisher() {
    return levelTable[currentLevel].second();
};