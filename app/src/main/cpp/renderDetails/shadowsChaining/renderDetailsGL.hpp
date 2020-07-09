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
#include "../shadows/renderDetailsGL.hpp"
#include "../textureWithShadows/renderDetailsGL.hpp"
#include "../../graphicsGL.hpp"

namespace shadowsChaining {
    class RenderDetailsGL;
    class CommonObjectDataGL : public renderDetails::CommonObjectData {
        friend RenderDetailsGL;
    private:
        std::shared_ptr<textureWithShadows::CommonObjectDataGL> m_textureCOD;
        std::shared_ptr<colorWithShadows::CommonObjectDataGL> m_colorCOD;
        std::shared_ptr<shadows::CommonObjectDataGL> m_shadowsCOD;

        CommonObjectDataGL(
                std::shared_ptr<textureWithShadows::CommonObjectDataGL> inTextureCOD,
                std::shared_ptr<colorWithShadows::CommonObjectDataGL> inColorCOD,
                std::shared_ptr<shadows::CommonObjectDataGL> inShadowsCOD)
                : CommonObjectData(),
                  m_textureCOD(std::move(inTextureCOD)),
                  m_colorCOD(std::move(inColorCOD)),
                  m_shadowsCOD(std::move(inShadowsCOD))
        {}
    };

    class DrawObjectDataGL : public renderDetails::DrawObjectData {
        friend RenderDetailsGL;
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

    class RenderDetailsGL : public renderDetails::RenderDetailsGL {
    public:
        std::shared_ptr<renderDetails::DrawObjectData> createDrawObjectData(
                std::shared_ptr<levelDrawer::TextureData> &textureData,
                glm::mat4 const &modelMatrix) override
        {
            auto fakeTexture = std::shared_ptr<levelDrawer::TextureData>();
            auto shadowsDOD = m_shadowsRenderDetails->createDrawObjectData(
                    fakeTexture, modelMatrix);
            return std::make_shared<DrawObjectDataGL>(shadowsDOD, modelMatrix);
        }

        ~RenderDetailsGL() override = default;

    private:
        std::shared_ptr<graphicsGL::Framebuffer> m_framebufferShadows;
        std::shared_ptr<shadows::RenderDetailsGL> m_shadowsRenderDetails;
        std::shared_ptr<textureWithShadows::RenderDetailsGL> m_textureRenderDetails;
        std::shared_ptr<colorWithShadows::RenderDetailsGL> m_colorRenderDetails;

        RenderDetailsGL(std::shared_ptr<GameRequester> const &inGameRequester,
                        uint32_t inWidth, uint32_t inHeight)
                : renderDetails::RenderDetailsGL{inWidth, inHeight}
        {}
    };
}
#endif // AMAZING_LABYRINTH_TEXTUREWITHSHADOWS_RENDER_DETAILS_DATA_GL_HPP
