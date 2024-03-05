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
#include "renderDetails/renderDetails.hpp"

namespace renderDetails {
    size_t constexpr const numberOfLightSourcesDarkMaze = 2;
    size_t constexpr const numberOfShadowMapsPerDarkObject = 4;
    size_t constexpr const numberOfShadowMapsDarkMaze = numberOfShadowMapsPerDarkObject * numberOfLightSourcesDarkMaze;

    struct ParametersDark : public ParametersBase {
    public:
        size_t pushBackViewPoint(float x, float y) {
            glm::vec3 viewPoint{x, y, m_minLevelZ + m_ballRadius};
            m_viewPoints.push_back(viewPoint);
            return m_viewPoints.size() - 1;
        }

        size_t numberViewPoints() { return m_viewPoints.size(); }

        void updateBallRadius(float inBallRadius) {
            m_ballRadius = inBallRadius;
        }

        void updateViewPoint(size_t viewPointNumber, float x, float y) {
            if (viewPointNumber >= m_viewPoints.size()) {
                throw std::runtime_error("Viewpoint requested is out of range.");
            }

            m_viewPoints[viewPointNumber].x = x;
            m_viewPoints[viewPointNumber].y = y;
            m_viewPoints[viewPointNumber].z = m_minLevelZ + m_ballRadius;
        }

        std::shared_ptr<ParametersPerspective> toGamePerspective() {
            return gameConstants::getPerspectiveParameters();
        }

        std::shared_ptr<ParametersPerspective> toShadowsPerspective(size_t direction, size_t viewPointNumber) {
            ParametersPerspective parameters;
            if (!m_initializingConstructorCalled) {
                if (viewPointNumber >= m_nbrViewPoints) {
                    throw std::runtime_error("Viewpoint requested is out of range.");
                }

                // not all of the information required to construct the shadows parameters
                // is known yet, so just give the caller the game parameters as a place holder
                return gameConstants::getPerspectiveParameters();
            }

            if (viewPointNumber >= m_viewPoints.size()) {
                throw std::runtime_error("Viewpoint requested is out of range.");
            }

            parameters.viewAngle = m_viewAngleConstant;
            parameters.viewPoint = m_viewPoints[viewPointNumber];
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

            return std::make_shared<ParametersPerspective>(parameters);
        }

        ParametersDark(size_t nbrViewPoints)
                : ParametersBase(),
                  m_ballRadius{},
                  m_gameBoardWidth{},
                  m_gameBoardHeight{},
                  m_minLevelZ{},
                  m_nbrViewPoints{nbrViewPoints},
                  m_viewPoints{},
                  m_initializingConstructorCalled{false}
        {}

        ParametersDark(std::vector<glm::vec3> viewPoints, float inBallRadius, float inGameBoardWidth, float inGameBoardHeight, float inMinLevelZ)
            : ParametersBase(),
              m_ballRadius{inBallRadius},
              m_gameBoardWidth{inGameBoardWidth},
              m_gameBoardHeight{inGameBoardHeight},
              m_minLevelZ{inMinLevelZ},
              m_nbrViewPoints{viewPoints.size()},
              m_viewPoints{std::move(viewPoints)},
              m_initializingConstructorCalled{true}
        {}

        ~ParametersDark() override = default;

    private:
        static float const constexpr m_viewAngleConstant = glm::pi<float>()/2;
        static float const constexpr m_errorConstant = 0.001;
        float m_ballRadius;
        float m_gameBoardWidth;
        float m_gameBoardHeight;
        float m_minLevelZ;
        size_t m_nbrViewPoints;
        std::vector<glm::vec3> m_viewPoints;

        bool m_initializingConstructorCalled;
    };

    class CommonObjectDataDark : public CommonObjectDataBase {
        ParametersDark const &parameters() { return m_parameters; }

        void update(ParametersBase const &parametersBase) override {
            auto parameters = dynamic_cast<ParametersDark const &>(parametersBase);
            m_parameters = parameters;
        }
    private:
        ParametersDark m_parameters;
    };
} // renderDetails

#endif // AMAZING_LABYRINTH_DARK_RENDER_DETAILS_HPP