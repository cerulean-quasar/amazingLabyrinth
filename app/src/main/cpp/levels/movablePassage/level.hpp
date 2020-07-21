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
              m_texturesChanged{ true },
              m_initDone{ false },
              m_objsIndexBall{ 0 }
        {
            m_levelDrawer.setClearColor(glm::vec4{0.6f, 0.8f, 1.0f, 1.0f});

            m_textureLockedComponent = lcd->placementLockedInPlaceTexture;

            m_componentModels[Component::ComponentType::noMovementRock] = lcd->rockModels;
            m_componentTextures[Component::ComponentType::noMovementRock] = lcd->rockTextures;

            m_componentModels[Component::ComponentType::noMovementDirt] = lcd->dirtModels;
            m_componentTextures[Component::ComponentType::noMovementDirt] = lcd->dirtTextures;

            m_componentTextureEnd = lcd->endTexture;
            m_textureEndOffBoard = lcd->endOffBoardTexture;

            m_componentModels[Component::ComponentType::closedCorner] = lcd->beginningCornerModels;
            m_componentTextures[Component::ComponentType::closedCorner] = lcd->beginningCornerTextures;

            m_componentModels[Component::ComponentType::closedBottom] = lcd->beginningSideModels;
            m_componentTextures[Component::ComponentType::closedBottom] = lcd->beginningSideTextures;

            m_componentModels[Component::ComponentType::open] = lcd->beginningOpenModels;
            m_componentTextures[Component::ComponentType::open] = lcd->beginningOpenTextures;

            for (auto const &rock : lcd->rockPlacements) {
                m_addedRocks.emplace_back(rock.row, rock.col);
            }

            initAddType(Component::ComponentType::straight, lcd->straight.numberPlacements,
                               lcd->straight.model, lcd->straight.texture);
            initAddType(Component::ComponentType::turn, lcd->turn.numberPlacements,
                               lcd->turn.model, lcd->turn.texture);
            initAddType(Component::ComponentType::tjunction, lcd->tjunction.numberPlacements,
                               lcd->tjunction.model, lcd->tjunction.texture);
            initAddType(Component::ComponentType::crossjunction, lcd->crossjunction.numberPlacements,
                               lcd->crossjunction.model, lcd->crossjunction.texture);

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

    private:
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
        bool m_texturesChanged;
        bool m_initDone;

        size_t m_objsIndexBall;
        size_t m_objDataIndexBall;

        std::array<std::vector<std::string>,
                Component::ComponentType::maxComponentType + 1> m_componentModels;
        std::array<std::vector<std::string>,
                Component::ComponentType::maxComponentType + 1> m_componentTextures;
        std::string m_componentTextureEnd;
        std::string m_textureEndOffBoard;
        std::string m_textureLockedComponent;

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

        // all the compoents should be added before calling this function.
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
                std::string const &model,
                std::string const &texture) {
            if (!texture.empty()) {
                m_componentTextures[inComponentType].push_back(texture);
            }
            if (!model.empty()) {
                m_componentModels[inComponentType].push_back(model);
            }
            for (uint32_t i = 0; i < nbrComponents; i++) {
                m_components[inComponentType]->add();
                m_nbrComponents++;
            }
        }

        void initAddPlayableComponents(std::shared_ptr<LevelSaveData> const &sd);

        // all other init functions before calling this function or starting the level
        void initDone(std::shared_ptr<LevelSaveData> const &sd);

        void addComponentModelMatrices(boost::optional<size_t> const &ballReference);
    };
} // namespace movablePassage

#endif /* AMAZING_LABYRINTH_MOVABLE_PASSAGE_LEVEL_HPP */
