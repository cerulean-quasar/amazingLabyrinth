/**
 * Copyright 2026 Cerulean Quasar. All Rights Reserved.
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
        float viewAngleShadows() const { return m_shadowMapViewAngleConstant; }
        float errorConstant() const { return m_errorConstant; }
        float gameBoardWidth() const { return m_gameBoardWidth; }
        float gameBoardHeight() const { return m_gameBoardHeight; }
        bool isLightSourceMobile(size_t lightSourceNumber) const { return m_lightSourceMoved[lightSourceNumber]; }
        std::vector<bool> const &lightSourceMoved() const { return m_lightSourceMoved; }

        glm::mat4 getLightProjView(size_t lightNumber, size_t direction, float aspectRatio, bool invertY, bool depth0to1) const {
            ParametersPerspective parameters = toShadowsParametersPerspective(lightNumber, direction);
            return getPerspectiveMatrix(
                        parameters.viewAngle,
                        aspectRatio,
                        parameters.nearPlane,
                        parameters.farPlane,
                        invertY, depth0to1) *
                glm::lookAt(parameters.viewPoint, parameters.lookAt, parameters.up);

        }

        size_t pushBackLightSource(float x, float y, bool isMobile) {
            glm::vec3 lightSource{x, y, m_floorZ + m_ballRadius};
            m_lightSources.push_back(lightSource);
            m_lightSourceMoved.push_back(isMobile);
            return m_lightSources.size();
        }

        void updateFloorZ(float z) {
            m_floorZ = z;
        }

        void updateBallRadius(float r) {
            m_ballRadius = r;
        }

        void updateLightSource(size_t lightSourceNumber, float x, float y) {
            if (lightSourceNumber >= m_lightSources.size()) {
                throw std::runtime_error("Viewpoint requested is out of range.");
            }

            m_lightSources[lightSourceNumber].x = x;
            m_lightSources[lightSourceNumber].y = y;
            m_lightSources[lightSourceNumber].z = m_floorZ + m_ballRadius;
        }

        std::shared_ptr<ParametersPerspective> toGamePerspective() const {
            std::shared_ptr<ParametersPerspective> parameters = gameConstants::getPerspectiveParameters();
            parameters->lightingSources = m_lightSources;

            return parameters;
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

            parameters.viewAngle = m_shadowMapViewAngleConstant;
            parameters.up = glm::vec3{0.0f, 0.0f, 1.0f};
            parameters.lookAt = glm::vec3{m_lightSources[lightNumber].x, m_lightSources[lightNumber].y,  m_floorZ + m_ballRadius};
            float distanceViewPointNearPlane = m_gameBoardHeight;
            switch (direction) {
            case 0:
                parameters.viewPoint = glm::vec3{ m_lightSources[lightNumber].x, m_lightSources[lightNumber].y - distanceViewPointNearPlane, m_floorZ + m_ballRadius };
                parameters.farPlane = m_gameBoardHeight/2 - m_lightSources[lightNumber].y;
                break;
            case 1:
                parameters.viewPoint = glm::vec3{m_lightSources[lightNumber].x + distanceViewPointNearPlane, m_lightSources[lightNumber].y, m_floorZ + m_ballRadius};
                parameters.farPlane = -m_gameBoardWidth/2 - m_lightSources[lightNumber].x;
                break;
            case 2:
                parameters.viewPoint = glm::vec3{m_lightSources[lightNumber].x, m_lightSources[lightNumber].y + distanceViewPointNearPlane, m_floorZ + m_ballRadius};
                parameters.farPlane = -m_gameBoardHeight/2 - m_lightSources[lightNumber].y;
                break;
            case 3:
                parameters.viewPoint = glm::vec3{m_lightSources[lightNumber].x - distanceViewPointNearPlane, m_lightSources[lightNumber].y, m_floorZ + m_ballRadius};
                parameters.farPlane = m_gameBoardWidth/2 - m_lightSources[lightNumber].x;
                break;
            default:
                throw std::runtime_error("Invalid direction in dark shadows perspective");
            }

            if (direction == 0 || direction == 2) {
                parameters.nearPlane = m_lightSources[lightNumber].y;
            } else {
                parameters.nearPlane = m_lightSources[lightNumber].x;
            }

            return parameters;
        }

        ParametersDark(size_t numberLightSources)
            : ParametersBase(),
            m_gameBoardWidth{1.0f},
            m_gameBoardHeight{1.0f},
            m_ballRadius{1.0f/20.0f},
            m_floorZ{levelTracker::m_maxZLevel},
            m_lightSources{}
        {
            for (size_t i = 0; i < numberLightSources; i++) {
                m_lightSources.push_back(glm::vec3{0.0f, 0.0f, m_floorZ + m_ballRadius});
                m_lightSourceMoved.push_back(true);
            }
        }

        ParametersDark(float inFloorZ, float inGameBoardWidth, float inGameBoardHeight, float inBallRadius)
            : ParametersBase(),
            m_floorZ{inFloorZ},
            m_gameBoardWidth{inGameBoardWidth},
            m_gameBoardHeight{inGameBoardHeight},
            m_lightSources{},
            m_ballRadius{inBallRadius}
        {}

        ~ParametersDark() override = default;

    private:
        static float const constexpr m_shadowMapViewAngleConstant = glm::pi<float>()/2;
        static float const constexpr m_errorConstant = 0.001;
        float const m_gameBoardWidth;
        float const m_gameBoardHeight;
        float m_floorZ;
        float m_ballRadius;

        // The first member of the pair is the position of the light source, the second member
        // indicates whether it is static or not.
        std::vector<glm::vec3> m_lightSources;

        std::vector<bool> m_lightSourceMoved;
    };
} // renderDetails

#endif // AMAZING_LABYRINTH_DARK_RENDER_DETAILS_HPP