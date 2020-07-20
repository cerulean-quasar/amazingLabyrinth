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
#ifndef AMAZING_LABYRINTH_MAZE_COLLECT_HPP
#define AMAZING_LABYRINTH_MAZE_COLLECT_HPP

#include <deque>
#include "../openAreaMaze/level.hpp"
#include "loadData.hpp"

namespace collectMaze {
    class Level : public openAreaMaze::Level {
    protected:
        float const collectBallScaleFactor;
        bool checkFinishCondition(float timeDiff) override;

        uint32_t m_numberCollectObjects;
        std::vector<std::pair<bool, glm::vec3>> m_collectionObjectLocations;
        std::deque<std::pair<uint32_t, uint32_t>> m_prevCells;
        std::string m_collectModel;
        std::string m_collectTexture;
        bool m_collectObjsSameAsBall;

        size_t m_objIndexCollect;
        std::vector<size_t> m_objDataIndicesCollect;
    public:
        static char constexpr const *m_name = "collectMaze";

        Level(std::shared_ptr<GameRequester> inGameRequester,
                    std::shared_ptr<LevelConfigData> const &lcd,
                    std::shared_ptr<LevelSaveData> const &sd,
                    levelDrawer::Adaptor inLevelDrawer,
                    float floorZ)
                : openAreaMaze::Level(std::move(inGameRequester), lcd, sd, std::move(inLevelDrawer), floorZ),
                  m_numberCollectObjects{lcd->numberCollectObjects},
                  collectBallScaleFactor{2.0f * m_scaleBall / 3.0f},
                  m_collectModel{lcd->collectModel},
                  m_collectTexture{lcd->collectTexture},
                  m_collectObjsSameAsBall{m_collectModel == m_ballModel && m_collectTexture == m_ballTexture}
        {
            if (sd) {
                for (size_t i = 0; i < sd->collectionObjLocations.size(); i++) {
                    m_collectionObjectLocations.emplace_back(sd->itemsCollected[i],
                                                             glm::vec3{
                                                                     sd->collectionObjLocations[i].x,
                                                                     sd->collectionObjLocations[i].y,
                                                                     getBallZPosition()});
                }

                for (auto const &prevCell : sd->previousCells) {
                    m_prevCells.emplace_back(prevCell.x, prevCell.y);
                }
            } else {
                generateCollectBallModelMatrices();
            }

            if (m_collectObjsSameAsBall) {
                m_objIndexCollect = m_objIndexBall;
            } else {
                m_objIndexCollect = m_levelDrawer.addObject(
                        std::make_shared<levelDrawer::ModelDescriptionPath>(m_collectModel),
                        std::make_shared<levelDrawer::TextureDescriptionPath>(m_collectTexture));
            }

            glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f),
                                               glm::vec3{collectBallScaleFactor, collectBallScaleFactor,
                                                         collectBallScaleFactor});

            for (auto const &item : m_collectionObjectLocations) {
                auto index = m_levelDrawer.addModelMatrixForObject(
                        m_objIndexCollect,
                        glm::translate(glm::mat4(1.0f), item.second) *
                                                    glm::mat4_cast(m_ball.totalRotated) *
                                                    scaleMatrix);
                m_objDataIndicesCollect.push_back(index);
            }
        }

        bool updateDrawObjects() override;

        char const *name() override { return m_name; }

        std::vector<uint8_t> saveData(levelTracker::GameSaveData const &gsd,
                char const *saveLevelDataKey) override;

    private:
        void generateCollectBallModelMatrices();
    };
} // namespace collectMaze
#endif /* AMAZING_LABYRINTH_MAZE_COLLECT_HPP */
