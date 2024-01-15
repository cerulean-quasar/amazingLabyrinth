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

#ifndef AMAZING_LABYRINTH_BASIC_LEVEL_HPP
#define AMAZING_LABYRINTH_BASIC_LEVEL_HPP

#include <functional>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../../common.hpp"
#include "loadData.hpp"
#include "../../levelTracker/types.hpp"
#include "../../mathGraphics.hpp"
#include "../../levelDrawer/levelDrawer.hpp"
#include "../../levelDrawer/modelTable/modelLoader.hpp"
#include "../../levelDrawer/textureTable/textureLoader.hpp"

namespace basic {
    class Level {
    protected:
        struct ModelDatum {
            std::vector<std::shared_ptr<levelDrawer::ModelDescription>> models;
            std::vector<std::shared_ptr<levelDrawer::TextureDescription>> textures;
            std::vector<std::shared_ptr<levelDrawer::TextureDescription>> alternateTextures;
        };

        using LoadedModelData = std::map<std::string, ModelDatum>;

        LoadedModelData m_modelData;

    private:
        LoadedModelData loadModels(std::vector<ModelConfigData> const &configData) {
            LoadedModelData finalData;
            for (auto const &configDatum : configData) {
                ModelDatum modelDatum;
                glm::vec3 defaultColor{configDatum.defaultColor[0], configDatum.defaultColor[1],
                                       configDatum.defaultColor[2]};
                if (!configDatum.modelFiles.empty()) {
                    uint8_t normalsToLoad = 0;
                    if (configDatum.loadFaceNormals) {
                        normalsToLoad |= levelDrawer::ModelDescription::LOAD_FACE_NORMALS;
                    }
                    if (configDatum.loadVertexNormals) {
                        normalsToLoad |= levelDrawer::ModelDescription::LOAD_VERTEX_NORMALS;
                    }
                    for (auto const &modelFile : configDatum.modelFiles) {
                        modelDatum.models.emplace_back(std::make_shared<levelDrawer::ModelDescriptionPath>(
                                modelFile, defaultColor, normalsToLoad));
                    }
                } else if (configDatum.modelType == modelTypeCube) {
                    modelDatum.models.emplace_back(std::make_shared<levelDrawer::ModelDescriptionCube>(glm::vec3{0.0f, 0.0f, 0.0f}, defaultColor));
                } else if (configDatum.modelType == modelTypeSquare) {
                    modelDatum.models.emplace_back(std::make_shared<levelDrawer::ModelDescriptionQuad>(glm::vec3{0.0f, 0.0f, 0.0f}, defaultColor));
                } else {
                    throw std::runtime_error("Loading Models: Invalid model type specified.");
                }

                for (auto const &textureFile : configDatum.textures) {
                    modelDatum.textures.push_back(std::make_shared<levelDrawer::TextureDescriptionPath>(
                            textureFile));
                }

                for (auto const &textureFile : configDatum.alternateTextures) {
                    modelDatum.alternateTextures.push_back(std::make_shared<levelDrawer::TextureDescriptionPath>(
                            textureFile));
                }

                finalData.emplace(configDatum.modelName, modelDatum);
            }

            return finalData;
        }

    protected:
        /* Names of models that we know in basic */
        static char constexpr const *ModelNameBall = "Ball";
        static char constexpr const *ModelNameHole = "Hole";
        static char constexpr const *ModelNameFloor = "Floor";

        static float constexpr m_originalBallDiameter = 2.0f;
        static float constexpr m_dragConstant = 0.2f;
        static float constexpr m_accelerationAdjustment = 0.1f;
        static float constexpr m_lengthTooSmallToNormalize = 0.001f;
        static float constexpr m_modelSize = 2.0f;

        levelDrawer::Adaptor m_levelDrawer;
        bool m_finished;
        float m_width;
        float m_height;
        float m_diagonal;
        float const m_mazeFloorZ;
        bool const m_ignoreZMovement;
        float m_scaleBall;
        bool m_bounce;

        // data on where the ball is, how fast it is moving, etc.
        struct {
            glm::vec3 prevPosition;
            glm::vec3 position;
            glm::vec3 velocity;
            glm::vec3 acceleration;
            glm::quat totalRotated;
        } m_ball;

        /**
         * Find the model entry in the list of model info in the config file.
         * @param modelName Name of the model (if no models for this name are found, an exception
         *        will be thrown.
         * @param nbrOfMinAllowedTextures Minimum number of textures expected. If the number of
         *        textures is less than this value, an exception is thrown.
         * @return The entry of the model config loaded from the config file.
         */
        auto const &findModelsAndTextures(std::string const &modelName, size_t nbrOfMinAllowedTextures = 0) {
            auto const it = m_modelData.find(modelName);
            if (it == m_modelData.end()) {
                throw std::runtime_error("Model data for: " + modelName + " not found in config file.");
            }

            if (it->second.models.empty()) {
                throw std::runtime_error("No models loaded for model: " + modelName);
            }

            if (it->second.textures.size() < nbrOfMinAllowedTextures) {
                throw std::runtime_error("Not enough textures for model: " +
                modelName + ". Expecting at least " + std::to_string(nbrOfMinAllowedTextures));
            }

            return it->second;
        }

        /**
         * Find the first texture in the model table entry.
         * @param modelDatum the table entry to get the texture from.
         * @param noneOk if true, then a null pointer will be returned if there are no textures,
         * if false, then an exception is thrown
         * @param returnAlternateTextures if true, return the first texture from the alternate
         * textures, otherwise use the non alternate textures.
         * @return a pointer to the texture.
         */
        auto getFirstTexture(ModelDatum const &modelDatum, bool noneOk = true, bool returnAlternateTextures = false) {
            auto const &textures = returnAlternateTextures ? modelDatum.alternateTextures : modelDatum.textures;
            if (textures.empty()) {
                if (noneOk) {
                    return std::shared_ptr<levelDrawer::TextureDescription>();
                } else {
                    throw std::runtime_error("Texture expected, however none configured for this model");
                }
            } else {
                return textures[0];
            }
        }

        float ballRadius() { return m_originalBallDiameter * m_scaleBall / 2.0f; }

        float ballDiameter() { return m_originalBallDiameter * m_scaleBall; }

        glm::mat4 ballScaleMatrix() { return glm::scale(glm::mat4(1.0f),
                                                        glm::vec3(m_scaleBall, m_scaleBall,
                                                                  m_scaleBall));
        }

        glm::vec3 getUpdatedPosition(float timeDiff) {
            return m_ball.position + m_ball.velocity * timeDiff + m_ball.acceleration * 0.5f * timeDiff * timeDiff;
        }

        glm::vec3 getUpdatedPosition(float timeDiff, glm::vec3 const &velocity, glm::vec3 const &acceleration) {
            return m_ball.position + velocity * timeDiff + acceleration * 0.5f * timeDiff * timeDiff;
        }

        glm::vec3 getUpdatedPosition(float timeDiff, glm::vec3 const &velocity, glm::vec3 const &acceleration, glm::vec3 const &position) {
            return position + velocity * timeDiff + acceleration * 0.5f * timeDiff * timeDiff;
        }

        glm::vec3 getUpdatedVelocity(float timeDiff) {
            return m_ball.velocity + m_ball.acceleration * timeDiff;
        }

        glm::vec3 getUpdatedVelocity(glm::vec3 const &acceleration, float timeDiff) {
            return m_ball.velocity + acceleration * timeDiff;
        }

        bool drawingNecessary() {
            bool _drawingNecessary =
                    glm::length(m_ball.position - m_ball.prevPosition) > m_diagonal / 200.0f;
            if (_drawingNecessary) {
                m_ball.prevPosition = m_ball.position;
            }

            return _drawingNecessary;
        }

        void updateRotation(float timeDiff) {
            glm::vec3 axis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), m_ball.velocity);
            if (glm::length(axis) > m_lengthTooSmallToNormalize) {
                float scaleFactor = 10.0f;
                glm::quat q = glm::angleAxis(glm::length(m_ball.velocity) * timeDiff * scaleFactor,
                                             glm::normalize(axis));

                m_ball.totalRotated = glm::normalize(q * m_ball.totalRotated);
            }
        }

        bool checkBallBorders(glm::vec3 &position, glm::vec3 &velocity);

    public:
        struct Request {
            virtual void defaultRD() {
                renderDetails::Query query{
                    renderDetails::DrawingStyle::standard,
                    {renderDetails::Features::color,
                        renderDetails::Features::texture},
                    {}};

                if (m_shadowsEnabled) {
                    query.optionalFeatures.setFeature({renderDetails::Features::chaining, renderDetails::Features::shadows});
                }

                m_levelDrawer.requestRenderDetails(
                    query, levelDrawer::DefaultConfig::getDefaultParameters());
            }

            Request(levelDrawer::Adaptor levelDrawer, bool shadowsEnabled)
                : m_levelDrawer(std::move(levelDrawer)),
                  m_shadowsEnabled(shadowsEnabled)
            {}

        protected:
            levelDrawer::Adaptor m_levelDrawer;
            bool m_shadowsEnabled;
        };

        static float constexpr m_floatErrorAmount = 0.0001f;

        virtual void updateAcceleration(float x, float y, float z) {
            m_ball.acceleration =
                    m_accelerationAdjustment * glm::vec3{-x, -y, m_ignoreZMovement ? 0 : -z};

            if ((m_ball.acceleration.x < 0.0f && m_ball.velocity.x > 0.0f) ||
                (m_ball.acceleration.x > 0.0f && m_ball.velocity.x < 0.0f)) {
                m_ball.velocity.x = 0.0f;
            }

            if ((m_ball.acceleration.y < 0.0f && m_ball.velocity.y > 0.0f) ||
                (m_ball.acceleration.y > 0.0f && m_ball.velocity.y < 0.0f)) {
                m_ball.velocity.y = 0.0f;
            }

            /* z is a special case where we want to keep it going for now.  For the fixed maze
             * mostly.
             */
        }

        virtual bool updateData() = 0;

        virtual bool updateDrawObjects() = 0;

        virtual void start() = 0;

        bool isFinished() { return m_finished; }

        virtual void getLevelFinisherCenter(float &x, float &y, float &z) = 0;

        virtual bool tap(float, float) { return false; }

        virtual bool drag(float, float, float, float) { return false; }

        virtual bool dragEnded(float, float) { return false; }

        virtual float getZForTapCoords() { return m_mazeFloorZ; }

        virtual char const *name() = 0;

        virtual std::vector<uint8_t> saveData(levelTracker::GameSaveData const &gsd, char const *saveLevelDataKey) = 0;

        Level(
                levelDrawer::Adaptor inLevelDrawer,
                std::shared_ptr<LevelConfigData> const &lcd,
                float mazeFloorZ,
                bool ignoreZMovement,
                Request &request)
                : m_levelDrawer{std::move(inLevelDrawer)},
                  m_finished(false),
                  m_mazeFloorZ{mazeFloorZ},
                  m_ignoreZMovement{ignoreZMovement},
                  m_bounce{lcd->bounceEnabled},
                  m_modelData{}
        {
            if (!lcd) {
                throw (std::runtime_error("Level Configuration missing"));
            }

            m_modelData = loadModels(lcd->models);

            request.defaultRD();

            auto projView = m_levelDrawer.getProjectionView();
            auto wh = getWidthHeight(mazeFloorZ, projView.first, projView.second);
            m_width = wh.first;
            m_height = wh.second;
            m_diagonal = glm::length(glm::vec2{m_width, m_height});
            m_scaleBall = lcd->ballSizeDiagonalRatio * m_diagonal;
            m_ball.totalRotated = glm::quat();
            m_ball.acceleration = {0.0f, 0.0f, 0.0f};
            m_ball.velocity = {0.0f, 0.0f, 0.0f};

            // put an out of bounds previous position so that we are sure to draw on the first
            // draw cycle.
            m_ball.prevPosition = {-10.0f, 0.0f, 0.0f};

            // the derived level will change this
            m_ball.position = {0.0f, 0.0f, 0.0f};
        }

        virtual ~Level() = default;
    };
} // namespace basic
#endif // AMAZING_LABYRINTH_BASIC_LEVEL_HPP
