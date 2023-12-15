/**
 * Copyright 2023 Cerulean Quasar. All Rights Reserved.
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

#ifndef AMAZING_LABYRINTH_MOVABLE_PASSAGE_LEVEL_HPP
#define AMAZING_LABYRINTH_MOVABLE_PASSAGE_LEVEL_HPP

#include <chrono>
#include <functional>
#include <memory>
#include <vector>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../common.hpp"
#include "../../random.hpp"
#include "../basic/level.hpp"
#include "../movablePassageAlgorithms.hpp"
#include "loadData.hpp"

// All models expected to be exported with Z-up, Y-forward.
namespace movablePassage {
    class Level : public basic::Level {
    public:
        static char constexpr const *m_name = "movablePassage";
        bool drag(float startX, float startY, float distanceX, float distanceY) override;

        bool dragEnded(float x, float y) override;

        bool tap(float x, float y) override;

        bool updateData() override;

        void addStaticDrawObjects();

        void addDynamicDrawObjects();

        bool updateDrawObjects() override;

        void start() override { m_prevTime = std::chrono::high_resolution_clock::now(); }

        void getLevelFinisherCenter(float &x, float &y, float &z) override {
            auto endPos = m_gameBoard.position(
                    m_gameBoard.heightInTiles() - m_nbrTileRowsForEnd, m_columnEndPosition);

            x = endPos.x;
            y = endPos.y;
            z = endPos.z;
        };

        char const *name() override { return m_name; }

        std::vector<uint8_t> saveData(levelTracker::GameSaveData const &gsd,
                                      char const *saveLevelDataKey) override;

        ~Level() override = default;

        Level(
            levelDrawer::Adaptor inLevelDrawer,
            std::shared_ptr<LevelConfigData> const &lcd,
            std::shared_ptr<LevelSaveData> const &sd,
            float maxZ)
            : basic::Level(std::move(inLevelDrawer), lcd, maxZ, true),
              m_random{},
              m_zdrawTopsOfObjects{ m_mazeFloorZ },
              m_zMovingPlacement{ m_mazeFloorZ + m_scaleBall },
              m_components{
                        std::make_shared<Component>(Component::ComponentType::straight),
                        std::make_shared<Component>(Component::ComponentType::tjunction),
                        std::make_shared<Component>(Component::ComponentType::crossjunction),
                        std::make_shared<Component>(Component::ComponentType::turn),
                        std::make_shared<Component>(Component::ComponentType::deadEnd),
                        std::make_shared<Component>(Component::ComponentType::open),
                        std::make_shared<Component>(Component::ComponentType::closedBottom),
                        std::make_shared<Component>(Component::ComponentType::closedCorner),
                        std::make_shared<Component>(Component::ComponentType::noMovementDirt),
                        std::make_shared<Component>(Component::ComponentType::noMovementRock)
              },
              m_gameBoard{},
              m_nbrComponents{ 0 },
              m_initDone{ false },
              m_objsIndexBall{ 0 }
        {
            m_levelDrawer.setClearColor(glm::vec4{0.0f, 0.0f, 0.0f, 1.0f});

            auto const &rockModels = findModelsAndTextures(ModelNameRock);
            m_componentModels[Component::ComponentType::noMovementRock] = rockModels.models;
            m_componentTextures[Component::ComponentType::noMovementRock] = rockModels.textures;

            auto const &dirtModels = findModelsAndTextures(ModelNameDirt);
            m_componentModels[Component::ComponentType::noMovementDirt] = dirtModels.models;
            m_componentTextures[Component::ComponentType::noMovementDirt] = dirtModels.textures;

            auto const &startModels = findModelsAndTextures(ModelNameStartCorner);
            m_componentModels[Component::ComponentType::closedCorner] = startModels.models;
            m_componentTextures[Component::ComponentType::closedCorner] = startModels.textures;

            auto const &sideModels = findModelsAndTextures(ModelNameStartSide);
            m_componentModels[Component::ComponentType::closedBottom] = sideModels.models;
            m_componentTextures[Component::ComponentType::closedBottom] = sideModels.textures;

            auto const &openModels = findModelsAndTextures(ModelNameStartCenter);
            m_componentModels[Component::ComponentType::open] = openModels.models;
            m_componentTextures[Component::ComponentType::open] = openModels.textures;

            for (auto const &rock : lcd->rockPlacements) {
                m_addedRocks.emplace_back(rock.row, rock.col);
            }

            initAddType(Component::ComponentType::straight, lcd->nbrPlacements.straight,
                               ModelNameStraight);
            initAddType(Component::ComponentType::turn, lcd->nbrPlacements.turn,
                               ModelNameTurn);
            initAddType(Component::ComponentType::tjunction, lcd->nbrPlacements.tJunction,
                               ModelNameTJunction);
            initAddType(Component::ComponentType::crossjunction, lcd->nbrPlacements.crossJunction,
                               ModelNameCrossJunction);

            initSetGameBoard(lcd->numberTilesX, lcd->numberTilesY, lcd->startColumn, lcd->endColumn);

            initAddPlayableComponents(sd);

            initDone(sd);

            auto projView = m_levelDrawer.getProjectionView();
            // do after the game board is initialized.
            auto wh = getWidthHeight(m_gameBoard.getZPosEndTile(), projView.first, projView.second);
            m_gameBoard.setCenterPos(glm::vec3{0.0f, (wh.second - m_gameBoard.heightInTiles() *
                                                               m_gameBoard.blockSize()) / 2.0f,
                                               m_mazeFloorZ});

            addStaticDrawObjects();
            addDynamicDrawObjects();
        }

    protected:
        static char const constexpr *ModelNameStartCorner = "StartCorner";
        static char const constexpr *ModelNameStartSide = "StartSide";
        static char const constexpr *ModelNameStartCenter = "StartCenter";
        static char const constexpr *ModelNameRock = "Rock";
        static char const constexpr *ModelNameDirt = "Dirt";
        static char const constexpr *ModelNameStraight = "Straight";
        static char const constexpr *ModelNameTJunction = "TJunction";
        static char const constexpr *ModelNameTurn = "Turn";
        static char const constexpr *ModelNameCrossJunction = "CrossJunction";
        static char const constexpr *ModelNameEnd = "End";
        static char const constexpr *ModelNameEndOffBoard = "EndOffBoard";

    private:
        static float constexpr m_offBoardComponentScaleMultiplier = 2.0f/3.0f;
        Random m_random;
        float const m_zMovingPlacement;
        float const m_zdrawTopsOfObjects;
        uint32_t m_ballRow;
        uint32_t m_ballCol;
        std::chrono::high_resolution_clock::time_point m_prevTime;
        static uint32_t constexpr m_nbrTileRowsForStart = 3;
        static uint32_t constexpr m_nbrTileRowsForEnd = 2;

        std::array<std::shared_ptr<Component>,
                Component::ComponentType::maxComponentType + 1> m_components;

        GameBoard m_gameBoard;
        uint32_t m_nbrComponents;
        bool m_initDone;

        size_t m_objsIndexBall;
        size_t m_objDataIndexBall;

        std::array<std::vector<std::shared_ptr<levelDrawer::ModelDescription>>,
                Component::ComponentType::maxComponentType + 1> m_componentModels;
        std::array<std::vector<std::shared_ptr<levelDrawer::TextureDescription>>,
                Component::ComponentType::maxComponentType + 1> m_componentTextures;
        std::array<std::vector<std::shared_ptr<levelDrawer::TextureDescription>>,
                Component::ComponentType::maxComponentType + 1> m_componentTexturesLockedInPlace;

        // rocks row and col do not consider the off game squares or the start or end.  Just
        // the middle of the game board.  (0,0) is at the bottom left part of the board.
        std::vector<std::pair<uint32_t, uint32_t>> m_addedRocks;

        // the row and column which starts the area in which users can place blocks
        std::pair<uint32_t, uint32_t> m_gameBoardStartRowColumn;

        // the row and column which ends the area in which users can place blocks
        std::pair<uint32_t, uint32_t> m_gameBoardEndRowColumn;

        // the first position that the ball is allowed to roll to that may contain a user placeable
        // component.
        std::pair<uint32_t, uint32_t> m_ballFirstPlaceableComponent;

        // the column in which the end tile is located.
        uint32_t m_columnEndPosition;

        // all the components should be added before calling this function.
        void initSetGameBoard(
                uint32_t nbrTilesX,
                uint32_t nbrTilesY,
                uint32_t startCplumn,
                uint32_t endColumn);

        // this function can be called anytime before initSetGameBoard. This function must be called for
        // Component::ComponentType::straight even if the user is not allowed to use the straight
        // component.
        void initAddType(
                Component::ComponentType inComponentType,
                uint32_t nbrComponents,
                std::string const &modelName) {

            auto const &modelData = findModelsAndTextures(modelName);

            /* the model */
            m_componentModels[inComponentType].push_back(modelData.models[0]);

            /* the texture */
            m_componentTextures[inComponentType].push_back(getFirstTexture(modelData, false));

            /* add the locked in place texture */
            m_componentTexturesLockedInPlace[inComponentType].push_back(
                    getFirstTexture(modelData, false, true));

            for (uint32_t i = 0; i < nbrComponents; i++) {
                m_components[inComponentType]->add();
                m_nbrComponents++;
            }
        }

        void initAddPlayableComponents(std::shared_ptr<LevelSaveData> const &sd);

        // all other init functions before calling this function or starting the level
        void initDone(std::shared_ptr<LevelSaveData> const &sd);
    };
} // namespace movablePassage

#endif /* AMAZING_LABYRINTH_MOVABLE_PASSAGE_LEVEL_HPP */
