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

#include "../basic/level.hpp"
#include "../openAreaMaze/level.hpp"
#include "../../levelTracker/types.hpp"

#include "loadData.hpp"

namespace avoidVortexMaze {
    class Level : public openAreaMaze::Level {
    protected:
        std::string m_avoidObjTexture;

        bool checkFinishCondition(float timeDiff) override;

        std::vector<glm::vec3> m_avoidObjectLocations;
    public:
        static char constexpr const *m_name = "avoidVortexMaze";

        std::vector<uint8_t> saveData(levelTracker::GameSaveData const &gsd, char const *saveLevelDataKey) override;

        char const *name() override { return m_name; }

        // lcd should never be null, sd may be null.
        Level(levelDrawer::Adaptor inLevelDrawer,
                std::shared_ptr<LevelConfigData> const &lcd,
                std::shared_ptr<LevelSaveData> const &sd,
                float maxZ)
                : openAreaMaze::Level(std::move(inLevelDrawer), lcd, sd, maxZ),
                m_avoidObjTexture(lcd->avoidTexture)
        {
            if (sd) {
                m_mazeBoard.setStart(sd->startRowCol.x, sd->startRowCol.y);
                for (auto avoidObjLocation : sd->avoidObjLocations) {
                    m_avoidObjectLocations.emplace_back(avoidObjLocation.x, avoidObjLocation.y,
                                                        getBallZPosition());
                }
            } else {
                generateAvoidModelMatrices(lcd->numberAvoidObjects);
            }

            auto objIndex = m_levelDrawer.addObject(std::make_shared<levelDrawer::ModelDescriptionQuad>(),
                    std::make_shared<levelDrawer::TextureDescriptionPath>(m_avoidObjTexture));

            for (auto const &item : m_avoidObjectLocations) {
                m_levelDrawer.addModelMatrixForObject(objIndex,
                        glm::translate(glm::mat4(1.0f), item) * scaleBall);
            }
        }
    private:
        void generateAvoidModelMatrices(uint32_t numberAvoidObjects);
    };

} // namespace avoidVortexMaze

#endif // AMAZING_LABYRINTH_AVOID_VORTEX_MAZE_LEVEL_HPP
