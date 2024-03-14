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

#ifndef AMAZING_LABYRINTH_DARK_RENDER_DETAILS_HPP
#define AMAZING_LABYRINTH_DARK_RENDER_DETAILS_HPP

#include <memory>
#include <glm/glm.hpp>
#include <array>

#include "levelDrawer/common.hpp"

namespace renderDetails {
    size_t constexpr const numberDirections = 4;

    struct ParametersDark : public ParametersBase {
    public:
        size_t numberLightSources() const { return m_lightSources.size(); }
        float viewAngleShadows() const { return m_viewAngleConstant; }
        float errorConstant() const { return m_errorConstant; }
        float ballRadius() const { return m_ballRadius; }
        float gameBoardWidth() const { return m_gameBoardWidth; }
        float gameBoardHeight() const { return m_gameBoardHeight; }
        float minLevelZ() const { return m_minLevelZ; }
        bool isLightSourceMobile(size_t lightSourceNumber) const { return m_lightSourceMoved[lightSourceNumber]; }
        std::vector<bool> const &lightSourceMoved() const { return m_lightSourceMoved; }
        float aspectRatio() const { return m_minLevelZ / m_gameBoardWidth; }

        glm::mat4 getLightProjView(size_t lightNumber, size_t direction, bool invertY, bool invertZ) const {
            ParametersPerspective parameters = toShadowsParametersPerspective(lightNumber, direction);
            return getPerspectiveMatrix(
                        parameters.viewAngle,
                        aspectRatio(),
                        parameters.nearPlane,
                        parameters.farPlane,
                        invertY, invertZ) *
                glm::lookAt(parameters.viewPoint, parameters.lookAt, parameters.up);

        }

        size_t pushBackLightSource(float x, float y, bool isMobile) {
            glm::vec3 lightSource{x, y, m_minLevelZ + m_ballRadius};
            m_lightSources.push_back(lightSource);
            m_lightSourceMoved.push_back(isMobile);
            return m_lightSources.size() - 1;
        }

        void updateBallRadius(float inBallRadius) {
            m_ballRadius = inBallRadius;
        }

        void updateLightSource(size_t lightSourceNumber, float x, float y) {
            if (lightSourceNumber >= m_lightSources.size()) {
                throw std::runtime_error("Viewpoint requested is out of range.");
            }

            m_lightSources[lightSourceNumber].x = x;
            m_lightSources[lightSourceNumber].y = y;
            m_lightSources[lightSourceNumber].z = m_minLevelZ + m_ballRadius;
        }

        std::shared_ptr<ParametersPerspective> toGamePerspective() const {
            return gameConstants::getPerspectiveParameters();
        }

        std::shared_ptr<ParametersPerspective> toShadowsParametersPerspectivePtr(size_t lightNumber, size_t direction) const {
            ParametersPerspective parameters = toShadowsParametersPerspective(lightNumber, direction);
            return std::make_shared<ParametersPerspective>(parameters);
        }

        ParametersPerspective toShadowsParametersPerspective(size_t lightNumber, size_t direction) const {
            ParametersPerspective parameters;
            if (lightNumber >= m_lightSources.size()) {
                throw std::runtime_error("Viewpoint requested is out of range.");
            }

            parameters.viewAngle = m_viewAngleConstant;
            parameters.viewPoint = m_lightSources[lightNumber];
            parameters.up = glm::vec3{0.0, 0.0, 1.0};
            switch (direction) {
                case 0:
                    parameters.lookAt = glm::vec3{0.0, 1.0, m_minLevelZ + m_ballRadius};
                    break;
                case 1:
                    parameters.lookAt = glm::vec3{1.0, 0.0, m_minLevelZ + m_ballRadius};
                    break;
                case 2:
                    parameters.lookAt = glm::vec3{0.0, -1.0, m_minLevelZ + m_ballRadius};
                    break;
                case 3:
                    parameters.lookAt = glm::vec3{-1.0, 0.0, m_minLevelZ + m_ballRadius};
            }

            parameters.nearPlane = m_ballRadius - m_errorConstant;
            if (direction == 0 || direction == 2) {
                parameters.farPlane = m_gameBoardHeight;
            } else {
                parameters.farPlane = m_gameBoardWidth;
            }

            return parameters;
        }

        ParametersDark()
          : ParametersBase(),
            m_ballRadius{},
            m_gameBoardWidth{},
            m_gameBoardHeight{},
            m_minLevelZ{},
            m_lightSources{}
          {}

        ParametersDark(float inBallRadius, float inGameBoardWidth, float inGameBoardHeight, float inMinLevelZ)
        : ParametersBase(),
          m_ballRadius{inBallRadius},
          m_gameBoardWidth{inGameBoardWidth},
          m_gameBoardHeight{inGameBoardHeight},
          m_minLevelZ{inMinLevelZ},
          m_lightSources{}
        {}

        ~ParametersDark() override = default;

    private:
        static float const constexpr m_viewAngleConstant = glm::pi<float>()/2;
        static float const constexpr m_errorConstant = 0.001;
        float m_ballRadius;
        float m_gameBoardWidth;
        float m_gameBoardHeight;
        float m_minLevelZ;

        // The first member of the pair is the position of the light source, the second member
        // indicates whether it is static or not.
        std::vector<glm::vec3> m_lightSources;

        std::vector<bool> m_lightSourceMoved;
    };
} // renderDetails

#endif // AMAZING_LABYRINTH_DARK_RENDER_DETAILS_HPP