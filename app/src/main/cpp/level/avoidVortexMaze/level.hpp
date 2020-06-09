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
#ifndef AMAZING_LABYRINTH_AVOID_VORTEX_MAZE_LEVEL_HPP
#define AMAZING_LABYRINTH_AVOID_VORTEX_MAZE_LEVEL_HPP

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "saveData.hpp"
#include "basic/level.hpp"
#include "../openAreaMaze/level.hpp"

#include "loadData.hpp"

namespace avoidVortexMaze {
    class Level : public openAreaMaze::Level {
    protected:
        static constexpr uint32_t nbrItemsToAvoid = 5;
        std::string m_avoidObjTexture;

        bool checkFinishCondition(float timeDiff) override;

        std::vector<glm::vec3> m_avoidObjectLocations;
    public:
        Level(std::shared_ptr<GameRequester> inGameRequester,
                  std::shared_ptr<LevelSaveData> sd,
                  float inWidth, float inHeight, float maxZ)
                : openAreaMaze::Level(std::move(inGameRequester), sd, inWidth, inHeight, maxZ) {
            m_mazeBoard.setStart(sd->startRowCol.x, sd->startRowCol.y);
            for (auto avoidObjLocation : sd->avoidObjLocations) {
                m_avoidObjectLocations.emplace_back(avoidObjLocation.x, avoidObjLocation.y,
                                                    getBallZPosition());
            }
        }

        Level(std::shared_ptr<GameRequester> inGameRequester,
                  generatedMaze::Level::CreateParameters const &parameters,
                  float inWidth, float inHeight, float maxZ)
                : openAreaMaze::Level(std::move(inGameRequester), parameters, inWidth, inHeight, maxZ) {
            generateAvoidModelMatrices();
        }

        bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) override;

        SaveLevelDataFcn getSaveLevelDataFcn() override;

        void initSetAvoidObjTexture(std::string avoidObjTexture) {
            m_avoidObjTexture = std::move(avoidObjTexture);
        }

    private:
        void generateAvoidModelMatrices();
    };

} // namespace avoidVortexMaze

#endif // AMAZING_LABYRINTH_AVOID_VORTEX_MAZE_LEVEL_HPP
