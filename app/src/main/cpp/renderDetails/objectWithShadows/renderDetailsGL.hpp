/**
 * Copyright 2023 Cerulean Quasar. All Rights Reserved.
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
#ifndef AMAZING_LABYRINTH_OBJECTWITHSHADOWS_RENDER_DETAILS_GL_HPP
#define AMAZING_LABYRINTH_OBJECTWITHSHADOWS_RENDER_DETAILS_GL_HPP

#include <memory>

#include <glm/glm.hpp>

#include <GLES3/gl3.h>

#include "../renderDetailsGL.hpp"
#include "../renderDetails.hpp"
#include "../../levelDrawer/textureTable/textureLoader.hpp"
#include "../../graphicsGL.hpp"
#include "../../renderLoader/renderLoaderGL.hpp"

namespace objectWithShadows {
    class CommonObjectDataGL : public renderDetails::CommonObjectDataPerspective {
    public:
        std::pair<glm::mat4, glm::mat4> getProjViewForLevel() override {
            /* perspective matrix: takes the perspective projection, the aspect ratio, near and far
             * view planes.
             */
            return std::make_pair<glm::mat4, glm::mat4>(
                    getPerspectiveMatrix(m_viewAngle, m_aspectRatio, m_nearPlane, m_farPlane,
                                         false, false),
                    view());
        }

        std::shared_ptr<graphicsGL::Framebuffer> const &shadowsFramebuffer() {return m_shadowsFramebuffer;}

        CommonObjectDataGL(renderDetails::ParametersObjectWithShadowsGL const &parameters, float aspectRatio)
                : renderDetails::CommonObjectDataPerspective(parameters, aspectRatio),
                  m_shadowsFramebuffer(parameters.shadowsFB)
        {}

        ~CommonObjectDataGL() override = default;
    private:
        std::shared_ptr<graphicsGL::Framebuffer> m_shadowsFramebuffer;
    };

    class DrawObjectDataGL : public renderDetails::DrawObjectDataGL {
    public:
        glm::mat4 modelMatrix(uint32_t) override {
            return m_modelMatrix;
        }

        void update(glm::mat4 const &modelMatrix) override {
            m_modelMatrix = modelMatrix;
        }

        DrawObjectDataGL(
                glm::mat4 inModelMatrix)
                : renderDetails::DrawObjectDataGL{},
                  m_modelMatrix{std::move(inModelMatrix)}
        {}

        ~DrawObjectDataGL() override = default;
    private:
        glm::mat4 m_modelMatrix;
    };

    class RenderDetailsGL : public renderDetails::RenderDetailsGL {
    public:
        std::string nameString() override { return m_renderDetailsName; }
        static renderDetails::ReferenceGL loadNew(
                char const *name,
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
                std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
                std::shared_ptr<levelDrawer::DrawObjectTableGL> const &drawObjTable,
                std::set<levelDrawer::ZValueReference>::iterator beginZValRefs,
                std::set<levelDrawer::ZValueReference>::iterator endZValRefs) override;

        RenderDetailsGL(
                char const *name,
                char const *vertShader,
                char const *textureFragShader,
                char const *colorFragShader,
                std::shared_ptr<GameRequester> const &inGameRequester,
                uint32_t inWidth,
                uint32_t inHeight,
                bool usesIntSurface);

        ~RenderDetailsGL() override = default;

    private:
        char const *m_renderDetailsName;
        std::shared_ptr<renderDetails::GLProgram> m_textureProgram;
        std::shared_ptr<renderDetails::GLProgram> m_colorProgram;

        static renderDetails::ReferenceGL createReference(
                std::shared_ptr<renderDetails::RenderDetailsGL> rd,
                std::shared_ptr<CommonObjectDataGL> cod);

    };
}

#endif // AMAZING_LABYRINTH_OBJECTWITHSHADOWS_RENDER_DETAILS_GL_HPP
