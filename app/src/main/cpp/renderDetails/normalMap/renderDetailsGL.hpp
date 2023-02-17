/**
 * Copyright 2022 Cerulean Quasar. All Rights Reserved.
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
#ifndef AMAZING_LABYRINTH_NORMALMAP_RENDER_DETAILS_GL_HPP
#define AMAZING_LABYRINTH_NORMALMAP_RENDER_DETAILS_GL_HPP

#include <memory>

#include <glm/glm.hpp>

#include <GLES3/gl3.h>
#include <boost/variant.hpp>

#include "../renderDetailsGL.hpp"
#include "../renderDetails.hpp"
#include "../../levelDrawer/textureTable/textureLoader.hpp"
#include "../../renderLoader/renderLoaderGL.hpp"

namespace normalMap {
    class CommonObjectDataGL : public renderDetails::CommonObjectDataOrtho {
    public:
        std::pair<glm::mat4, glm::mat4> getProjViewForLevel() override {
            return std::make_pair<glm::mat4, glm::mat4>(
                    getOrthoMatrix(m_minusX, m_plusX, m_minusY, m_plusY,
                                   m_nearPlane, m_farPlane, false, false),
                    view());
        }

        CommonObjectDataGL(
                renderDetails::ParametersNormalMap const &parameters)
                : renderDetails::CommonObjectDataOrtho(parameters.toOrtho())
        {}

        ~CommonObjectDataGL() override = default;
    private:
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

    class PostProcessNormalsVisitor : public boost::static_visitor<std::vector<float>> {
    public:
        PostProcessNormalsVisitor(uint32_t surfaceWidth, uint32_t surfaceHeight)
                : m_surfaceWidth{surfaceWidth},
                  m_surfaceHeight{surfaceHeight}
        {}
        template <typename value_type>
        std::vector<float> operator()(std::vector<value_type> const &input) {
            std::vector<float> results;
            bitmapToNormals(input, m_surfaceWidth, m_surfaceHeight, 4, false, results);
            return std::move(results);
        }

    private:
        uint32_t m_surfaceWidth;
        uint32_t m_surfaceHeight;
    };

    class RenderDetailsGL : public renderDetails::RenderDetailsGL {
    public:
        std::string nameString() override { return name(); }
        static char const *name() { return normalMapRenderDetailsName; }

        static renderDetails::ReferenceGL loadNew(
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
                std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
                std::shared_ptr<levelDrawer::DrawObjectTableGL> const &drawObjTable,
                std::set<levelDrawer::ZValueReference>::iterator beginZValRefs,
                std::set<levelDrawer::ZValueReference>::iterator endZValRefs) override;

        bool overrideClearColor(glm::vec4 &clearColor) override {
            clearColor = {0.5f, 0.5f, 1.0f, 1.0f};
            return true;
        }

        void postProcessImageBuffer(
                std::shared_ptr<renderDetails::CommonObjectData> const &,
                renderDetails::PostprocessingDataInputGL const &input,
                std::vector<float> &results) override
        {
            PostProcessNormalsVisitor postProcessNormalsVisitor(m_surfaceWidth, m_surfaceHeight);
            results = boost::apply_visitor(postProcessNormalsVisitor, input);
        }

        void loadPipeline(std::shared_ptr<GameRequester> const &inGameRequester);

        RenderDetailsGL(std::shared_ptr<GameRequester> const &inGameRequester,
                        uint32_t inWidth, uint32_t inHeight, bool isIntSurface);

        ~RenderDetailsGL() override {
            glDeleteShader(m_programID);
        }

    private:
        static char constexpr const *NORMAL_VERT_FILE ="shaders/normalGL.vert";
        static char constexpr const *SIMPLE_FRAG_FILE = "shaders/simpleGL.frag";
        static char constexpr const *NORMAL3_VERT_FILE ="shaders/normalGL3.vert";
        static char constexpr const *SIMPLE3_FRAG_FILE = "shaders/simpleGL3.frag";

        GLuint m_programID;

        static renderDetails::ReferenceGL createReference(
                std::shared_ptr<renderDetails::RenderDetailsGL> rd,
                std::shared_ptr<CommonObjectDataGL> cod);
    };
}
#endif // AMAZING_LABYRINTH_NORMALMAP_RENDER_DETAILS_GL_HPP
