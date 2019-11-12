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
#ifndef AMAZING_LABYRINTH_MAZE_COLLECT_HPP
#define AMAZING_LABYRINTH_MAZE_COLLECT_HPP

#include "mazeOpenArea.hpp"

struct MazeCollectSaveData : public MazeSaveData {
    std::vector<Point<float>> collectionObjLocations;
    std::vector<bool> itemsCollected;
    std::vector<Point<uint32_t>> previousCells;

    // Intentionally slices other to pass part into the MazeSaveData constructor.
    MazeCollectSaveData(MazeCollectSaveData &&other) noexcept
        : MazeSaveData(std::move(other)),
        collectionObjLocations{other.collectionObjLocations},
        itemsCollected{other.itemsCollected},
        previousCells{other.previousCells}
    {
    }

    MazeCollectSaveData()
            : MazeSaveData{},
              collectionObjLocations{},
              itemsCollected{},
              previousCells{}
    {
    }

    MazeCollectSaveData(
        uint32_t nbrRows_,
        uint32_t ballRow_,
        uint32_t ballCol_,
        Point<float> &&ballPos_,
        uint32_t rowEnd_,
        uint32_t colEnd_,
        std::vector<uint32_t> &&wallTextures_,
        std::vector<uint8_t> &&mazeWallsVector_,
        std::vector<Point<float>> &&collectionObjLocations_,
        std::vector<bool> &&itemsCollected_,
        std::vector<Point<uint32_t>> &&previousCells_)
        : MazeSaveData{nbrRows_, ballRow_, ballCol_, std::move(ballPos_), rowEnd_, colEnd_,
                       std::move(wallTextures_), std::move(mazeWallsVector_)},
          collectionObjLocations{std::move(collectionObjLocations_)},
          itemsCollected{std::move(itemsCollected_)},
          previousCells{std::move(previousCells_)}
    {
    }
};

class MazeCollect : public MazeOpenArea {
protected:
    static constexpr uint32_t nbrItemsToCollect = 5;
    bool checkFinishCondition(float timeDiff) override;
    std::vector<std::pair<bool, glm::vec3>> m_collectionObjectLocations;
    std::deque<std::pair<uint32_t, uint32_t>> m_prevCells;
public:
    MazeCollect(std::shared_ptr<GameRequester> inGameRequester,
                std::shared_ptr<MazeCollectSaveData> sd,
                float inWidth, float inHeight, float maxZ)
            :MazeOpenArea(std::move(inGameRequester), sd, inWidth, inHeight, maxZ)
    {
        for (size_t i = 0; i < sd->collectionObjLocations.size(); i++) {
            m_collectionObjectLocations.emplace_back(sd->itemsCollected[i],
                    glm::vec3{sd->collectionObjLocations[i].x, sd->collectionObjLocations[i].y, getBallZPosition()});
        }

        for (auto const &prevCell : sd->previousCells) {
            m_prevCells.emplace_back(prevCell.x, prevCell.y);
        }
    }

    MazeCollect(std::shared_ptr<GameRequester> inGameRequester, Maze::CreateParameters const &parameters,
            float inWidth, float inHeight, float maxZ)
            :MazeOpenArea(std::move(inGameRequester), parameters, inWidth, inHeight, maxZ)
    {
        generateCollectBallModelMatrices();
    }

    bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged) override;
    SaveLevelDataFcn getSaveLevelDataFcn() override;
private:
    void generateCollectBallModelMatrices();
};

#endif /* AMAZING_LABYRINTH_MAZE_COLLECT_HPP */
