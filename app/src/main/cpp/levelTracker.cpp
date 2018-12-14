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
#include "movingQuadsLevel.hpp"

std::shared_ptr<LevelStarter> getLevelStarterBeginning() {
    std::shared_ptr<LevelStarter> levelStarter(new LevelStarter());
    levelStarter->addTextString("In the\nbeginning of\nthe universe\nof mazes...");
    levelStarter->addTextString("...there was nothing\nexcept for\na ball and\na strange\nspacial anomaly.");
    return levelStarter;
}

std::shared_ptr<LevelStarter> getLevelStarterIcePlanet() {
    std::shared_ptr<LevelStarter> levelStarter(new LevelStarter());
    levelStarter->addTextString("Now the maze\nuniverse is\nfilled with stars\nand black holes...");
    levelStarter->addTextString("...and a lonely\nice planet seeks\nout the warmth\nof a star.");
    return levelStarter;
}

std::shared_ptr<LevelStarter> getLevelStarterPufferFish() {
    std::shared_ptr<LevelStarter> levelStarter(new LevelStarter());
    levelStarter->addTextString("On the planet,\nthe ice begins\nto melt\nand life ignites...");
    levelStarter->addTextString("...here, a puffer\nfish is looking\nfor a kelp meal.");
    return levelStarter;
}

std::shared_ptr<LevelStarter> getLevelStarterRolarBear() {
    std::shared_ptr<LevelStarter> levelStarter(new LevelStarter());
    levelStarter->addTextString("As the ice\nplanet warms\nand the ice\nmelts...");
    levelStarter->addTextString("...an over hot\nrolar bear\nyearns for\nthe cold of the\nnorth pole.");
    return levelStarter;
}

std::shared_ptr<LevelStarter> getLevelStarterBee1() {
    std::shared_ptr<LevelStarter> levelStarter(new LevelStarter());
    levelStarter->addTextString("Then came an age\nwhen flowers\ncovered\nthe planet...");
    levelStarter->addTextString("...and one roller\nbee seeks out\nthe nectar\nof a large flower.");
    return levelStarter;
}

std::shared_ptr<LevelStarter> getLevelStarterBee2() {
    std::shared_ptr<LevelStarter> levelStarter(new LevelStarter());
    levelStarter->addTextString("The plants\nbecame more\nnumerous\nand the\nroller bee...");
    levelStarter->addTextString("...has to search\nlong and hard\nfor the\nlarge flower.");
    return levelStarter;
}

std::shared_ptr<Level> getLevelBeginning(uint32_t width, uint32_t height) {
    std::shared_ptr<OpenAreaLevel> openAreaLevel(new OpenAreaLevel());
    openAreaLevel->init(width, height);
    openAreaLevel->initSetBallTexture("textures/beginning/ballWhite.png");
    openAreaLevel->initSetHoleTexture("textures/beginning/holeAnomaly.png");
    return openAreaLevel;
}

std::shared_ptr<Level> getLevelIcePlanet(uint32_t width, uint32_t height) {
    std::shared_ptr<AvoidVortexLevel> level(new AvoidVortexLevel());
    level->init(width, height);
    level->initSetBallTexture("textures/icePlanet/ballIcePlanet.png");
    level->initSetHoleTexture("textures/icePlanet/holeSun.png");
    level->initSetVortexTexture("textures/icePlanet/vortexBlackHole.png");
    level->initSetStartVortexTexture("textures/beginning/holeAnomaly.png");
    return level;
}

std::shared_ptr<Level> getLevelPufferFish(uint32_t width, uint32_t height) {
    std::shared_ptr<Maze> maze(new Maze(7, 7, Maze::Mode::BFS));
    maze->init(width, height);
    maze->initSetBallTexture("textures/pufferFish/ballFish.png");
    maze->initAddWallTexture("textures/pufferFish/wallIce1.png");
    maze->initAddWallTexture("textures/pufferFish/wallIce2.png");
    maze->initAddWallTexture("textures/pufferFish/wallIce3.png");
    maze->initAddWallTexture("textures/pufferFish/wallIce4.png");
    maze->initSetFloorTexture("textures/pufferFish/floor.png");
    maze->initSetHoleTexture("textures/pufferFish/hole.png");
    return maze;
}

std::shared_ptr<Level> getLevelRolarBear(uint32_t width, uint32_t height) {
    std::shared_ptr<MovingQuadsLevel> level(new MovingQuadsLevel());
    level->init(width, height);
    level->initAddMiddleQuadTexture("textures/rolarBear/movingQuadRolarBear1.png");
    level->initAddMiddleQuadTexture("textures/rolarBear/movingQuadRolarBear2.png");
    level->initAddMiddleQuadTexture("textures/rolarBear/movingQuadRolarBear3.png");
    level->initAddMiddleQuadTexture("textures/rolarBear/movingQuadRolarBear4.png");
    level->initSetBallTexture("textures/rolarBear/ballRolarBear.png");
    level->initSetStartQuadTexture("textures/rolarBear/rolarBearStartQuad.png");
    level->initSetEndQuadTexture("textures/rolarBear/rolarBearEndQuad.png");
    return level;
}

std::shared_ptr<Level> getLevelBee1(uint32_t width, uint32_t height) {
    std::shared_ptr<Maze> maze(new Maze(5, 5, Maze::Mode::DFS));
    maze->init(width, height);
    maze->initSetBallTexture("textures/rollerBee/ballBee.png");
    maze->initAddWallTexture("textures/rollerBee/wallFlower1.png");
    maze->initAddWallTexture("textures/rollerBee/wallFlower2.png");
    maze->initAddWallTexture("textures/rollerBee/wallFlower3.png");
    maze->initAddWallTexture("textures/rollerBee/wallFlower4.png");
    maze->initSetFloorTexture("textures/rollerBee/floor.png");
    maze->initSetHoleTexture("textures/rollerBee/hole.png");
    return maze;
}

std::shared_ptr<Level> getLevelBee2(uint32_t width, uint32_t height) {
    std::shared_ptr<Maze> maze(new Maze(10, 10, Maze::Mode::DFS));
    maze->init(width, height);
    maze->initAddWallTexture("textures/rollerBee/wallFlower1.png");
    maze->initAddWallTexture("textures/rollerBee/wallFlower2.png");
    maze->initAddWallTexture("textures/rollerBee/wallFlower3.png");
    maze->initAddWallTexture("textures/rollerBee/wallFlower4.png");
    maze->initSetBallTexture("textures/rollerBee/ballBee.png");
    maze->initSetFloorTexture("textures/rollerBee/floor.png");
    maze->initSetHoleTexture("textures/rollerBee/hole.png");
    return maze;
}

std::shared_ptr<LevelFinish> getLevelFinisherBeginning(float x, float y) {
    std::shared_ptr<GrowingQuadLevelFinish> levelFinish(new GrowingQuadLevelFinish(x,y));
    levelFinish->initTexture("textures/beginning/starField.png");
    return levelFinish;
}

std::shared_ptr<LevelFinish> getLevelFinisherIcePlanet(float x, float y) {
    std::shared_ptr<GrowingQuadLevelFinish> levelFinish(new GrowingQuadLevelFinish(x,y));
    levelFinish->initTexture("textures/icePlanet/icePlanet.png");
    return levelFinish;
}

std::shared_ptr<LevelFinish> getLevelFinisherPufferFish(float x, float y) {
    std::shared_ptr<ManyQuadCoverUpLevelFinish> levelFinish(new ManyQuadCoverUpLevelFinish());
    levelFinish->initAddTexture("textures/pufferFish/hole.png");
    return levelFinish;
}

std::shared_ptr<LevelFinish> getLevelFinisherRolarBear(float x, float y) {
    std::shared_ptr<ManyQuadCoverUpLevelFinish> levelFinish(new ManyQuadCoverUpLevelFinish());
    levelFinish->initAddTexture("textures/rolarBear/rolarBearSmiley1.png");
    return levelFinish;
}

std::shared_ptr<LevelFinish> getLevelFinisherBee1(float x, float y) {
    std::shared_ptr<ManyQuadCoverUpLevelFinish> levelFinish(new ManyQuadCoverUpLevelFinish());
    for (auto &&image : std::vector<std::string>
            {"textures/rollerBee/flower1.png",
             "textures/rollerBee/flower2.png",
             "textures/rollerBee/flower3.png",
             "textures/rollerBee/flower4.png"}) {
        levelFinish->initAddTexture(image);
    }
    return levelFinish;
}

std::shared_ptr<LevelFinish> getLevelFinisherBee2(float x, float y) {
    std::shared_ptr<ManyQuadCoverUpLevelFinish> levelFinish(new ManyQuadCoverUpLevelFinish());
    for (auto &&image : std::vector<std::string>
            {"textures/rollerBee/flower1.png",
             "textures/rollerBee/flower2.png",
             "textures/rollerBee/flower3.png",
             "textures/rollerBee/flower4.png"}) {
        levelFinish->initAddTexture(image);
    }
    return levelFinish;
}

LevelTable LevelTracker::s_levelTable = {
        LevelEntry {getLevelStarterBeginning, getLevelBeginning, getLevelFinisherBeginning, "The beginning"},
        LevelEntry {getLevelStarterIcePlanet, getLevelIcePlanet, getLevelFinisherIcePlanet, "The lonely planet"},
        LevelEntry {getLevelStarterPufferFish, getLevelPufferFish, getLevelFinisherPufferFish, "The puffer fish"},
        LevelEntry {getLevelStarterRolarBear, getLevelRolarBear, getLevelFinisherRolarBear, "The rolar bear"},
        LevelEntry {getLevelStarterBee1, getLevelBee1, getLevelFinisherBee1, "The roller bee"},
        LevelEntry {getLevelStarterBee2, getLevelBee2, getLevelFinisherBee2, "The search"} };

LevelTracker::LevelTracker(uint32_t level, uint32_t inWidth, uint32_t inHeight)
        :m_currentLevel(level), m_width(inWidth), m_height(inHeight) {
    if (level > s_levelTable.size() - 1) {
        throw std::runtime_error("Invalid level: " + std::to_string(level));
    }
}

void LevelTracker::gotoNextLevel() {
    m_currentLevel = (m_currentLevel + 1) % s_levelTable.size();
}

std::shared_ptr<LevelStarter> LevelTracker::getLevelStarter() {
    return s_levelTable[m_currentLevel].starter();
}

std::shared_ptr<Level> LevelTracker::getLevel() {
    return s_levelTable[m_currentLevel].level(m_width, m_height);
}

std::shared_ptr<LevelFinish> LevelTracker::getLevelFinisher(float centerX = 0.0f, float centerY = 0.0f) {
    return s_levelTable[m_currentLevel].finisher(centerX, centerY);
}

std::vector<std::string> LevelTracker::getLevelDescriptions() {
    std::vector<std::string> ret;

    for (int i = 0; i < s_levelTable.size(); i++) {
        ret.push_back(std::to_string(i) + ": " + s_levelTable[i].levelDescription);
    }

    return ret;
}