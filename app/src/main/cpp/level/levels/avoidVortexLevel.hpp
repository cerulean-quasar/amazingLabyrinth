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

#ifndef AMAZING_LABYRINTH_AVOID_VORTEX_LEVEL_HPP
#define AMAZING_LABYRINTH_AVOID_VORTEX_LEVEL_HPP

#include <cstdint>
#include <vector>
#include <list>
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "../../graphics.hpp"
#include "../../random.hpp"
#include "../level.hpp"
#include "../../saveData.hpp"

int constexpr avoidVortexlLevelVersion = 1;
struct AvoidVortexLevelSaveData : public LevelSaveData {
    Point<float> ball;
    Point<float> hole;
    Point<float> startPos;
    std::vector<Point<float>> vortexes;
    AvoidVortexLevelSaveData(AvoidVortexLevelSaveData &&other) noexcept
            : LevelSaveData{avoidVortexlLevelVersion},
              ball{other.ball},
              hole{other.hole},
              startPos{other.startPos},
              vortexes{std::move(other.vortexes)} {
    }

    AvoidVortexLevelSaveData()
            : LevelSaveData{avoidVortexlLevelVersion},
              ball{0.0f, 0.0f},
              hole{0.0f, 0.0f},
              startPos{0.0f, 0.0f},
              vortexes{}
    {
    }

    AvoidVortexLevelSaveData(
            Point<float> &&ball_,
            Point<float> &&hole_,
            Point<float> &&startPos_,
            std::vector<Point<float>> &&vortexes_)
            : LevelSaveData{avoidVortexlLevelVersion},
              ball{ball_},
              hole{hole_},
              startPos{startPos_},
              vortexes{std::move(vortexes_)}
    {
    }
};

class AvoidVortexLevel : public Level {
private:
    static constexpr uint32_t numberOfVortexes = 8;
    static constexpr float scaleFactor = 1.0f/16.0f;

    std::string holeTexture;
    std::string ballTexture;
    std::string vortexTexture;
    std::string startVortexTexture;

    static constexpr float viscosity = 0.005f;
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

    /* vertex and index data for drawing the ball. */
    std::vector<Vertex> ballVertices;
    std::vector<uint32_t> ballIndices;

    /* vertex and index data for drawing the hole. */
    std::vector<Vertex> quadVertices;
    std::vector<uint32_t> quadIndices;

    glm::mat4 modelMatrixHole;
    glm::mat4 modelMatrixBall;
    glm::mat4 modelMatrixStartVortex;
    std::vector<glm::mat4> modelMatrixVortexes;

    // the scale matrix for the ball.
    glm::mat4 scale;

    // data on where the ball is, how fast it is moving, etc.
    struct {
        glm::vec3 prevPosition;
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec3 acceleration;
        glm::quat totalRotated;
    } ball;

    bool ballProximity(glm::vec3 const &objPosition);
    void loadModels();
    void preGenerate();
    void generate();
    void postGenerate();
    void generateModelMatrices();
public:
    glm::vec4 getBackgroundColor() override { return glm::vec4(0.0, 0.0, 0.0, 1.0); }
    void updateAcceleration(float x, float y, float z) override { ball.acceleration = {-x, -y, 0.0f}; }
    bool updateData() override;
    bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) override;
    bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged) override;
    void start() override {
        prevTime = std::chrono::high_resolution_clock::now();
    }

    void getLevelFinisherCenter(float &x, float &y) override {
        x = holePosition.x;
        y = holePosition.y;
    }

    void initSetHoleTexture(std::string const &texture) { holeTexture = texture; }
    void initSetVortexTexture(std::string const &texture) { vortexTexture = texture; }
    void initSetStartVortexTexture(std::string const &texture) { startVortexTexture = texture; }
    void initSetBallTexture(std::string const &texture) { ballTexture = texture; }
    SaveLevelDataFcn getSaveLevelDataFcn() override;

    AvoidVortexLevel(std::shared_ptr<GameRequester> inGameRequester, float width, float height, float maxZ)
            : Level(std::move(inGameRequester), width, height, maxZ),
              maxX(m_width/2),
              maxY(m_height/2),
              prevTime(std::chrono::high_resolution_clock::now()) {
        loadModels();
        preGenerate();
        generate();
        postGenerate();
        generateModelMatrices();
    }

    AvoidVortexLevel(std::shared_ptr<GameRequester> inGameRequester,
            std::shared_ptr<AvoidVortexLevelSaveData> sd,
            float width,
            float height,
            float maxZ)
            : Level(std::move(inGameRequester), width, height, maxZ),
              maxX(m_width/2),
              maxY(m_height/2),
              prevTime(std::chrono::high_resolution_clock::now()) {
        loadModels();
        preGenerate();
        if (sd == nullptr) {
            generate();
        } else {
            ball.position.x = sd->ball.x;
            ball.position.y = sd->ball.y;
            holePosition.x = sd->hole.x;
            holePosition.y = sd->hole.y;
            startPosition.x = sd->startPos.x;
            startPosition.y = sd->startPos.y;
            vortexPositions.reserve(sd->vortexes.size());
            for (auto const &vortex : sd->vortexes) {
                vortexPositions.emplace_back(vortex.x, vortex.y,
                                             m_maxZ - scaleFactor * m_originalBallDiameter);
            }
        }
        postGenerate();
        generateModelMatrices();
    }
    ~AvoidVortexLevel() override = default;
};
#endif