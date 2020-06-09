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
#ifndef AMAZING_LABYRINTH_OPEN_AREA_LEVEL_HPP
#define AMAZING_LABYRINTH_OPEN_AREA_LEVEL_HPP

#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../levelFinish.hpp"
#include "../level.hpp"

int constexpr openArealLevelVersion = 1;
struct OpenAreaLevelSaveData : public LevelSaveData {
    Point<float> ball;
    Point<float> hole;
    OpenAreaLevelSaveData(OpenAreaLevelSaveData &&other) noexcept
            : LevelSaveData{openArealLevelVersion},
              ball{other.ball},
              hole{other.hole} {
    }

    OpenAreaLevelSaveData()
            : LevelSaveData{openArealLevelVersion},
              ball{0.0f, 0.0f},
              hole{0.0f, 0.0f}
    {
    }

    OpenAreaLevelSaveData(Point<float> &&ball_, Point<float> &&hole_)
            : LevelSaveData{openArealLevelVersion},
              ball{ball_},
              hole{hole_}
    {
    }
};

class OpenAreaLevel : public Level {
private:
    std::string holeTexture;
    std::string ballTexture;

    Random random;
    std::chrono::high_resolution_clock::time_point prevTime;

    glm::vec3 holePosition;

    /* vertex and index data for drawing the ball. */
    std::vector<Vertex> ballVertices;
    std::vector<uint32_t> ballIndices;

    /* vertex and index data for drawing the hole. */
    std::vector<Vertex> holeVertices;
    std::vector<uint32_t> holeIndices;

    glm::mat4 modelMatrixHole;
    glm::mat4 modelMatrixBall;

    glm::mat4 scale;

    void loadModels();
    void generate(glm::vec2 ballPos, glm::vec2 holePos) {
        scale = ballScaleMatrix();

        m_ball.prevPosition = {-10.0f, 0.0f, m_mazeFloorZ + ballRadius() };
        m_ball.position = { ballPos.x, ballPos.y, m_mazeFloorZ + ballRadius() };

        holePosition = {holePos.x, holePos.y, m_mazeFloorZ};
    }

    void generate() {
        float smallestDistance = m_diagonal/ 5.0f;
        glm::vec2 holePos;
        glm::vec2 ballPos;
        do {
            float r = ballRadius();
            holePos.x = random.getFloat(-m_width/2 + r, m_width/2 - r);
            holePos.y = random.getFloat(-m_height/2 + r, m_height/2 - r);

            ballPos.x = random.getFloat(-m_width/2 + r, m_width/2 - r);
            ballPos.y = random.getFloat(-m_height/2 + r, m_height/2 - r);
        } while (glm::length(ballPos - holePos) < smallestDistance);
        generate(ballPos, holePos);
    }
    void generateModelMatrices();

public:
    OpenAreaLevel(
            std::shared_ptr<GameRequester> inGameRequester,
            std::shared_ptr<OpenAreaLevelSaveData> const &levelRestoreData,
            float width,
            float height,
            float maxZ)
            : Level(std::move(inGameRequester), width, height, maxZ, true, 1.0f/40.0f),
            prevTime(std::chrono::high_resolution_clock::now())
    {
        loadModels();
        if(levelRestoreData == nullptr) {
            generate();
        } else {
            generate({levelRestoreData->ball.x, levelRestoreData->ball.y},
                     {levelRestoreData->hole.x, levelRestoreData->hole.y});
        }
        generateModelMatrices();
    }
    glm::vec4 getBackgroundColor() override { return {0.0f, 0.0f, 0.0f, 1.0f}; }
    bool updateData() override;
    bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) override;
    bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged) override;
    void start() override {
        prevTime = std::chrono::high_resolution_clock::now();
    }

    void initSetHoleTexture(std::string const &texture) { holeTexture = texture; }
    void initSetBallTexture(std::string const &texture) { ballTexture = texture; }

    void getLevelFinisherCenter(float &x, float &y) override {
        x = holePosition.x;
        y = holePosition.y;
    }

    SaveLevelDataFcn getSaveLevelDataFcn() override;

    ~OpenAreaLevel() override = default;
};
#endif /* AMAZING_LABYRINTH_OPEN_AREA_LEVEL_HPP */
