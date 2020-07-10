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
#ifndef AMAZING_LABYRINTH_RENDER_DETAILS_COMMON_GL_HPP
#define AMAZING_LABYRINTH_RENDER_DETAILS_COMMON_GL_HPP
#include <memory>
#include <string>

#include <GLES3/gl3.h>

#include "../common.hpp"
#include "renderDetails.hpp"
#include "../levelDrawer/drawObjectTable/drawObjectTable.hpp"
#include "../levelDrawer/drawObjectTable/drawObjectTableGL.hpp"
#include "../graphicsGL.hpp"

namespace renderDetails {
    struct ParametersGL {
        bool useIntTexture;
        uint32_t width;
        uint32_t height;
    };

    struct ParametersWithShadowsGL : public ParametersGL {
        std::shared_ptr<graphicsGL::Framebuffer> shadowsFB;
    };

    class DrawObjectDataGL : public DrawObjectData {
    public:
        virtual glm::mat4 modelMatrix() = 0;

        ~DrawObjectDataGL() override = default;
    };

    class RenderDetailsGL : public RenderDetails {
    public:
        virtual void draw(
                levelDrawer::DrawObjectTable<levelDrawer::DrawObjectGLTraits> const &drawObjectTable,
                std::vector<size_t> const &drawObjectsIndexList) = 0;

        RenderDetailsGL(uint32_t inWidth, uint32_t inHeight)
        : RenderDetails{inWidth, inHeight}
        {}

        ~RenderDetailsGL() override  = default;
    protected:
        static GLuint loadShaders(std::shared_ptr<GameRequester> const &gameRequester,
                           std::string const &vertexShaderFile, std::string const &fragmentShaderFile);
    };

    using ReferenceGL = renderDetails::RenderDetailsReference<RenderDetailsGL, renderDetails::CommonObjectData, renderDetails::DrawObjectDataGL>;
}

#endif // AMAZING_LABYRINTH_RENDER_DETAILS_COMMON_GL_HPP