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

#ifndef AMAZING_LABYRINTH_ROTATABLE_PASSAGE_LEVEL_HPP
#define AMAZING_LABYRINTH_ROTATABLE_PASSAGE_LEVEL_HPP

#include <chrono>
#include <vector>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../basic/level.hpp"
#include "../movablePassageAlgorithms.hpp"
#include "../generatedMazeAlgorithms.hpp"

#include "loadData.hpp"

// All models expected to be exported with Z-up, Y-forward.

namespace rotatablePassage {
    class Level : public basic::Level {
    public:
        static char constexpr const *m_name = "rotatablePassage";

        bool tap(float x, float y) override {
            auto projView = m_levelDrawer.getProjectionView();

            auto XY = getXYAtZ(x, y, m_mazeFloorZ, projView.first, projView.second);

            glm::vec2 position{XY.first, XY.second};
            return m_gameBoard.tap(position);
        }

        bool updateData() override;

        bool updateDrawObjects() override;

        void addStaticDrawObjects();

        void addDynamicDrawObjects();

        void start() override { m_prevTime = std::chrono::high_resolution_clock::now(); }

        void getLevelFinisherCenter(float &x, float &y, float &z) override {
            auto endPos = m_gameBoard.position(m_endRow, m_endCol);
            x = endPos.x;
            y = endPos.y;
            z = m_mazeFloorZ;
        };

        char const *name() override { return m_name; }

        std::vector<uint8_t> saveData(levelTracker::GameSaveData const &gsd,
                                      char const *saveLevelDataKey) override;

        Level(
            levelDrawer::Adaptor inLevelDrawer,
            std::shared_ptr<LevelConfigData> const &lcd,
            std::shared_ptr<LevelSaveData> const &sd,
            float maxZ)
            : basic::Level(inLevelDrawer, lcd, maxZ, true),
              m_gameBoard(),
              m_components{
                      std::make_shared<Component>(Component::ComponentType::straight),
                      std::make_shared<Component>(Component::ComponentType::tjunction),
                      std::make_shared<Component>(Component::ComponentType::crossjunction),
                      std::make_shared<Component>(Component::ComponentType::turn),
                      std::make_shared<Component>(Component::ComponentType::deadEnd)},
              m_ballRow{0},
              m_ballCol{0},
              m_endRow{0},
              m_endCol{0},
              m_objIndexBall{0},
              m_objDataIndexBall{0},
              m_holeModel{lcd->holeModel},
              m_holeTexture{lcd->holeTexture}
        {
            m_levelDrawer.setClearColor(glm::vec4{0.0f, 0.0f, 0.0f, 1.0f});

            m_componentModels[Component::ComponentType::straight].push_back(lcd->straight.model);
            m_componentTextures[Component::ComponentType::straight].push_back(lcd->straight.texture);
            m_componentTexturesLockedInPlace[Component::ComponentType::straight].push_back(
                    lcd->straight.lockedInPlaceTexture);

            m_componentModels[Component::ComponentType::tjunction].push_back(lcd->tJunction.model);
            m_componentTextures[Component::ComponentType::tjunction].push_back(lcd->tJunction.texture);
            m_componentTexturesLockedInPlace[Component::ComponentType::tjunction].push_back(
                    lcd->tJunction.lockedInPlaceTexture);

            m_componentModels[Component::ComponentType::crossjunction].push_back(
                    lcd->crossJunction.model);
            m_componentTextures[Component::ComponentType::crossjunction].push_back(
                    lcd->crossJunction.texture);
            m_componentTexturesLockedInPlace[Component::ComponentType::crossjunction].push_back(
                    lcd->crossJunction.lockedInPlaceTexture);

            m_componentModels[Component::ComponentType::turn].push_back(lcd->turn.model);
            m_componentTextures[Component::ComponentType::turn].push_back(lcd->turn.texture);
            m_componentTexturesLockedInPlace[Component::ComponentType::turn].push_back(
                    lcd->turn.lockedInPlaceTexture);

            m_componentModels[Component::ComponentType::deadEnd].push_back(lcd->deadEnd.model);
            m_componentTextures[Component::ComponentType::deadEnd].push_back(lcd->deadEnd.texture);
            m_componentTexturesLockedInPlace[Component::ComponentType::deadEnd].push_back(
                    lcd->deadEnd.lockedInPlaceTexture);

            m_borderTextures = lcd->borderTextures;

            if (sd) {
                initSetGameBoardFromSaveData(sd);
            } else {
                initSetGameBoard(lcd->numberRows,
                                 lcd->dfsSearch ? GeneratedMazeBoard::Mode::DFS
                                                : GeneratedMazeBoard::Mode::BFS);
            }

            m_prevTime = std::chrono::high_resolution_clock::now();
            m_scaleBall = m_gameBoard.blockSize()/2/m_originalBallDiameter;

            for (auto &component : m_components) {
                component->setSize(m_gameBoard.blockSize());
            }

            addStaticDrawObjects();
            addDynamicDrawObjects();
        }

    private:
        std::chrono::high_resolution_clock::time_point m_prevTime;

        Random m_randomNumbers;

        GameBoard m_gameBoard;

        std::array<std::shared_ptr<Component>,
                Component::ComponentType::maxComponentJunctionType + 1> m_components;

        uint32_t m_ballRow;
        uint32_t m_ballCol;

        uint32_t m_ballStartRow;
        uint32_t m_ballStartCol;

        uint32_t m_endRow;
        uint32_t m_endCol;

        size_t m_objIndexBall;
        size_t m_objDataIndexBall;

        std::string m_holeModel;
        std::string m_holeTexture;

        std::array<std::vector<std::string>,
                Component::ComponentType::maxComponentType + 1> m_componentModels;
        std::array<std::vector<std::string>,
                Component::ComponentType::maxComponentType + 1> m_componentTextures;
        std::array<std::vector<std::string>,
                Component::ComponentType::maxComponentType + 1> m_componentTexturesLockedInPlace;

        std::vector<std::string> m_borderTextures;

        void initSetGameBoard(uint32_t numberRows, GeneratedMazeBoard::Mode mode);
        void initSetGameBoardFromSaveData(std::shared_ptr<LevelSaveData> const &sd);
    };
} // namespace rotatablePassage
#endif // AMAZING_LABYRINTH_ROTATABLE_PASSAGE_LEVEL_HPP
