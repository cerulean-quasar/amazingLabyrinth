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

#ifndef AMAZING_LABYRINTH_MOVABLE_PASSAGE_HPP
#define AMAZING_LABYRINTH_MOVABLE_PASSAGE_HPP

#include <chrono>
#include <functional>
#include <memory>
#include <vector>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../common.hpp"
#include "../../saveData.hpp"
#include "../level.hpp"
#include "../movablePassageAlgorithms.hpp"

struct MovablePassageSaveData : public LevelSaveData {
    static int constexpr m_movablePassageVersion = 1;
    MovablePassageSaveData() : LevelSaveData{m_movablePassageVersion} {}
};

// All models expected to be exported with Z-up, Y-forward.
class MovablePassage : public Level {
public:
    bool drag(float startX, float startY, float distanceX, float distanceY) override;
    bool dragEnded(float x, float y) override;
    bool tap(float x, float y) override;
    glm::vec4 getBackgroundColor() override { return {0.6f, 0.8f, 1.0f, 1.0f}; }
    bool updateData() override;
    bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) override;
    bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged) override;
    void start() override { m_prevTime = std::chrono::high_resolution_clock::now(); }
    void getLevelFinisherCenter(float &x, float &y) override { x = 0.0f; y = 0.0f; };
    SaveLevelDataFcn getSaveLevelDataFcn() override;

    // call after all other init functions are completed but before updateStaticDrawObjects
    std::vector<float> getAdditionalWHatZRequests() override {
        return std::vector<float>{m_gameBoard.getZPosEndTile()};
    }

    // call after getAdditionalWHatZRequests to return the requested width/height at z.
    void setAdditionalWH(float, float height, float z) override {
        if (z == m_gameBoard.getZPosEndTile()) {
            m_gameBoard.setCenterPos(glm::vec3{0.0f, (height - m_gameBoard.heightInTiles()* m_gameBoard.blockSize())/2.0f, m_mazeFloorZ});
        }
    }

    // this function can be called at any time before the level is started.
    void initSetBallInfo(
        std::string const &ballModel,
        std::string const &ballTexture)
    {
        m_ballModel = ballModel;
        m_ballTextureName = ballTexture;
    }

    // this function can be called at any time before the level is started.
    void initSetGameBoardInfo(
            std::string const &lockedComponentTexture,
            std::vector<std::string> const &blockedRockModel,
            std::string const &blockedRockTexture,
            std::vector<std::string> const &blockedDirtTexture,
            std::string const &componentTextureEnd,
            std::string const &textureEndOffBoard,
            std::string const &startCornerModel,
            std::string const &startCornerTexture,
            std::string const &startSideModel,
            std::vector<std::string> const &startSideTexture,
            std::string const &startOpenModel,
            std::vector<std::string> const &startOpenTexture)
    {
        m_textureLockedComponent = lockedComponentTexture;

        m_componentModels[Component::ComponentType::noMovementRock] = blockedRockModel;
        m_componentTextures[Component::ComponentType::noMovementRock].push_back(blockedRockTexture);
        m_componentTextures[Component::ComponentType::noMovementDirt] = blockedDirtTexture;

        m_componentTextureEnd = componentTextureEnd;
        m_textureEndOffBoard = textureEndOffBoard;

        m_componentModels[Component::ComponentType::closedCorner].push_back(startCornerModel);
        m_componentTextures[Component::ComponentType::closedCorner].push_back(startCornerTexture);

        m_componentModels[Component::ComponentType::closedBottom].push_back(startSideModel);
        m_componentTextures[Component::ComponentType::closedBottom] = startSideTexture;

        m_componentModels[Component::ComponentType::open].push_back(startOpenModel);
        m_componentTextures[Component::ComponentType::open] = startOpenTexture;
    }

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
        std::string const &texture)
    {
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

    // this function can be called at any time before initDone.
    void initAddRock(
        uint32_t row,
        uint32_t col)
    {
        m_addedRocks.emplace_back(row, col);
    }

    // all other init functions before calling this function or starting the level
    void initDone();

    ~MovablePassage() override = default;

    MovablePassage(
            std::shared_ptr<GameRequester> inGameRequester,
            std::shared_ptr<MovablePassageSaveData> /*sd*/,
            float width, float height, float maxZ)
            : Level(inGameRequester, width, height, maxZ, true, 1/50.0f, false),
              m_random{},
              m_zdrawTopsOfObjects{m_mazeFloorZ},
              m_zMovingPlacement{m_mazeFloorZ + m_scaleBall},
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
                      std::make_shared<Component>(Component::ComponentType::noMovementRock)},
              m_gameBoard{},
              m_nbrComponents{0},
              m_texturesChanged{true},
              m_initDone{false},
              m_objsReferenceBall{0}
    {
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

    std::array<std::shared_ptr<Component>, Component::ComponentType::maxComponentType + 1> m_components;

    GameBoard m_gameBoard;
    uint32_t m_nbrComponents;
    bool m_texturesChanged;
    bool m_initDone;

    std::string m_ballModel;
    std::string m_ballTextureName;
    uint32_t m_objsReferenceBall;

    std::array<std::vector<std::string>, Component::ComponentType::maxComponentType + 1> m_componentModels;
    std::array<std::vector<std::string>, Component::ComponentType::maxComponentType + 1> m_componentTextures;
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

    // the column in which the end tile is located.
    uint32_t m_columnEndPosition;
};
#endif /* AMAZING_LABYRINTH_MOVABLE_PASSAGE_HPP */
