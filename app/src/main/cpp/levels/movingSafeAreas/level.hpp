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

#ifndef AMAZING_LABYRINTH_MOVING_SAFE_AREAS_LEVEL_HPP
#define AMAZING_LABYRINTH_MOVING_SAFE_AREAS_LEVEL_HPP

#include <cstdint>
#include <vector>
#include <list>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../../random.hpp"
#include "../basic/level.hpp"

#include "loadData.hpp"

namespace movingSafeAreas {
    class Level : public basic::Level {
    private:
        static uint32_t constexpr numberOfMidQuadRows = 4;
        static uint32_t constexpr minQuadsInRow = 1;
        static uint32_t constexpr maxQuadsInRow = 3;
        static constexpr float m_quadOriginalSize = 2.0f;
        float const maxX;
        float const maxY;
        float timeDiffSinceLastMove;

        std::chrono::high_resolution_clock::time_point m_prevTime;

        glm::vec3 m_endQuadPosition;
        glm::vec3 m_startQuadPosition;
        float m_quadScaleY;

        // the start position of the ball.  The ball goes back here if it falls off the quads.
        glm::vec3 m_startPosition;

        // moving quads.  The goal is to keep the ball on these moving quads.
        struct MovingQuadRow {
            std::vector<glm::vec3> positions;
            float speed;
            glm::vec3 scale;

            MovingQuadRow()
                    : positions{},
                      speed{0.0f},
                      scale{0.0f, 0.0f, 0.0f} {}

            MovingQuadRow(std::vector<glm::vec3> const &positions_, float speed_,
                          glm::vec3 const &scale_) {
                positions = positions_;
                speed = speed_;
                scale = scale_;
            }

            MovingQuadRow(std::vector<glm::vec3> &&positions_, float speed_, glm::vec3 &&scale_) {
                positions = std::move(positions_);
                speed = speed_;
                scale = std::move(scale_);
            }
        };

        std::vector<MovingQuadRow> m_movingQuads;

        // indices to identify the ball for moving
        levelDrawer::DrawObjReference m_objRefBall;
        levelDrawer::DrawObjDataReference m_objDataRefBall;

        // indices to identify the moving quads
        std::vector<levelDrawer::DrawObjReference> m_objRefQuad;
        std::vector<std::vector<levelDrawer::DrawObjDataReference>> m_objDataRefQuad;

        bool ballOnQuad(glm::vec3 const &centerPos, float xSize);

        float minQuadMovingSpeed() { return m_width / 40.0f; }

        float maxQuadMovingSpeed() { return m_width / 10.0f; }

        void preGenerate();

        void generate();

    protected:
        static char const constexpr *ModelNameStartingArea = "StartingArea";
        static char const constexpr *ModelNameMiddleArea = "MiddleArea";
        static char const constexpr *ModelNameEndingArea = "EndingArea";

    public:
        static char constexpr const *m_name = "movingSafeAreas";

        struct Request : public basic::Level::Request {
            Request(levelDrawer::Adaptor levelDrawer, bool shadowsEnabled)
                    : basic::Level::Request(std::move(levelDrawer), shadowsEnabled)
            {}
        };

        bool updateData() override;

        bool updateDrawObjects() override;

        void start() override {
            m_prevTime = std::chrono::high_resolution_clock::now();
        }

        void getLevelFinisherCenter(float &x, float &y, float &z) override {
            x = m_endQuadPosition.x;
            y = m_endQuadPosition.y;
            z = m_endQuadPosition.z;
        }

        char const *name() override { return m_name; }

        std::vector<uint8_t> saveData(levelTracker::GameSaveData const &gsd,
                                      char const *saveLevelDataKey) override;

        Level(levelDrawer::Adaptor inLevelDrawer,
                std::shared_ptr<basic::LevelConfigData> const &lcd,
                std::shared_ptr<LevelSaveData> const &saveData,
                float maxZ,
                Request &request)
            : basic::Level(std::move(inLevelDrawer), lcd, maxZ, true, request),
              maxX(m_width / 2),
              maxY(m_height / 2),
              m_prevTime(std::chrono::high_resolution_clock::now()),
              timeDiffSinceLastMove{0.0f}
        {
            m_levelDrawer.setClearColor(glm::vec4(0.2, 0.2, 1.0, 1.0));
            preGenerate();
            if (saveData == nullptr) {
                generate();
            } else {
                m_ball.position.x = saveData->ball.x;
                m_ball.position.y = saveData->ball.y;
                m_movingQuads.reserve(saveData->quadRows.size());
                for (auto const &quadRow : saveData->quadRows) {
                    std::vector<glm::vec3> positions;
                    positions.reserve(quadRow.positions.size());
                    for (auto const &position : quadRow.positions) {
                        positions.emplace_back(position.x, position.y, m_mazeFloorZ);
                    }
                    m_movingQuads.emplace_back(std::move(positions), quadRow.speed,
                                               glm::vec3{quadRow.scale.x, quadRow.scale.y, 1.0f});
                }
            }

            // the starting area
            auto const &startingModelData = findModelsAndTextures(ModelNameStartingArea);
            auto objIndex = m_levelDrawer.addObject(
                    startingModelData.models[0],
                    getFirstTexture(startingModelData));

            glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3{2 * maxX, m_quadScaleY, 1.0f});
            m_levelDrawer.addModelMatrixForObject(
                    objIndex,
                    glm::translate(glm::mat4(1.0f), m_startQuadPosition) * scale);

            // the ending area
            auto const &endingModelData = findModelsAndTextures(ModelNameEndingArea);
            objIndex = m_levelDrawer.addObject(
                    endingModelData.models[0],
                    getFirstTexture(endingModelData));

            m_levelDrawer.addModelMatrixForObject(
                    objIndex,
                    glm::translate(glm::mat4(1.0f), m_endQuadPosition) * scale);

            // next we have the moving quads
            auto const &middleAreaModelData = findModelsAndTextures(ModelNameMiddleArea);
            if (middleAreaModelData.textures.empty()) {
                throw std::runtime_error(std::string("Expected at least one texture for: ") + ModelNameMiddleArea + " got zero.");
            }
            for (auto const &texture : middleAreaModelData.textures) {
                objIndex = m_levelDrawer.addObject(
                        middleAreaModelData.models[0],
                        texture);
                m_objRefQuad.push_back(objIndex);
            }

            size_t nbrTextures = middleAreaModelData.textures.size();
            m_objDataRefQuad.resize(nbrTextures);

            size_t nbrRowsForTexture = m_movingQuads.size() / nbrTextures;
            size_t leftover = m_movingQuads.size() % nbrTextures;
            size_t i = 0;
            for (auto const &movingQuadRow : m_movingQuads) {
                size_t textureNumber;
                if (nbrRowsForTexture == 0) {
                    textureNumber = i;
                } else if (i <= leftover * nbrRowsForTexture) {
                    textureNumber = i / (nbrRowsForTexture + 1);
                } else {
                    textureNumber = i / nbrRowsForTexture;
                }

                for (auto const &movingQuadPos : movingQuadRow.positions) {
                    auto objDataIndex = m_levelDrawer.addModelMatrixForObject(
                            m_objRefQuad[textureNumber],
                            glm::translate(glm::mat4(1.0f), movingQuadPos) *
                            glm::scale(glm::mat4(1.0f), movingQuadRow.scale));
                    m_objDataRefQuad[textureNumber].push_back(objDataIndex);
                }
                i++;
            }

            // the ball
            auto const &ballModelData = findModelsAndTextures(ModelNameBall);
            m_objRefBall = m_levelDrawer.addObject(
                    ballModelData.models[0],
                    getFirstTexture(ballModelData));

            m_objDataRefBall = m_levelDrawer.addModelMatrixForObject(
                    m_objRefBall,
                    glm::translate(glm::mat4(1.0f), m_ball.position) *
                    glm::mat4_cast(m_ball.totalRotated) *
                    ballScaleMatrix());
        }

        ~Level() override = default;
    };
} // namespace movingSafeAreas

#endif /* AMAZING_LABYRINTH_MOVING_SAFE_AREAS_LEVEL_HPP */
