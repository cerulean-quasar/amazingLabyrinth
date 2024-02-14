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
    size_t constexpr const numberOfLightSourcesDarkMaze = 2;
    size_t constexpr const numberOfShadowMapsPerDarkObject = 4;
    size_t constexpr const numberOfShadowMapsDarkMaze = numberOfShadowMapsPerDarkObject * numberOfLightSourcesDarkMaze;

    inline void darkInitializeShadowMapParameters(ParametersPerspective &parametersShadows, ParametersPerspective const &parameters, size_t i, bool completeInitializationRequired) {
        // shadows CODs
        if (completeInitializationRequired) {
            parametersShadows = parameters;
            parametersShadows.lightingSources.resize(1);
        }
        parametersShadows.up = glm::vec3{0.0, 0.0, 1.0};

        switch (i / parameters.lightingSources.size()) {
            case 0:
                parametersShadows.lookAt = glm::vec3{0.0, 1.0, 0.0};
                break;
            case 1:
                parametersShadows.lookAt = glm::vec3{1.0, 0.0, 0.0};
                break;
            case 2:
                parametersShadows.lookAt = glm::vec3{0.0, -1.0, 0.0};
                break;
            case 3:
                parametersShadows.lookAt = glm::vec3{-1.0, 0.0, 0.0};
        }

        if (i == numberOfShadowMapsDarkMaze / parameters.lightingSources.size()) {
            parametersShadows.lightingSources[0] = parameters.lightingSources[i/numberOfShadowMapsPerDarkObject];
        }
    }
}

#endif // AMAZING_LABYRINTH_DARK_RENDER_DETAILS_HPP