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

#ifndef AMAZING_LABYRINTH_MOVING_SAFE_AREAS_LEVEL_HPP
#define AMAZING_LABYRINTH_MOVING_SAFE_AREAS_LEVEL_HPP

#include <cstdint>
#include <vector>
#include <list>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../../graphics.hpp"
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
        float const spaceBetweenQuadsX;
        float const maxX;
        float const maxY;
        float timeDiffSinceLastMove;

        std::string m_startQuadTexture;
        std::string m_endQuadTexture;
        std::vector<std::string> m_middleQuadTextures;

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

        /* vertex and index data for drawing the ball. */
        std::vector<Vertex> m_ballVertices;
        std::vector<uint32_t> m_ballIndices;

        /* vertex and index data for drawing the hole. */
        std::vector<Vertex> m_quadVertices;
        std::vector<uint32_t> m_quadIndices;

        bool ballOnQuad(glm::vec3 const &centerPos, float xSize);

        float minQuadMovingSpeed() { return m_width / 40.0f; }

        float maxQuadMovingSpeed() { return m_width / 10.0f; }

        void loadModels();

        void preGenerate();

        void generate();

    public:
        static char constexpr const *m_name = "movingSafeAreas";

        glm::vec4 getBackgroundColor() override { return glm::vec4(0.2, 0.2, 1.0, 1.0); }

        bool updateData() override;

        bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) override;

        bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures,
                                      bool &texturesChanged) override;

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

        Level(
            std::shared_ptr<GameRequester> inGameRequester,
            std::shared_ptr<LevelConfigData> const &lcd,
            std::shared_ptr<LevelSaveData> const &saveData,
            glm::mat4 const &proj,
            glm::mat4 const &view,
            float maxZ)
            : basic::Level(std::move(inGameRequester), lcd, proj, view, maxZ, true),
              spaceBetweenQuadsX{m_width / 10.0f},
              maxX(m_width / 2),
              maxY(m_height / 2),
              m_prevTime(std::chrono::high_resolution_clock::now()),
              timeDiffSinceLastMove{0.0f},
              m_startQuadTexture{lcd->startQuadTexture},
              m_endQuadTexture{lcd->endQuadTexture},
              m_middleQuadTextures{lcd->middleQuadTextures}
        {
            loadModels();
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
        }

        ~Level() override = default;
    };
} // namespace movingSafeAreas

#endif /* AMAZING_LABYRINTH_MOVING_QUADS_LEVEL_HPP */
