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
#ifndef AMAZING_LABYRINTH_DARK_RENDER_DETAILS_GL_HPP
#define AMAZING_LABYRINTH_DARK_RENDER_DETAILS_GL_HPP

#include <memory>
#include <array>

#include "renderDetails.hpp"

namespace renderDetails {
    struct ParametersDarkObjectGL : public ParametersPerspective {
        std::vector<std::shared_ptr<graphicsGL::Framebuffer>> darkFramebuffers;
        bool frameBuffersValid;
        std::vector<glm::mat4> projViewsLights;

        ParametersDarkObjectGL(ParametersPerspective const &parameters,
                               std::vector<std::shared_ptr<graphicsGL::Framebuffer>> fbs,
                               std::vector<glm::mat4> inProjViewLights)
                : ParametersPerspective(parameters),
                  darkFramebuffers(std::move(fbs)),
                  frameBuffersValid(true),
                  projViewsLights(std::move(inProjViewLights)) {}

        ParametersDarkObjectGL(ParametersPerspective const &parameters,
                               std::vector<glm::mat4> inProjViewLights)
                : ParametersPerspective(parameters),
                  darkFramebuffers{},
                  frameBuffersValid(false),
                  projViewsLights(std::move(inProjViewLights)) {}

        ~ParametersDarkObjectGL() override = default;
    };
} // renderDetails

#endif // AMAZING_LABYRINTH_DARK_RENDER_DETAILS_GL_HPP