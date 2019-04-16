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
#include <array>
#include "levelTracker.hpp"
#include "maze.hpp"
#include "openAreaLevel.hpp"
#include "avoidVortexLevel.hpp"
#include "movingQuadsLevel.hpp"
#include "mazeCollect.hpp"

float constexpr LevelTracker::m_maxZLevel;
float constexpr LevelTracker::m_maxZLevelFinisher;

std::shared_ptr<LevelStarter> getLevelStarterBeginning(float width, float height, float maxZ) {
    std::shared_ptr<LevelStarter> levelStarter(new LevelStarter(width, height, maxZ));
    levelStarter->addTextString("In the\nbeginning of\nthe universe\nof mazes...");
    levelStarter->addTextString("...there was nothing\nexcept for\na ball and\na strange\nspacial anomaly.");
    return levelStarter;
}

std::shared_ptr<LevelStarter> getLevelStarterIcePlanet(float width, float height, float maxZ) {
    std::shared_ptr<LevelStarter> levelStarter(new LevelStarter(width, height, maxZ));
    levelStarter->addTextString("Now the maze\nuniverse is\nfilled with stars\nand black holes...");
    levelStarter->addTextString("...and a lonely\nice planet seeks\nout the warmth\nof a star.");
    return levelStarter;
}

std::shared_ptr<LevelStarter> getLevelStarterPufferFish(float width, float height, float maxZ) {
    std::shared_ptr<LevelStarter> levelStarter(new LevelStarter(width, height, maxZ));
    levelStarter->addTextString("On the planet,\nthe ice begins\nto melt\nand life ignites...");
    levelStarter->addTextString("...here, a puffer\nfish is looking\nfor a kelp meal.");
    return levelStarter;
}

std::shared_ptr<LevelStarter> getLevelStarterRolarBear(float width, float height, float maxZ) {
    std::shared_ptr<LevelStarter> levelStarter(new LevelStarter(width, height, maxZ));
    levelStarter->addTextString("As the ice\nplanet warms\nand the ice\nmelts...");
    levelStarter->addTextString("...an over hot\nrolar bear\nyearns for\nthe cold of the\nnorth pole.");
    return levelStarter;
}

std::shared_ptr<LevelStarter> getLevelStarterBee1(float width, float height, float maxZ) {
    std::shared_ptr<LevelStarter> levelStarter(new LevelStarter(width, height, maxZ));
    levelStarter->addTextString("Next comes an age\nwhen flowers\ncovered\nthe planet...");
    levelStarter->addTextString("...and one roller\nbee seeks out\nthe nectar\nof a large flower.");
    return levelStarter;
}

std::shared_ptr<LevelStarter> getLevelStarterBee2(float width, float height, float maxZ) {
    std::shared_ptr<LevelStarter> levelStarter(new LevelStarter(width, height, maxZ));
    levelStarter->addTextString("The plants have\nbecome more\nnumerous\nand the\nroller bee...");
    levelStarter->addTextString("...has to search\nlong and hard\nfor the\nlarge flower.");
    return levelStarter;
}

std::shared_ptr<LevelStarter> getLevelStarterCat(float width, float height, float maxZ) {
    std::shared_ptr<LevelStarter> levelStarter(new LevelStarter(width, height, maxZ));
    levelStarter->addTextString("In the jungle,\na mother cat...");
    levelStarter->addTextString("...searches\nfor her\nlost kittens.");
    return levelStarter;
}

std::shared_ptr<Level> getLevelBeginning(float width, float height, float maxZ) {
    std::shared_ptr<OpenAreaLevel> openAreaLevel(new OpenAreaLevel(width, height, maxZ));
    openAreaLevel->init();
    openAreaLevel->initSetBallTexture("textures/beginning/ballWhite.png");
    openAreaLevel->initSetHoleTexture("textures/beginning/holeAnomaly.png");
    return openAreaLevel;
}

std::shared_ptr<Level> getLevelIcePlanet(float width, float height, float maxZ) {
    std::shared_ptr<AvoidVortexLevel> level(new AvoidVortexLevel(width, height, maxZ));
    level->init();
    level->initSetBallTexture("textures/icePlanet/ballIcePlanet.png");
    level->initSetHoleTexture("textures/icePlanet/holeSun.png");
    level->initSetVortexTexture("textures/icePlanet/vortexBlackHole.png");
    level->initSetStartVortexTexture("textures/beginning/holeAnomaly.png");
    return level;
}

std::shared_ptr<Level> getLevelPufferFish(float width, float height, float maxZ) {
    std::shared_ptr<Maze> maze(new Maze(10, Maze::Mode::BFS, width, height, maxZ));
    maze->init();
    maze->initSetBallTexture("textures/pufferFish/ballFish.png");
    maze->initAddWallTexture("textures/pufferFish/wallIce1.png");
    maze->initAddWallTexture("textures/pufferFish/wallIce2.png");
    maze->initAddWallTexture("textures/pufferFish/wallIce3.png");
    maze->initAddWallTexture("textures/pufferFish/wallIce4.png");
    maze->initSetFloorTexture("textures/pufferFish/floor.png");
    maze->initSetHoleTexture("textures/pufferFish/hole.png");
    return maze;
}

std::shared_ptr<Level> getLevelRolarBear(float width, float height, float maxZ) {
    std::shared_ptr<MovingQuadsLevel> level(new MovingQuadsLevel(width, height, maxZ));
    level->init();
    level->initAddMiddleQuadTexture("textures/rolarBear/movingQuadRolarBear1.png");
    level->initAddMiddleQuadTexture("textures/rolarBear/movingQuadRolarBear2.png");
    level->initAddMiddleQuadTexture("textures/rolarBear/movingQuadRolarBear3.png");
    level->initAddMiddleQuadTexture("textures/rolarBear/movingQuadRolarBear4.png");
    level->initSetBallTexture("textures/rolarBear/ballRolarBear.png");
    level->initSetStartQuadTexture("textures/rolarBear/rolarBearStartQuad.png");
    level->initSetEndQuadTexture("textures/rolarBear/rolarBearEndQuad.png");
    return level;
}

std::shared_ptr<Level> getLevelBee1(float width, float height, float maxZ) {
    std::shared_ptr<Maze> maze(new Maze(10, Maze::Mode::DFS, width, height, maxZ));
    maze->init();
    maze->initSetBallTexture("textures/rollerBee/ballBee.png");
    maze->initAddWallTexture("textures/rollerBee/wallFlower1.png");
    maze->initAddWallTexture("textures/rollerBee/wallFlower2.png");
    maze->initAddWallTexture("textures/rollerBee/wallFlower3.png");
    maze->initAddWallTexture("textures/rollerBee/wallFlower4.png");
    maze->initSetFloorTexture("textures/rollerBee/floor.png");
    maze->initSetHoleTexture("textures/rollerBee/hole.png");
    return maze;
}

std::shared_ptr<Level> getLevelBee2(float width, float height, float maxZ) {
    std::shared_ptr<Maze> maze(new Maze(15, Maze::Mode::DFS, width, height, maxZ));
    maze->init();
    maze->initAddWallTexture("textures/rollerBee/wallFlower1.png");
    maze->initAddWallTexture("textures/rollerBee/wallFlower2.png");
    maze->initAddWallTexture("textures/rollerBee/wallFlower3.png");
    maze->initAddWallTexture("textures/rollerBee/wallFlower4.png");
    maze->initSetBallTexture("textures/rollerBee/ballBee.png");
    maze->initSetFloorTexture("textures/rollerBee/floor.png");
    maze->initSetHoleTexture("textures/rollerBee/hole.png");
    return maze;
}

std::shared_ptr<Level> getLevelCat(float width, float height, float maxZ) {
    std::shared_ptr<MazeCollect> maze(new MazeCollect(10, Maze::Mode::DFS, width, height, maxZ));
    maze->init();
    maze->initAddWallTexture("textures/rollerBee/wallFlower1.png");
    maze->initAddWallTexture("textures/rollerBee/wallFlower2.png");
    maze->initAddWallTexture("textures/rollerBee/wallFlower3.png");
    maze->initAddWallTexture("textures/rollerBee/wallFlower4.png");
    maze->initSetBallTexture("textures/cat/catBall.png");
    maze->initSetFloorTexture("textures/rollerBee/floor.png");
    maze->initSetHoleTexture("textures/cat/catHole.png");
    return maze;
}

std::shared_ptr<LevelFinish> getLevelFinisherBeginning(float x, float y, float maxZ) {
    std::shared_ptr<GrowingQuadLevelFinish> levelFinish(new GrowingQuadLevelFinish(x,y,maxZ));
    levelFinish->initTexture("textures/beginning/starField.png");
    return levelFinish;
}

std::shared_ptr<LevelFinish> getLevelFinisherIcePlanet(float x, float y, float maxZ) {
    std::shared_ptr<GrowingQuadLevelFinish> levelFinish(new GrowingQuadLevelFinish(x,y,maxZ));
    levelFinish->initTexture("textures/icePlanet/icePlanet.png");
    return levelFinish;
}

std::shared_ptr<LevelFinish> getLevelFinisherPufferFish(float x, float y, float maxZ) {
    std::shared_ptr<ManyQuadCoverUpLevelFinish> levelFinish(new ManyQuadCoverUpLevelFinish(maxZ));
    levelFinish->initAddTexture("textures/pufferFish/hole.png");
    return levelFinish;
}

std::shared_ptr<LevelFinish> getLevelFinisherRolarBear(float x, float y, float maxZ) {
    std::shared_ptr<ManyQuadCoverUpLevelFinish> levelFinish(new ManyQuadCoverUpLevelFinish(maxZ));
    levelFinish->initAddTexture("textures/rolarBear/rolarBearSmiley1.png");
    return levelFinish;
}

std::shared_ptr<LevelFinish> getLevelFinisherBee1(float x, float y, float maxZ) {
    std::shared_ptr<ManyQuadCoverUpLevelFinish> levelFinish(new ManyQuadCoverUpLevelFinish(maxZ));
    for (auto &&image : std::vector<std::string>
            {"textures/rollerBee/flower1.png",
             "textures/rollerBee/flower2.png",
             "textures/rollerBee/flower3.png",
             "textures/rollerBee/flower4.png"}) {
        levelFinish->initAddTexture(image);
    }
    return levelFinish;
}

std::shared_ptr<LevelFinish> getLevelFinisherBee2(float x, float y, float maxZ) {
    std::shared_ptr<ManyQuadCoverUpLevelFinish> levelFinish(new ManyQuadCoverUpLevelFinish(maxZ));
    for (auto &&image : std::vector<std::string>
            {"textures/rollerBee/flower1.png",
             "textures/rollerBee/flower2.png",
             "textures/rollerBee/flower3.png",
             "textures/rollerBee/flower4.png"}) {
        levelFinish->initAddTexture(image);
    }
    return levelFinish;
}

std::shared_ptr<LevelFinish> getLevelFinisherCat(float x, float y, float maxZ) {
    std::shared_ptr<ManyQuadCoverUpLevelFinish> levelFinish(new ManyQuadCoverUpLevelFinish(maxZ));
    levelFinish->initAddTexture("textures/pufferFish/hole.png");
    return levelFinish;
}

LevelTable LevelTracker::s_levelTable = {
        LevelEntry {getLevelStarterBeginning, getLevelBeginning, getLevelFinisherBeginning, "The beginning"},
        LevelEntry {getLevelStarterIcePlanet, getLevelIcePlanet, getLevelFinisherIcePlanet, "The lonely planet"},
        LevelEntry {getLevelStarterPufferFish, getLevelPufferFish, getLevelFinisherPufferFish, "The puffer fish"},
        LevelEntry {getLevelStarterRolarBear, getLevelRolarBear, getLevelFinisherRolarBear, "The rolar bear"},
        LevelEntry {getLevelStarterBee1, getLevelBee1, getLevelFinisherBee1, "The roller bee"},
        LevelEntry {getLevelStarterBee2, getLevelBee2, getLevelFinisherBee2, "The search"},
        LevelEntry {getLevelStarterCat, getLevelCat, getLevelFinisherCat, "The cat"} };

std::pair<float, float> LevelTracker::getWidthHeight(
        float maxZ, glm::mat4 const &proj, glm::mat4 const &view) {
    glm::vec4 worldZ{0.0f, 0.0f, maxZ, 1.0f};
    glm::vec4 z = proj * view * worldZ;

    glm::vec4 plus = glm::vec4{z.w, z.w, z.z, z.w};
    glm::vec4 worldPlus = glm::inverse(view) * glm::inverse(proj) * plus;

    return std::make_pair<float, float>(worldPlus.x/worldPlus.w * 2, worldPlus.y/worldPlus.w * 2);
}

void LevelTracker::gotoNextLevel() {
    m_currentLevel = (m_currentLevel + 1) % s_levelTable.size();
}

std::shared_ptr<LevelStarter> LevelTracker::getLevelStarter() {
    return s_levelTable[m_currentLevel].starter(m_widthLevelStarter, m_heightLevelStarter, m_maxZLevelStarter);
}

std::shared_ptr<Level> LevelTracker::getLevel() {
    return s_levelTable[m_currentLevel].level(m_widthLevel, m_heightLevel, m_maxZLevel);
}

std::shared_ptr<LevelFinish> LevelTracker::getLevelFinisher(float centerX, float centerY,
        glm::mat4 const &proj, glm::mat4 const &view) {
    glm::vec4 locationScreen = proj * view * glm::vec4{centerX, centerY, m_maxZLevel, 1.0f};
    glm::vec4 z = proj * view * glm::vec4{0.0f, 0.0f, m_maxZLevelFinisher, 1.0f};
    locationScreen = glm::vec4{locationScreen.x * z.w, locationScreen.y * z.w,
                               z.z * locationScreen.w, locationScreen.w * z.w};
    glm::vec4 locationWorld = glm::inverse(view) * glm::inverse(proj) * locationScreen;
    return s_levelTable[m_currentLevel].finisher(locationWorld.x/locationWorld.w,
            locationWorld.y/locationWorld.w, m_maxZLevelFinisher);
}

std::vector<std::string> LevelTracker::getLevelDescriptions() {
    std::vector<std::string> ret;

    for (int i = 0; i < s_levelTable.size(); i++) {
        ret.push_back(std::to_string(i) + ": " + s_levelTable[i].levelDescription);
    }

    return ret;
}