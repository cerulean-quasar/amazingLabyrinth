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
#ifndef AMAZING_LABYRINTH_RENDER_DETAILS_GL_HPP
#define AMAZING_LABYRINTH_RENDER_DETAILS_GL_HPP
#include <memory>
#include <string>

#include <GLES3/gl3.h>

#include "../common.hpp"
#include "renderDetails.hpp"
#include "../graphicsGL.hpp"
#include "../levelDrawer/levelDrawerGL.hpp"

namespace renderDetails {
    struct ParametersGL {
        bool useIntTexture;
        uint32_t width;
        uint32_t height;

        virtual ~ParametersGL() = default;
    };

    struct ParametersWithShadowsGL : public ParametersGL {
        std::shared_ptr<graphicsGL::Framebuffer> shadowsFB;

        ~ParametersWithShadowsGL() override = default;
    };

    class DrawObjectDataGL : public DrawObjectData {
    public:
        virtual glm::mat4 modelMatrix(uint32_t) = 0;

        ~DrawObjectDataGL() override = default;
    };

    class RenderDetailsGL : public RenderDetails {
    public:

        // GL commands that need to occur before the main draw.  They can be things like generating
        // a shadow map or other draws to a framebuffer.  Most shaders don't need these commands so
        // the default does nothing.
        virtual void preMainDraw(
                uint32_t modelMatrixID,
                renderDetails::DrawTypes<levelDrawer::DrawObjectTableGL>::CommonObjectDataList const &commonObjectDataList,
                renderDetails::DrawTypes<levelDrawer::DrawObjectTableGL>::DrawObjectTableList const &drawObjTableList,
                renderDetails::DrawTypes<levelDrawer::DrawObjectTableGL>::IndicesForDrawList const &drawObjectsIndicesList)
        {}

        virtual void draw(
                uint32_t modelMatrixID,
                std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
                std::shared_ptr<levelDrawer::DrawObjectTableGL> const &drawObjTable,
                std::vector<size_t> const &drawObjectsIndices) = 0;

        RenderDetailsGL(uint32_t inWidth, uint32_t inHeight)
        : RenderDetails{inWidth, inHeight}
        {}

        ~RenderDetailsGL() override  = default;
    protected:
        static void drawVertices(
                GLuint programID,
                std::shared_ptr<levelDrawer::ModelDataGL> const &modelData);

        static GLuint loadShaders(std::shared_ptr<GameRequester> const &gameRequester,
                           std::string const &vertexShaderFile, std::string const &fragmentShaderFile);
    };

    using ReferenceGL = Reference<RenderDetailsGL, CommonObjectData, DrawObjectDataGL>;
}

#endif // AMAZING_LABYRINTH_RENDER_DETAILS_GL_HPP