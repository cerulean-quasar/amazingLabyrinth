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
    struct ParametersGL : public Parameters {
        bool useIntTexture;

        virtual ~ParametersGL() = default;
    };

    struct ParametersWithShadowsGL : public ParametersGL {
        std::shared_ptr<graphicsGL::Framebuffer> shadowsFB;

        ~ParametersWithShadowsGL() override = default;
    };

    struct ParametersWithWidthHeightAtDepthGL : public ParametersGL {
        float widthAtDepth;
        float heightAtDepth;
        float nearestDepth;
        float farthestDepth;

        ~ParametersWithSurfaceWidthHeightAtDepthGL() override = default;
    };

    class DrawObjectDataGL : public DrawObjectData {
    public:
        virtual glm::mat4 modelMatrix(uint32_t) = 0;

        ~DrawObjectDataGL() override = default;
    };

    class RenderDetailsGL : public RenderDetails {
    public:
        // For postprocessing results written to an image buffer whose contents are put in input.
        // This function is for render details that produce results to be used by the CPU.  Most
        // render details don't need this.
        virtual void postProcessImageBuffer(
                std::shared_ptr<renderDetails::CommonObjectData> const &,
                std::vector<float> const &input,
                std::vector<float> &results)
        {
            output.resize(input.size());
            std::copy(input.begin(), input.end(), output.begin());
        }

        // GL commands that need to occur before the main draw.  They can be things like generating
        // a shadow map or other draws to a framebuffer.  Most shaders don't need these commands so
        // the default does nothing.
        virtual void preMainDraw(
                uint32_t modelMatrixID,
                levelDrawer::LevelDrawerGL::CommonObjectDataList const &commonObjectDataList,
                levelDrawer::LevelDrawerGL::DrawObjectTableList const &drawObjTableList,
                levelDrawer::LevelDrawerGL::IndicesForDrawList const &drawObjectsIndicesList)
        {}

        virtual void reload(std::shared_ptr<GameRequester> const &gameRequester,
                            std::shared_ptr<RenderLoaderGL> const &renderLoader,
                            ParametersGL const &parameters)
        {
            // do nothing.  For GL, we need to dump all render details and reload everything.
        }

        virtual void draw(
                uint32_t modelMatrixID,
                std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
                std::shared_ptr<levelDrawer::DrawObjectTableGL> const &drawObjTable,
                std::vector<size_t> const &drawObjectsIndices) = 0;

        virtual bool overrideClearColor(glm::vec4 &clearColor) {
            return false;
        }

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