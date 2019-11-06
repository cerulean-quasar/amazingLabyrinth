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

class MazeCollect : public MazeOpenArea {
protected:
    static constexpr uint32_t nbrItemsToCollect = 5;
    bool checkFinishCondition(float timeDiff) override;
    std::vector<std::pair<bool, glm::vec3>> m_collectionObjectLocations;
    std::deque<std::pair<uint32_t, uint32_t>> m_prevCells;
public:
    MazeCollect(std::shared_ptr<GameRequester> inGameRequester,
                float inWidth, float inHeight, float maxZ)
            :MazeOpenArea(std::move(inGameRequester), inWidth, inHeight, maxZ)
    {
        generateCollectBallModelMatrices();
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
