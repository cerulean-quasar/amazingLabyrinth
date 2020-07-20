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

#ifndef AMAZING_LABYRINTH_AVOID_VORTEX_OPEN_AREA_LEVEL_HPP
#define AMAZING_LABYRINTH_AVOID_VORTEX_OPEN_AREA_LEVEL_HPP

#include <cstdint>
#include <vector>
#include <list>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../../random.hpp"
#include "../basic/level.hpp"
#include "loadData.hpp"

namespace avoidVortexOpenArea {
    class Level : public basic::Level {
    private:
        static uint32_t constexpr numberOfVortexes = 8;

        float const maxX;
        float const maxY;
        Random random;
        std::chrono::high_resolution_clock::time_point prevTime;

        glm::vec3 holePosition;

        // the start position of the ball.  The ball goes back here if it touches a vortex.
        glm::vec3 startPosition;

        // the position of the quad being displayed to designate the start position of the ball.
        glm::vec3 startPositionQuad;

        // if the ball touches these vortexes, it goes back to startPosition.
        std::vector<glm::vec3> vortexPositions;

        /* The object index for drawing the ball - needed to update the ball's location. */
        size_t m_objIndexBall;
        size_t m_objDataIndexBall;

        glm::mat4 modelMatrixHole;
        glm::mat4 modelMatrixBall;
        glm::mat4 modelMatrixStartVortex;
        std::vector<glm::mat4> modelMatrixVortexes;

        // the scale matrix for the ball.
        glm::mat4 scale;

        bool ballProximity(glm::vec3 const &objPosition);

        void preGenerate();

        void generate();

        void postGenerate();

        void generateModelMatrices();

    public:
        static char constexpr const *m_name = "avoidVortexOpenArea";

        bool updateData() override;

        bool updateDrawObjects() override;

        void start() override {
            prevTime = std::chrono::high_resolution_clock::now();
        }

        void getLevelFinisherCenter(float &x, float &y, float &z) override {
            x = holePosition.x;
            y = holePosition.y;
            z = holePosition.z;
        }

        char const *name() override { return m_name; }

        std::vector<uint8_t> saveData(levelTracker::GameSaveData const &gsd,
                                      char const *saveLevelDataKey) override;

        Level(std::shared_ptr<GameRequester> inGameRequester,
                         std::shared_ptr<LevelConfigData> const &lcd,
                         std::shared_ptr<LevelSaveData> const &sd,
                         levelDrawer::Adaptor inLevelDrawer,
                         float floorZ)
                : basic::Level(std::move(inGameRequester), lcd, std::move(inLevelDrawer), floorZ, true),
                  maxX(m_width / 2),
                  maxY(m_height / 2),
                  prevTime(std::chrono::high_resolution_clock::now())
        {
            m_levelDrawer.setClearColor(glm::vec4{0.0f, 0.0f, 0.0f, 1.0f});
            preGenerate();
            if (sd == nullptr) {
                generate();
            } else {
                m_ball.position.x = sd->ball.x;
                m_ball.position.y = sd->ball.y;
                holePosition.x = sd->hole.x;
                holePosition.y = sd->hole.y;
                startPosition.x = sd->startPos.x;
                startPosition.y = sd->startPos.y;
                vortexPositions.reserve(sd->vortexes.size());
                for (auto const &vortex : sd->vortexes) {
                    vortexPositions.emplace_back(vortex.x, vortex.y, m_mazeFloorZ);
                }
            }
            postGenerate();
            generateModelMatrices();

            auto objIndexHole = m_levelDrawer.addObject(
                    std::make_shared<levelDrawer::ModelDescriptionQuad>(),
                    std::make_shared<levelDrawer::TextureDescriptionPath>(lcd->holeTexture));

            m_levelDrawer.addModelMatrixForObject(objIndexHole, modelMatrixHole);

            auto objIndexVortex = m_levelDrawer.addObject(
                    std::make_shared<levelDrawer::ModelDescriptionQuad>(),
                    std::make_shared<levelDrawer::TextureDescriptionPath>(lcd->vortexTexture));

            for (auto const &modelMatrixVortex : modelMatrixVortexes) {
                m_levelDrawer.addModelMatrixForObject(objIndexVortex, modelMatrixVortex);
            }

            auto objIndexStartVortex = m_levelDrawer.addObject(
                    std::make_shared<levelDrawer::ModelDescriptionQuad>(),
                    std::make_shared<levelDrawer::TextureDescriptionPath>(lcd->startVortexTexture));

            m_levelDrawer.addModelMatrixForObject(objIndexStartVortex, modelMatrixStartVortex);

            m_objIndexBall = m_levelDrawer.addObject(
                    std::make_shared<levelDrawer::ModelDescriptionPath>(m_ballModel),
                    std::make_shared<levelDrawer::TextureDescriptionPath>(m_ballTexture));

            m_objDataIndexBall = m_levelDrawer.addModelMatrixForObject(m_objIndexBall, modelMatrixBall);
        }

        ~Level() override = default;
    };

} // namespace avoidVortexOpenArea

#endif // AMAZING_LABYRINTH_AVOID_VORTEX_OPEN_AREA_LEVEL_HPP