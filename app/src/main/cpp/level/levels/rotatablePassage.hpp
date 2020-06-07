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

#ifndef AMAZING_LABYRINTH_ROTATABLE_PASSAGE_HPP
#define AMAZING_LABYRINTH_ROTATABLE_PASSAGE_HPP

#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../level.hpp"
#include "../../saveData.hpp"
#include "../../graphics.hpp"
#include "../movablePassageAlgorithms.hpp"
#include "../generatedMazeAlgorithms.hpp"

struct RotatablePassageSaveData : public LevelSaveData {
    static int constexpr m_movablePassageVersion = 1;
    RotatablePassageSaveData() : LevelSaveData{m_movablePassageVersion} {}
};

// All models expected to be exported with Z-up, Y-forward.
class RotatablePassage : public Level {
public:
    bool tap(float x, float y) override {
        glm::vec2 position{x, y};
        return m_gameBoard.tap(position);
    }

    glm::vec4 getBackgroundColor() override { return {0.0f, 0.0f, 0.0f, 1.0f}; }

    bool updateData() override;

    bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) override;

    bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures,
                                  bool &texturesChanged) override;

    void start() override { m_prevTime = std::chrono::high_resolution_clock::now(); }

    void getLevelFinisherCenter(float &x, float &y) override {
        x = 0.0f;
        y = 0.0f;
    };

    SaveLevelDataFcn getSaveLevelDataFcn() override;

    // this function can be called at any time before the level is started.
    void initSetBallInfo(
            std::string const &ballModel,
            std::string const &ballTexture)
    {
        m_ballModel = ballModel;
        m_ballTextureName = ballTexture;
    }

    // this function can be called at any time before the level is started.
    void initSetHoleInfo(
            std::string const &holeModel,
            std::string const &holeTexture)
    {
        m_holeModel = holeModel;
        m_holeTexture = holeTexture;
    }

    // this function can be called at any time before the level is started.
    void initSetGameBoardInfo(
            std::string const &straightModel,
            std::string const &straightTexture,
            std::string const &straightTextureLockedInPlace,
            std::string const &tjunctionModel,
            std::string const &tjunctionTexture,
            std::string const &tjunctionTextureLockedInPlace,
            std::string const &crossjunctionModel,
            std::string const &crossjunctionTexture,
            std::string const &crossjunctionTextureLockedInPlace,
            std::string const &turnModel,
            std::string const &turnTexture,
            std::string const &turnTextureLockedInPlace,
            std::string const &deadEndModel,
            std::string const &deadEndTexture,
            std::string const &deadEndTextureLockedInPlace,
            std::vector<std::string> const &borderTextures)
    {
        m_componentModels[Component::ComponentType::straight].push_back(straightModel);
        m_componentTextures[Component::ComponentType::straight].push_back(straightTexture);
        m_componentTexturesLockedInPlace[Component::ComponentType::straight].push_back(straightTextureLockedInPlace);

        m_componentModels[Component::ComponentType::tjunction].push_back(tjunctionModel);
        m_componentTextures[Component::ComponentType::tjunction].push_back(tjunctionTexture);
        m_componentTexturesLockedInPlace[Component::ComponentType::tjunction].push_back(tjunctionTextureLockedInPlace);

        m_componentModels[Component::ComponentType::crossjunction].push_back(crossjunctionModel);
        m_componentTextures[Component::ComponentType::crossjunction].push_back(crossjunctionTexture);
        m_componentTexturesLockedInPlace[Component::ComponentType::crossjunction].push_back(crossjunctionTextureLockedInPlace);

        m_componentModels[Component::ComponentType::turn].push_back(turnModel);
        m_componentTextures[Component::ComponentType::turn].push_back(turnTexture);
        m_componentTexturesLockedInPlace[Component::ComponentType::turn].push_back(turnTextureLockedInPlace);

        m_componentModels[Component::ComponentType::deadEnd].push_back(deadEndModel);
        m_componentTextures[Component::ComponentType::deadEnd].push_back(deadEndTexture);
        m_componentTexturesLockedInPlace[Component::ComponentType::deadEnd].push_back(deadEndTextureLockedInPlace);

        m_borderTextures = borderTextures;
    }

    // this function can be called at any time before the level is started.
    void initSetGameBoard(uint32_t nbrTilesX, GeneratedMazeBoard::Mode mode);

    RotatablePassage(
        std::shared_ptr<GameRequester> inGameRequester,
        std::shared_ptr<RotatablePassageSaveData> /*sd*/,
        float width, float height, float maxZ)
        : Level(inGameRequester, width, height, maxZ, true, 1/50.0f, false),
          m_initDone{false},
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
          m_objsReferenceBall{0}
    {}
private:
    std::chrono::high_resolution_clock::time_point m_prevTime;
    bool m_initDone;

    Random m_randomNumbers;

    GameBoard m_gameBoard;

    std::array<std::shared_ptr<Component>, Component::ComponentType::maxComponentJunctionType + 1> m_components;

    uint32_t m_ballRow;
    uint32_t m_ballCol;

    uint32_t m_endRow;
    uint32_t m_endCol;

    std::string m_ballModel;
    std::string m_ballTextureName;
    size_t m_objsReferenceBall;

    std::string m_holeModel;
    std::string m_holeTexture;

    std::array<std::vector<std::string>, Component::ComponentType::maxComponentType + 1> m_componentModels;
    std::array<std::vector<std::string>, Component::ComponentType::maxComponentType + 1> m_componentTextures;
    std::array<std::vector<std::string>, Component::ComponentType::maxComponentType + 1> m_componentTexturesLockedInPlace;

    std::vector<std::string> m_borderTextures;
};

#endif // AMAZING_LABYRINTH_ROTATABLE_PASSAGE_HPP
