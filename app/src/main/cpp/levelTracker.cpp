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

std::shared_ptr<LevelStarter> getLevelStarter0() {
    std::shared_ptr<LevelStarter> levelStarter(new  LevelStarter());
    levelStarter->addTextString("In the\nbeginning of\nthe universe\nof mazes...");
    levelStarter->addTextString("...there was nothing\nexcept for\na ball and\na strange\nspacial anomaly.");
    return levelStarter;
}

std::shared_ptr<LevelStarter> getLevelStarter1() {
    std::shared_ptr<LevelStarter> levelStarter(new LevelStarter());
    levelStarter->addTextString("Then came an age\nwhen flowers\ncovered\nthe planet...");
    levelStarter->addTextString("...and one roller\nbee seeks out\nthe nectar\nof a large flower.");
    return levelStarter;
}

std::shared_ptr<LevelStarter> getLevelStarter2() {
    std::shared_ptr<LevelStarter> levelStarter(new LevelStarter());
    levelStarter->addTextString("The plants\nbecame more\nnumerous\nand the\nroller bee...");
    levelStarter->addTextString("...has to search\nlong and hard\nfor the\nlarge flower.");
    return levelStarter;
}

std::shared_ptr<Level> getLevel0(uint32_t width, uint32_t height) {
    std::shared_ptr<OpenAreaLevel> openAreaLevel(new OpenAreaLevel());
    openAreaLevel->init(width, height);
    openAreaLevel->initSetBallTexture("textures/ballWhite.png");
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
    maze->initSetBallTexture("textures/ballBee.png");
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
    maze->initSetBallTexture("textures/ballBee.png");
    maze->initSetFloorTexture("textures/floor.png");
    maze->initSetHoleTexture("textures/hole.png");
    return maze;
}

std::shared_ptr<LevelFinish> getLevelFinisher0(float x, float y) {
    std::shared_ptr<GrowingQuadLevelFinish> levelFinish(new GrowingQuadLevelFinish(x,y));
    return levelFinish;
}

std::shared_ptr<LevelFinish> getLevelFinisher1(float x, float y) {
    std::shared_ptr<ManyQuadCoverUpLevelFinish> levelFinish(new ManyQuadCoverUpLevelFinish());
    return levelFinish;
}

std::shared_ptr<LevelFinish> getLevelFinisher2(float x, float y) {
    std::shared_ptr<ManyQuadCoverUpLevelFinish> levelFinish(new ManyQuadCoverUpLevelFinish());
    return levelFinish;
}

LevelTracker::LevelTracker() :currentLevel(0), width(0), height(0) {
    levelTable[0] = {getLevelStarter0, getLevel0, getLevelFinisher0};
    levelTable[1] = {getLevelStarter1, getLevel1, getLevelFinisher1};
    levelTable[2] = {getLevelStarter2, getLevel2, getLevelFinisher2};
}

void LevelTracker::gotoNextLevel() {
    currentLevel = (currentLevel + 1) % levelTable.size();
}

std::shared_ptr<LevelStarter> LevelTracker::getLevelStarter() {
    return levelTable[currentLevel].starter();
}

std::shared_ptr<Level> LevelTracker::getLevel() {
    return levelTable[currentLevel].level(width, height);
}

std::shared_ptr<LevelFinish> LevelTracker::getLevelFinisher(float centerX = 0.0f, float centerY = 0.0f) {
    return levelTable[currentLevel].finisher(centerX, centerY);
}
