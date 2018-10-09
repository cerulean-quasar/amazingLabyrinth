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
#include "avoidVortexLevel.hpp"

std::shared_ptr<LevelStarter> getLevelStarter0() {
    std::shared_ptr<LevelStarter> levelStarter(new  LevelStarter());
    levelStarter->addTextString("In the\nbeginning of\nthe universe\nof mazes...");
    levelStarter->addTextString("...there was nothing\nexcept for\na ball and\na strange\nspacial anomaly.");
    return levelStarter;
}

std::shared_ptr<LevelStarter> getLevelStarter1() {
    std::shared_ptr<LevelStarter> levelStarter(new LevelStarter());
    levelStarter->addTextString("Now the maze\nuniverse is\nfilled with stars\nand black holes...");
    levelStarter->addTextString("...and a lonely\nice planet seeks\nout the warmth\nof a star.");
    return levelStarter;
}

std::shared_ptr<LevelStarter> getLevelStarter2() {
    std::shared_ptr<LevelStarter> levelStarter(new LevelStarter());
    levelStarter->addTextString("Then came an age\nwhen flowers\ncovered\nthe planet...");
    levelStarter->addTextString("...and one roller\nbee seeks out\nthe nectar\nof a large flower.");
    return levelStarter;
}

std::shared_ptr<LevelStarter> getLevelStarter3() {
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
    std::shared_ptr<AvoidVortexLevel> level(new AvoidVortexLevel());
    level->init(width, height);
    level->initSetBallTexture("textures/ballIcePlanet.png");
    level->initSetHoleTexture("textures/holeSun.png");
    level->initSetVortexTexture("textures/vortexBlackHole.png");
    level->initSetStartVortexTexture("textures/holeAnomaly.png");
    return level;
}

std::shared_ptr<Level> getLevel2(uint32_t width, uint32_t height) {
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

std::shared_ptr<Level> getLevel3(uint32_t width, uint32_t height) {
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
    levelFinish->initTexture("textures/starField.png");
    return levelFinish;
}

std::shared_ptr<LevelFinish> getLevelFinisher1(float x, float y) {
    std::shared_ptr<GrowingQuadLevelFinish> levelFinish(new GrowingQuadLevelFinish(x,y));
    levelFinish->initTexture("textures/icePlanet.png");
    return levelFinish;
}

std::shared_ptr<LevelFinish> getLevelFinisher2(float x, float y) {
    std::shared_ptr<ManyQuadCoverUpLevelFinish> levelFinish(new ManyQuadCoverUpLevelFinish());
    for (auto &&image : std::vector<std::string>
            {"textures/flower1.png",
             "textures/flower2.png",
             "textures/flower3.png",
             "textures/flower4.png"}) {
        levelFinish->initAddTexture(image);
    }
    return levelFinish;
}

std::shared_ptr<LevelFinish> getLevelFinisher3(float x, float y) {
    std::shared_ptr<ManyQuadCoverUpLevelFinish> levelFinish(new ManyQuadCoverUpLevelFinish());
    for (auto &&image : std::vector<std::string>
            {"textures/flower1.png",
             "textures/flower2.png",
             "textures/flower3.png",
             "textures/flower4.png"}) {
        levelFinish->initAddTexture(image);
    }
    return levelFinish;
}

LevelTable LevelTracker::levelTable = {
        LevelEntry {getLevelStarter0, getLevel0, getLevelFinisher0, "The universe begins"},
        LevelEntry {getLevelStarter1, getLevel1, getLevelFinisher1, "The lonely planet"},
        LevelEntry {getLevelStarter2, getLevel2, getLevelFinisher2, "The roller bee"},
        LevelEntry {getLevelStarter3, getLevel3, getLevelFinisher3, "The search"} };

LevelTracker::LevelTracker(uint32_t level) :currentLevel(level), width(0), height(0) {
    if (level > levelTable.size() - 1) {
        throw std::runtime_error("Invalid level: " + std::to_string(level));
    }
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

std::vector<std::string> LevelTracker::getLevelDescriptions() {
    std::vector<std::string> ret;

    for (int i = 0; i < levelTable.size(); i++) {
        ret.push_back(std::to_string(i) + ": " + levelTable[i].levelDescription);
    }

    return ret;
}