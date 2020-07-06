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
#ifndef AMAZING_LABYRINTH_TEXTUREWITHSHADOWS_RENDER_DETAILS_DATA_GL_HPP
#define AMAZING_LABYRINTH_TEXTUREWITHSHADOWS_RENDER_DETAILS_DATA_GL_HPP

#include <memory>

#include <glm/glm.hpp>

#include <GLES3/gl3.h>

#include "../renderDetailsGL.hpp"
#include "../renderDetails.hpp"
#include "config.hpp"
#include "../../levelDrawer/textureTable/textureLoader.hpp"
#include "../shadows/renderDetailsDataGL.hpp"

namespace textureWithShadows {
    class RenderDetailsDataGL;
    class CommonObjectDataGL : public renderDetails::CommonObjectData {
        friend RenderDetailsDataGL;
    public:
        glm::mat4 getPerspectiveMatrixForLevel() override {
            /* perspective matrix: takes the perspective projection, the aspect ratio, near and far
             * view planes.
             */
            return getPerspectiveMatrix(m_viewAngle, m_aspectRatio, m_nearPlane, m_farPlane,
                                        false, false);
        }
    private:
        std::shared_ptr<shadows::CommonObjectDataGL> m_shadowsCOD;

        CommonObjectDataGL(
                std::shared_ptr<shadows::CommonObjectDataGL> shadowsCOD,
                float aspectRatio,
                Config const &config)
            : CommonObjectData(config.viewAngle, aspectRatio, config.nearPlane, config.farPlane,
                    config.viewPoint, config.lookAt, config.up),
            m_shadowsCOD(std::move(shadowsCOD))
        {}
    };

    class DrawObjectDataGL : public renderDetails::DrawObjectData {
        friend RenderDetailsDataGL;
    public:
        void update(glm::mat4 const &modelMatrix) override {
            m_modelMatrix = modelMatrix;
        }

        ~DrawObjectDataGL() override = default;
    private:
        DrawObjectDataGL(
                std::shared_ptr<shadows::DrawObjectDataGL> shadowsDOD,
                glm::mat4 const &inModelMatrix)
                : renderDetails::DrawObjectData{},
                  m_shadowsDOD{std::move(shadowsDOD)},
                  m_modelMatrix{inModelMatrix}
        {}

        std::shared_ptr<shadows::DrawObjectDataGL> m_shadowsDOD;
        glm::mat4 m_modelMatrix;
    };

    class RenderDetailsDataGL : public renderDetails::RenderDetailsData {
    public:
        std::shared_ptr<renderDetails::CommonObjectData> createCommonObjectData(
                Config const &config)
        {
            shadows::Config configShadows{};
            configShadows.viewAngle = config.viewAngle;
            configShadows.nearPlane = config.nearPlane;
            configShadows.farPlane = config.farPlane;
            configShadows.lightingSource = config.lightingSource;
            configShadows.lookAt = config.lookAt;
            configShadows.up = config.up;
            auto shadowsCOD = m_shadowsRenderDetails->createCommonObjectData(configShadows);
            return std::make_shared<CommonObjectDataGL>(
                    std::move(shadowsCOD),
                    m_surfaceWidth/static_cast<float>(m_surfaceHeight),
                    config);
        }

        std::shared_ptr<renderDetails::DrawObjectData> createDrawObjectData(
                std::shared_ptr<TextureData> &textureData,
                glm::mat4 const &modelMatrix) override
        {
            auto fakeTexture = std::shared_ptr<TextureData>();
            auto shadowsDOD = m_shadowsRenderDetails->createDrawObjectData(
                    fakeTexture, modelMatrix);
            return std::make_shared<DrawObjectDataGL>(shadowsDOD, modelMatrix);
        }

        RenderDetailsDataGL(std::shared_ptr<GameRequester> const &inGameRequester,
                            bool useIntTexture,
                            uint32_t inWidth, uint32_t inHeight);

        ~RenderDetailsDataGL() override = default;

    private:
        static char constexpr const *SHADER_VERT_FILE = "shaders/shaderGL.vert";
        static char constexpr const *SHADER_FRAG_FILE = "shaders/shaderGL.frag";

        bool m_useIntTexture;
        std::shared_ptr<renderDetails::Framebuffer> m_framebufferShadows;
        std::shared_ptr<shadows::RenderDetailsDataGL> m_shadowsRenderDetails;
        GLuint m_mainProgramID;
    };
}
#endif // AMAZING_LABYRINTH_TEXTUREWITHSHADOWS_RENDER_DETAILS_DATA_GL_HPP
