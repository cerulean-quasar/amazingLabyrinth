/**
 * Copyright 2024 Cerulean Quasar. All Rights Reserved.
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
#ifndef AMAZING_LABYRINTH_OPEN_AREA_LEVEL_HPP
#define AMAZING_LABYRINTH_OPEN_AREA_LEVEL_HPP

#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../basic/level.hpp"
#include "loadData.hpp"
#include "../../random.hpp"

namespace openArea {
    class Level : public basic::Level {
    private:
        Random random;
        std::chrono::high_resolution_clock::time_point prevTime;

        glm::vec3 holePosition;

        /* The object index for drawing the ball - needed to update the ball's location. */
        levelDrawer::DrawObjReference m_objRefBall;
        levelDrawer::DrawObjDataReference m_objDataRefBall;

        glm::mat4 modelMatrixHole;
        glm::mat4 modelMatrixBall;

        glm::mat4 scale;

        void generate(glm::vec2 ballPos, glm::vec2 holePos) {
            scale = ballScaleMatrix();

            m_ball.prevPosition = {-10.0f, 0.0f, m_mazeFloorZ + ballRadius()};
            m_ball.position = {ballPos.x, ballPos.y, m_mazeFloorZ + ballRadius()};

            holePosition = {holePos.x, holePos.y, m_mazeFloorZ};
        }

        void generate() {
            float smallestDistance = m_diagonal / 5.0f;
            glm::vec2 holePos;
            glm::vec2 ballPos;
            do {
                float r = ballRadius();
                holePos.x = random.getFloat(-m_width / 2 + r, m_width / 2 - r);
                holePos.y = random.getFloat(-m_height / 2 + r, m_height / 2 - r);

                ballPos.x = random.getFloat(-m_width / 2 + r, m_width / 2 - r);
                ballPos.y = random.getFloat(-m_height / 2 + r, m_height / 2 - r);
            } while (glm::length(ballPos - holePos) < smallestDistance);
            generate(ballPos, holePos);
        }

        void generateModelMatrices();

    public:
        static char constexpr const *m_name = "openArea";
        struct Request :public basic::Level::Request {
            Request(levelDrawer::Adaptor levelDrawer, bool shadowsEnabled)
                    : basic::Level::Request(std::move(levelDrawer), shadowsEnabled)
            {}
        };
        Level(
                levelDrawer::Adaptor inLevelDrawer,
                std::shared_ptr<basic::LevelConfigData> const &lcd,
                std::shared_ptr <LevelSaveData> const &levelRestoreData,
                float maxZ,
                Request &request)
                : basic::Level(std::move(inLevelDrawer), lcd, maxZ, true, request),
                  prevTime(std::chrono::high_resolution_clock::now())
        {
            if (levelRestoreData == nullptr) {
                generate();
            } else {
                generate({levelRestoreData->ball.x, levelRestoreData->ball.y},
                         {levelRestoreData->hole.x, levelRestoreData->hole.y});
            }

            generateModelMatrices();

            m_levelDrawer.setClearColor(glm::vec4{0.0f, 0.0f, 0.0f, 1.0f});

            glm::vec3 ballColor{1.0f, 1.0f, 1.0f};

            // the ball
            auto const &ballModelData = findModelsAndTextures(ModelNameBall);
            m_objRefBall = m_levelDrawer.addObject(
                    ballModelData.models[0],
                    getFirstTexture(ballModelData));

            m_objDataRefBall = m_levelDrawer.addModelMatrixForObject(m_objRefBall, modelMatrixBall);

            // the hole
            auto const &holeModelData = findModelsAndTextures(ModelNameHole);
            auto objIndexHole = m_levelDrawer.addObject(
                    holeModelData.models[0],
                    getFirstTexture(holeModelData));

            m_levelDrawer.addModelMatrixForObject(objIndexHole, modelMatrixHole);
        }

        bool updateData() override;

        bool updateDrawObjects() override;

        void start() override {
            prevTime = std::chrono::high_resolution_clock::now();
        }

        void getLevelFinisherCenter(float &x, float &y, float &z) override {
            x = holePosition.x;
            y = holePosition.y;
            z = m_mazeFloorZ;
        }

        char const *name() override { return m_name; }

        std::vector<uint8_t> saveData(levelTracker::GameSaveData const &gsd,
                                      char const *saveLevelDataKey) override;

        ~Level() override = default;
    };
} // namespace openArea
#endif // AMAZING_LABYRINTH_OPEN_AREA_LEVEL_HPP
