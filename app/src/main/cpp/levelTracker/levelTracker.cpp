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
#include "internals.hpp"

namespace levelTracker {
    namespace DataVariables {
        char constexpr const *Levels = "Levels";
        char constexpr const *Name = "Name";
        char constexpr const *File = "File";
        char constexpr const *Components = "Components";
        char constexpr const *Starter = "Starter";
        char constexpr const *Level = "Level";
        char constexpr const *Finisher = "Finisher";
    }

    float constexpr Loader::m_maxZLevelStarter;
    float constexpr Loader::m_maxZLevel;
    float constexpr Loader::m_maxZLevelFinisher;

    LevelMapTable &starterTable() {
        static std::unordered_map<std::string, GenerateLevelFcn> map;
        return map;
    }

    LevelMapTable &levelTable() {
        static std::unordered_map<std::string, GenerateLevelFcn> map;
        return map;
    }

    FinisherMapTable &finisherTable() {
        static std::unordered_map<std::string, GenerateLevelFcn> map;
        return map;
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

    void from_json(nlohmann::json const &j, LevelTableEntry &val) {
        val.levelName = j[DataVariables::Name].get<std::string>();
        val.fileName = j[DataVariables::File].get<std::string>();
    }

    void Loader::gotoNextLevel() {
        m_currentLevel = (m_currentLevel + 1) % m_levelTable.size();
    }

    struct Components {
        std::string starter;
        std::string level;
        std::string finisher;

        Components() = default;

        Components(Components &&other) noexcept
            : starter{std::move(other.starter)},
              level{std::move(other.level)},
              finisher{std::move(other.finisher)}
        {}
    };

    void from_json(nlohmann::json const &j, Components &val) {
        val.starter = j[DataVariables::Starter].get<std::string>();
        val.level = j[DataVariables::Level].get<std::string>();
        val.finisher = j[DataVariables::Finisher].get<std::string>();
    }

    LevelGroup Loader::getLevelGroupFcns() {
        nlohmann::json *jsdLevel = nullptr;
        nlohmann::json j = nlohmann::json::from_cbor(getDataFromFile(m_levelTable[m_currentLevel].fileName));
        Components components = j[DataVariables::Components].get<Components>();

        LevelGroup group;
        auto starterIt = starterTable().find(components.starter);
        if (starterIt == starterTable().end()) {
            throw std::runtime_error("Invalid level starter.");
        }

        group.getStarterFcn = starterIt->second(nullptr, j[DataVariables::Starter]);

        auto levelIt = levelTable().find(components.level);
        if (levelIt == levelTable().end()) {
            throw std::runtime_error("Invalid level");
        }

        group.getLevelFcn = levelIt->second(jsdLevel, j[DataVariables::Level]);

        auto finisherIt = finisherTable().find(components.finisher);
        if (finisherIt == finisherTable().end()) {
            throw std::runtime_error("Invalid finisher");
        }

        group.getFinisherFcn = finisherIt->second(nullptr, j[DataVariables::Finisher]);
        
        return group;
    }

    Loader::Loader(
        std::shared_ptr<GameRequester> inGameRequester)
        : m_gameRequester{std::move(inGameRequester)},
          m_currentLevel{}
    {
        nlohmann::json j = nlohmann::json::from_cbor(getDataFromFile(m_gameRequester->getLevelTableName()));
        m_levelTable = j[DataVariables::Levels].get<std::vector<LevelTableEntry>>();
    }
}
