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
#include <array>
#include <functional>
#include <boost/optional.hpp>
#include <vector>

#include "levelTracker.hpp"
#include "levels/maze.hpp"
#include "levels/openAreaLevel.hpp"
#include "levels/avoidVortexLevel.hpp"
#include "levels/movingQuadsLevel.hpp"
#include "levels/mazeCollect.hpp"
#include "levels/mazeAvoid.hpp"
#include "levels/fixedMaze.hpp"

float constexpr LevelTracker::m_maxZLevelStarter;
float constexpr LevelTracker::m_maxZLevel;
float constexpr LevelTracker::m_maxZLevelFinisher;

LevelGroup LevelTracker::getLevelGroupBeginning(std::shared_ptr<OpenAreaLevelSaveData> const &levelBundle, bool needsStarter) {
    return {
        getStarterFcn(needsStarter, std::vector<std::string>{
            "In the\nbeginning of\nthe universe\nof mazes...",
            "...there was\nnothing except\nfor a ball and\na strange\nspacial anomaly."}),
        GetLevelFcn([levelBundle](LevelTracker &tracker, glm::mat4 const &proj, glm::mat4 const &view) {
            auto level = tracker.getLevel<OpenAreaLevel>(levelBundle, proj, view);
            level->initSetBallTexture("textures/beginning/ballWhite.png");
            level->initSetHoleTexture("textures/beginning/holeAnomaly.png");
            return level;
        }),
        GetFinisherFcn([](LevelTracker &tracker, float centerX, float centerY, glm::mat4 const &proj, glm::mat4 const &view) {
            auto levelFinish = tracker.getFinisher<GrowingQuadLevelFinish>(centerX, centerY, proj, view);
            levelFinish->initTexture("textures/beginning/starField.png");
            return levelFinish;
        })
    };
}

LevelGroup LevelTracker::getLevelGroupLonelyPlanet(std::shared_ptr<AvoidVortexLevelSaveData> const &levelBundle, bool needsStarter) {
    return {
        getStarterFcn(needsStarter, std::vector<std::string>{
            "Now the maze\nuniverse is\nfilled with stars\nand black holes...",
            "...and a lonely\nice planet seeks\nout the warmth\nof a star."}),
        GetLevelFcn([levelBundle](LevelTracker &tracker, glm::mat4 const &proj, glm::mat4 const &view) {
            auto level = tracker.getLevel<AvoidVortexLevel>(levelBundle, proj, view);
            level->initSetBallTexture("textures/icePlanet/ballIcePlanet.png");
            level->initSetHoleTexture("textures/icePlanet/holeSun.png");
            level->initSetVortexTexture("textures/icePlanet/vortexBlackHole.png");
            level->initSetStartVortexTexture("textures/beginning/holeAnomaly.png");
            return level;
        }),
        GetFinisherFcn([](LevelTracker &tracker, float centerX, float centerY, glm::mat4 const &proj, glm::mat4 const &view) {
            auto levelFinish = tracker.getFinisher<GrowingQuadLevelFinish>(centerX, centerY, proj, view);
            levelFinish->initTexture("textures/icePlanet/icePlanet.png");
            return levelFinish;
        })
    };
}

LevelGroup LevelTracker::getLevelGroupPufferFish(std::shared_ptr<MazeSaveData> const &levelBundle, bool needsStarter) {
    return {
        getStarterFcn(needsStarter, std::vector<std::string>{
            "On the planet,\nthe ice begins\nto melt\nand life ignites...",
            "...here, a puffer\nfish is looking\nfor a kelp meal."}),
        GetLevelFcn([levelBundle](LevelTracker &tracker, glm::mat4 const &proj, glm::mat4 const &view) {
            std::shared_ptr<Maze> level;
            if (levelBundle == nullptr) {
                Maze::CreateParameters parameters = { 10, GeneratedMazeBoard::Mode::BFS };
                level = tracker.getLevel<Maze>(parameters, proj, view);
            } else {
                level = tracker.getLevel<Maze>(levelBundle, proj, view);
            }
            level->initSetBallTexture("textures/pufferFish/ballFish.png");
            level->initAddWallTexture("textures/pufferFish/wallIce1.png");
            level->initAddWallTexture("textures/pufferFish/wallIce2.png");
            level->initAddWallTexture("textures/pufferFish/wallIce3.png");
            level->initAddWallTexture("textures/pufferFish/wallIce4.png");
            level->doneAddingWallTextures();
            level->initSetFloorTexture("textures/pufferFish/floor.png");
            level->initSetHoleTexture("textures/pufferFish/hole.png");
            return level;
        }),
        GetFinisherFcn([](LevelTracker &tracker, float centerX, float centerY, glm::mat4 const &proj, glm::mat4 const &view) {
            auto levelFinish = tracker.getFinisher<ManyQuadCoverUpLevelFinish>(centerX, centerY, proj, view);
            levelFinish->initAddTexture("textures/pufferFish/hole.png");
            return levelFinish;
        })
    };
}

LevelGroup LevelTracker::getLevelGroupRolarBear(std::shared_ptr<MovingQuadsLevelSaveData> const &levelBundle, bool needsStarter) {
    return {
        getStarterFcn(needsStarter, std::vector<std::string>{
            "As the ice\nplanet warms\nand the ice\nmelts...",
            "...an over hot\nrolar bear\nyearns for\nthe cold of the\nnorth pole."}),
        GetLevelFcn([levelBundle](LevelTracker &tracker, glm::mat4 const &proj, glm::mat4 const &view) {
            auto level = tracker.getLevel<MovingQuadsLevel>(levelBundle, proj, view);
            level->initAddMiddleQuadTexture("textures/rolarBear/movingQuadRolarBear1.png");
            level->initAddMiddleQuadTexture("textures/rolarBear/movingQuadRolarBear2.png");
            level->initAddMiddleQuadTexture("textures/rolarBear/movingQuadRolarBear3.png");
            level->initAddMiddleQuadTexture("textures/rolarBear/movingQuadRolarBear4.png");
            level->initSetBallTexture("textures/rolarBear/ballRolarBear.png");
            level->initSetStartQuadTexture("textures/rolarBear/rolarBearStartQuad.png");
            level->initSetEndQuadTexture("textures/rolarBear/rolarBearEndQuad.png");
            return level;
        }),
        GetFinisherFcn([](LevelTracker &tracker, float centerX, float centerY, glm::mat4 const &proj, glm::mat4 const &view) {
            auto levelFinish = tracker.getFinisher<ManyQuadCoverUpLevelFinish>(centerX, centerY, proj, view);
            levelFinish->initAddTexture("textures/rolarBear/rolarBearSmiley1.png");
            return levelFinish;
        })
    };
}

LevelGroup LevelTracker::getLevelGroupBee1(std::shared_ptr<MazeSaveData> const &levelBundle, bool needsStarter) {
    return {
        getStarterFcn(needsStarter, std::vector<std::string>{
            "Next comes an\nage when\nflowers covered\nthe planet...",
            "...and one roller\nbee seeks out\nthe nectar\nof a large flower."}),
        GetLevelFcn([levelBundle](LevelTracker &tracker, glm::mat4 const &proj, glm::mat4 const &view) {
            std::shared_ptr<Maze> level;
            if (levelBundle == nullptr) {
                Maze::CreateParameters parameters = { 10, GeneratedMazeBoard::Mode::DFS };
                level = tracker.getLevel<Maze>(parameters, proj, view);
            } else {
                level = tracker.getLevel<Maze>(levelBundle, proj, view);
            }
            level->initSetBallTexture("textures/rollerBee/ballBee.png");
            level->initSetFloorTexture("textures/rollerBee/floor.png");
            level->initSetHoleTexture("textures/rollerBee/hole.png");
            level->initAddWallTexture("textures/rollerBee/wallFlower1.png");
            level->initAddWallTexture("textures/rollerBee/wallFlower2.png");
            level->initAddWallTexture("textures/rollerBee/wallFlower3.png");
            level->initAddWallTexture("textures/rollerBee/wallFlower4.png");
            level->doneAddingWallTextures();
            return level;
        }),
        GetFinisherFcn([](LevelTracker &tracker, float centerX, float centerY, glm::mat4 const &proj, glm::mat4 const &view) {
            auto levelFinish = tracker.getFinisher<ManyQuadCoverUpLevelFinish>(centerX, centerY, proj, view);
            for (auto &&image : std::vector<std::string>
                    {"textures/rollerBee/flower1.png",
                     "textures/rollerBee/flower2.png",
                     "textures/rollerBee/flower3.png",
                     "textures/rollerBee/flower4.png"}) {
                levelFinish->initAddTexture(image);
            }
            return levelFinish;
        })
    };
}

LevelGroup LevelTracker::getLevelGroupBee2(std::shared_ptr<MazeSaveData> const &levelBundle, bool needsStarter) {
    return {
        getStarterFcn(needsStarter, std::vector<std::string>{
            "The plants have\nbecome more\nnumerous\nand the\nroller bee...",
            "...has to search\nlong and hard\nfor the\nlarge flower."}),
        GetLevelFcn([levelBundle](LevelTracker &tracker, glm::mat4 const &proj, glm::mat4 const &view) {
            std::shared_ptr<Maze> level;
            if (levelBundle == nullptr) {
                Maze::CreateParameters parameters = { 15, GeneratedMazeBoard::Mode::DFS };
                level = tracker.getLevel<Maze>(parameters, proj, view);
            } else {
                level = tracker.getLevel<Maze>(levelBundle, proj, view);
            }
            level->initSetBallTexture("textures/rollerBee/ballBee.png");
            level->initSetFloorTexture("textures/rollerBee/floor.png");
            level->initSetHoleTexture("textures/rollerBee/hole.png");
            level->initAddWallTexture("textures/rollerBee/wallFlower1.png");
            level->initAddWallTexture("textures/rollerBee/wallFlower2.png");
            level->initAddWallTexture("textures/rollerBee/wallFlower3.png");
            level->initAddWallTexture("textures/rollerBee/wallFlower4.png");
            level->doneAddingWallTextures();
            return level;
        }),
        GetFinisherFcn([](LevelTracker &tracker, float centerX, float centerY, glm::mat4 const &proj, glm::mat4 const &view) {
            auto levelFinish = tracker.getFinisher<ManyQuadCoverUpLevelFinish>(centerX, centerY, proj, view);
            for (auto &&image : std::vector<std::string>
                    {"textures/rollerBee/flower1.png",
                     "textures/rollerBee/flower2.png",
                     "textures/rollerBee/flower3.png",
                     "textures/rollerBee/flower4.png"}) {
                levelFinish->initAddTexture(image);
            }
            return levelFinish;
        })
    };
}

LevelGroup LevelTracker::getLevelGroupCat(std::shared_ptr<MazeCollectSaveData> const &levelBundle, bool needsStarter) {
    return {
        getStarterFcn(needsStarter, std::vector<std::string>{
            "In the jungle,\na mother cat...",
            "...searches\nfor her\nlost kittens."}),
        GetLevelFcn([levelBundle](LevelTracker &tracker, glm::mat4 const &proj, glm::mat4 const &view) {
            std::shared_ptr<MazeCollect> level;
            if (levelBundle == nullptr) {
                Maze::CreateParameters parameters = { 10, GeneratedMazeBoard::Mode::BFS };
                level = tracker.getLevel<MazeCollect>(parameters, proj, view);
            } else {
                level = tracker.getLevel<MazeCollect>(levelBundle, proj, view);
            }
            level->initAddWallTexture("textures/rollerBee/wallFlower1.png");
            level->initAddWallTexture("textures/rollerBee/wallFlower2.png");
            level->initAddWallTexture("textures/rollerBee/wallFlower3.png");
            level->initAddWallTexture("textures/rollerBee/wallFlower4.png");
            level->doneAddingWallTextures();
            level->initSetBallTexture("textures/cat/catBall.png");
            level->initSetFloorTexture("textures/rollerBee/floor.png");
            level->initSetHoleTexture("textures/cat/catHole.png");
            return level;
        }),
        GetFinisherFcn([](LevelTracker &tracker, float centerX, float centerY, glm::mat4 const &proj, glm::mat4 const &view) {
            auto levelFinish = tracker.getFinisher<ManyQuadCoverUpLevelFinish>(centerX, centerY, proj, view);
            levelFinish->initAddTexture("textures/pufferFish/hole.png");
            return levelFinish;
        })
    };
}

LevelGroup LevelTracker::getLevelGroupBunny(std::shared_ptr<MazeAvoidSaveData> const &levelBundle, bool needsStarter) {
    return {
            getStarterFcn(needsStarter, std::vector<std::string>{
                    "In the marsh,\na bunny seeks\na carrot...",
                    "...be carefull\nof quicksand!"}),
            GetLevelFcn([levelBundle](LevelTracker &tracker, glm::mat4 const &proj, glm::mat4 const &view) {
                std::shared_ptr<MazeAvoid> level;
                if (levelBundle == nullptr) {
                    Maze::CreateParameters parameters = { 10, GeneratedMazeBoard::Mode::DFS };
                    level = tracker.getLevel<MazeAvoid>(parameters, proj, view);
                } else {
                    level = tracker.getLevel<MazeAvoid>(levelBundle, proj, view);
                }
                level->initAddWallTexture("textures/bunny/wall1.png");
                level->initAddWallTexture("textures/bunny/wall2.png");
                level->initAddWallTexture("textures/bunny/wall3.png");
                level->doneAddingWallTextures();
                level->initSetBallTexture("textures/bunny/ball.png");
                level->initSetFloorTexture("textures/bunny/floor.png");
                level->initSetHoleTexture("textures/bunny/hole.png");
                level->initSetAvoidObjTexture("textures/bunny/avoid.png");
                return level;
            }),
            GetFinisherFcn([](LevelTracker &tracker, float centerX, float centerY, glm::mat4 const &proj, glm::mat4 const &view) {
                auto levelFinish = tracker.getFinisher<ManyQuadCoverUpLevelFinish>(centerX, centerY, proj, view);
                levelFinish->initAddTexture("textures/bunny/hole.png");
                return levelFinish;
            })
    };
}

LevelGroup LevelTracker::getLevelGroupFrog(std::shared_ptr<FixedMazeSaveData> const &levelBundle, bool needsStarter) {
    return {
            getStarterFcn(needsStarter, std::vector<std::string>{
                    "In a forest,\na frog\nseeks a fly.",
                    "Watch out for\nmushrooms, they\nare bouncy!"}),
            GetLevelFcn([levelBundle](LevelTracker &tracker, glm::mat4 const &proj, glm::mat4 const &view) {
                auto level = tracker.getLevel<FixedMaze>(levelBundle, proj, view);
                level->initSetBallInfo("models/frog/frog.modelcbor", "textures/frog/frog.png");
                level->initSetFloorInfo("models/frog/frogFloor.modelcbor", "textures/frog/frogFloor.png");
                level->initSetBounceParameters(1/20.0f, 1/50.0f);
                level->init();
                return level;
            }),
            GetFinisherFcn([](LevelTracker &tracker, float centerX, float centerY, glm::mat4 const &proj, glm::mat4 const &view) {
                auto levelFinish = tracker.getFinisher<ManyQuadCoverUpLevelFinish>(centerX, centerY, proj, view);
                levelFinish->initAddTexture("textures/bunny/hole.png");
                return levelFinish;
            })
    };
}

LevelGroup LevelTracker::getLevelGroupGopher(
        std::shared_ptr<MovablePassageSaveData> const &levelBundle,
        bool needsStarter)
{
    return {
            getStarterFcn(needsStarter, std::vector<std::string>{
                    "Underground,\na gopher\nsearches for\na juicy beet.",
                    "Move the tunnel\npieces and turn\nthem so that the\ngopher can\nreach the beet."}),
            GetLevelFcn([levelBundle](LevelTracker &tracker, glm::mat4 const &proj, glm::mat4 const &view) {
                auto level = tracker.getLevel<MovablePassage>(levelBundle, proj, view);
                level->initSetBallInfo("models/gopher/gopher.modelcbor", "textures/gopher/gopher.png");
                std::vector<std::string> textures{"textures/gopher/dirt1.png",
                                                  "textures/gopher/dirt2.png",
                                                  "textures/gopher/dirt3.png",
                                                  "textures/gopher/dirt4.png"};
                std::vector<std::string> rockModels{
                        "models/gopher/rock1.modelcbor",
                        "models/gopher/rock2.modelcbor",
                        "models/gopher/rock3.modelcbor",
                        "models/gopher/rock4.modelcbor"
                };
                level->initSetGameBoardInfo(
                        "textures/gopher/dirt1.png",
                        rockModels,
                        "textures/gopher/rock.png",
                        textures,
                        "textures/gopher/end.png",
                        "textures/gopher/endOffBoard.png",
                        "models/gopher/startCorner.modelcbor", "textures/gopher/dirt1.png",
                        "models/gopher/startSide.modelcbor", textures,
                        "models/gopher/startOpen.modelcbor", textures);
                level->initAddRock(1,1);
                level->initAddRock(1,2);
                level->initAddRock(1,3);
                level->initAddRock(1,4);
                level->initAddRock(1,5);
                level->initAddRock(1,6);
                level->initAddRock(3,0);
                level->initAddRock(3,2);
                level->initAddRock(3,3);
                level->initAddRock(3,5);
                level->initAddRock(5,1);
                level->initAddRock(5,2);
                level->initAddRock(5,3);
                level->initAddRock(5,5);
                level->initAddRock(5,6);
                level->initAddRock(6,5);
                level->initAddRock(6,6);
                level->initAddRock(7,0);
                level->initAddRock(7,1);
                level->initAddRock(7,2);
                level->initAddRock(7,3);
                level->initAddType(Component::ComponentType::straight, 3,
                    "models/gopher/straight.modelcbor", "textures/gopher/dirt.png");
                level->initAddType(Component::ComponentType::turn, 10,
                    "models/gopher/turn.modelcbor", "textures/gopher/dirt.png");
                level->initAddType(Component::ComponentType::tjunction, 2,
                    "models/gopher/tjunction.modelcbor", "textures/gopher/dirt.png");
                level->initAddType(Component::ComponentType::crossjunction, 2,
                    "models/gopher/crossjunction.modelcbor", "textures/gopher/dirt.png");
                level->initSetGameBoard(8, 8, 1, 6);
                level->initDone();

                // call after all other init functions are completed but before updateStaticDrawObjects
                auto extraWHatZRequested = level->getAdditionalWHatZRequests();
                for (auto const &extraZ : extraWHatZRequested) {
                    auto extraWH = getWidthHeight(extraZ, proj, view);
                    level->setAdditionalWH(extraWH.first, extraWH.second, extraZ);
                }

                return level;
            }),
            GetFinisherFcn([](LevelTracker &tracker, float centerX, float centerY, glm::mat4 const &proj, glm::mat4 const &view) {
                auto levelFinish = tracker.getFinisher<ManyQuadCoverUpLevelFinish>(centerX, centerY, proj, view);
                levelFinish->initAddTexture("textures/bunny/hole.png");
                return levelFinish;
            })
    };
}


LevelGroup LevelTracker::getLevelGroupMouse(
        std::shared_ptr<RotatablePassageSaveData> const &levelBundle,
        bool needsStarter)
{
    return {
            getStarterFcn(needsStarter, std::vector<std::string>{
                    "A mouse\nsearches for a\npiece of cheese\nin a hedge maze.",
                    "Help the mouse\nby turning the\nhedge maze\npieces so that\nthere is a path\nto the cheese."}),
            GetLevelFcn([levelBundle](LevelTracker &tracker, glm::mat4 const &proj, glm::mat4 const &view) {
                auto level = tracker.getLevel<RotatablePassage>(levelBundle, proj, view);
                level->initSetBallInfo("models/mouse/mouse.modelcbor", "textures/mouse/mouse.png");
                level->initSetHoleInfo("models/mouse/cheese.modelcbor", "textures/mouse/cheese.png");
                std::vector<std::string> textures{"textures/rollerBee/wallFlower1.png",
                                                  "textures/rollerBee/wallFlower2.png",
                                                  "textures/rollerBee/wallFlower3.png",
                                                  "textures/rollerBee/wallFlower4.png"};
                level->initSetGameBoardInfo(
                        "models/gopher/straight.modelcbor",
                        "textures/mouse/wall.png",
                        "textures/mouse/wallWithFlower.png",
                        "models/gopher/tjunction.modelcbor",
                        "textures/mouse/wall.png",
                        "textures/mouse/wallWithFlower.png",
                        "models/gopher/crossjunction.modelcbor",
                        "textures/mouse/wall.png",
                        "textures/mouse/wallWithFlower.png",
                        "models/gopher/turn.modelcbor",
                        "textures/mouse/wall.png",
                        "textures/mouse/wallWithFlower.png",
                        "models/movablePassage/deadEnd.modelcbor",
                        "textures/mouse/wall.png",
                        "textures/mouse/wallWithFlower.png",
                        textures);
                level->initSetGameBoard(8, GeneratedMazeBoard::Mode::DFS);

                // call after all other init functions are completed but before updateStaticDrawObjects
                auto extraWHatZRequested = level->getAdditionalWHatZRequests();
                for (auto const &extraZ : extraWHatZRequested) {
                    auto extraWH = getWidthHeight(extraZ, proj, view);
                    level->setAdditionalWH(extraWH.first, extraWH.second, extraZ);
                }

                return level;
            }),
            GetFinisherFcn([](LevelTracker &tracker, float centerX, float centerY, glm::mat4 const &proj, glm::mat4 const &view) {
                auto levelFinish = tracker.getFinisher<ManyQuadCoverUpLevelFinish>(centerX, centerY, proj, view);
                levelFinish->initAddTexture("textures/bunny/hole.png");
                return levelFinish;
            })
    };
}

void LevelTracker::gotoNextLevel() {
    m_currentLevel = (m_currentLevel + 1) % getLevelTable().size();
}

std::vector<std::string> LevelTracker::getLevelDescriptions() {
    std::vector<std::string> ret;

    for (size_t i = 0; i < getLevelTable().size(); i++) {
        ret.push_back(getLevelTable()[i].levelDescription);
    }

    return ret;
}
