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
#ifndef AMAZING_LABYRINTH_DEPTHMAP_RENDER_DETAILS_GL_HPP
#define AMAZING_LABYRINTH_DEPTHMAP_RENDER_DETAILS_GL_HPP

#include <memory>

#include <glm/glm.hpp>

#include <GLES3/gl3.h>

#include "../renderDetailsGL.hpp"
#include "../renderDetails.hpp"
#include "../../levelDrawer/textureTable/textureLoader.hpp"
#include "../../levelDrawer/common.hpp"
#include "../../renderLoader/renderLoaderGL.hpp"

namespace depthMap {
    class CommonObjectDataGL : public renderDetails::CommonObjectDataOrtho {
    public:
        std::pair<glm::mat4, glm::mat4> getProjViewForLevel() override {
            return std::make_pair<glm::mat4, glm::mat4>(
                    getOrthoMatrix(m_minusX, m_plusX, m_minusY, m_plusY,
                                   m_nearPlane, m_farPlane, false, false),
                    view());
        }

        float nearestDepth() { return m_nearestDepth; }
        float farthestDepth() { return m_farthestDepth; }

        CommonObjectDataGL(renderDetails::ParametersDepthMap const &parameters)
                : CommonObjectDataOrtho(parameters.toOrtho()),
                  m_nearestDepth{parameters.nearestDepth},
                  m_farthestDepth{parameters.farthestDepth}
        {}

        ~CommonObjectDataGL() override = default;
    private:
        float m_nearestDepth;
        float m_farthestDepth;
    };

    class DrawObjectDataGL : public renderDetails::DrawObjectDataGL {
    public:
        glm::mat4 modelMatrix(uint32_t) override { return m_modelMatrix; }

        void update(glm::mat4 const &modelMatrix) override {
            m_modelMatrix = modelMatrix;
        }

        DrawObjectDataGL(glm::mat4 const &inModelMatrix)
                : m_modelMatrix{inModelMatrix}
        {}

        ~DrawObjectDataGL() override = default;
    private:
        glm::mat4 m_modelMatrix;
    };

    class PostProcessDepthVisitor : public boost::static_visitor<std::vector<float>> {
    public:
        template <typename value_type>
        std::vector<float> operator() (std::vector<value_type> const &input) {
            std::vector<float> results;
            bitmapToDepthMap(input, m_farthestDepth, m_nearestDepth,
                             m_surfaceWidth, m_surfaceHeight, 4, false, results);
            return std::move(results);
        }

        PostProcessDepthVisitor(
                uint32_t surfaceWidth, uint32_t surfaceHeight,
                float farthestDepth, float nearestDepth)
                : m_surfaceWidth{surfaceWidth},
                m_surfaceHeight{surfaceHeight},
                m_farthestDepth{farthestDepth},
                m_nearestDepth{nearestDepth}
        {}
    private:
        uint32_t m_surfaceWidth;
        uint32_t m_surfaceHeight;
        float m_farthestDepth;
        float m_nearestDepth;
    };

    class RenderDetailsGL : public renderDetails::RenderDetailsGL {
    public:
        static renderDetails::ReferenceGL loadNew(
                renderDetails::Description const &description,
                std::vector<char const *> const &shaders,
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<RenderLoaderGL> const &renderLoader,
                std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails,
                std::shared_ptr<renderDetails::Parameters> const &parameters);

        static renderDetails::ReferenceGL loadExisting(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<RenderLoaderGL> const &renderLoader,
                std::shared_ptr<renderDetails::RenderDetailsGL> rdBase,
                std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails,
                std::shared_ptr<renderDetails::Parameters> const &parameters);

        void draw(
                uint32_t modelMatrixID,
                std::shared_ptr<renderDetails::CommonObjectDataBase> const &commonObjectData,
                std::shared_ptr<levelDrawer::DrawObjectTableGL> const &drawObjTable,
                std::set<levelDrawer::ZValueReference>::iterator beginZValRefs,
                std::set<levelDrawer::ZValueReference>::iterator endZValRefs) override;

        void postProcessImageBuffer(
                std::shared_ptr<renderDetails::CommonObjectDataBase> const &commonObjectData,
                renderDetails::PostprocessingDataInputGL const &input,
                std::vector<float> &results) override
        {
            auto cod = dynamic_cast<CommonObjectDataGL*>(commonObjectData.get());
            PostProcessDepthVisitor visitor(m_surfaceWidth, m_surfaceHeight, cod->farthestDepth(), cod->nearestDepth());
            results = boost::apply_visitor(visitor, input);
        }

        bool overrideClearColor(glm::vec4 &clearColor) override {
            clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
            return true;
        }

        RenderDetailsGL(
                renderDetails::Description description,
                char const *linearDepthVertShader,
                char const *linearDepthVertShader3,
                char const *simpleFragShader,
                char const *simpleFragShader3,
                std::shared_ptr<GameRequester> const &inGameRequester,
                uint32_t inWidth, uint32_t inHeight, bool useIntSurface);

        ~RenderDetailsGL() override = default;

    private:
        char const *m_linearDepthVertShader;
        char const *m_linearDepthVertShader3;
        char const *m_simpleFragShader;
        char const *m_simpleFragShader3;
        renderDetails::Program m_depthProgram;

        static renderDetails::ReferenceGL createReference(
                std::shared_ptr<renderDetails::RenderDetailsGL> rd,
                std::shared_ptr<CommonObjectDataGL> cod);

        void loadPipeline(std::shared_ptr<GameRequester> const &inGameRequester);
    };
}
#endif // AMAZING_LABYRINTH_DEPTHMAP_RENDER_DETAILS_GL_HPP
