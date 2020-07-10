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
#include <vector>

#include <glm/glm.hpp>

#include <GLES3/gl3.h>

#include "../renderDetailsGL.hpp"
#include "../renderDetails.hpp"
#include "config.hpp"
#include "../shadows/renderDetailsGL.hpp"
#include "../textureWithShadows/renderDetailsGL.hpp"
#include "../../graphicsGL.hpp"
#include "../../renderLoader/renderLoaderGL.hpp"

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
            m_mainDOD->update(modelMatrix);
            m_shadowsDOD->update(modelMatrix);
        }

        ~DrawObjectDataGL() override = default;
    private:
        DrawObjectDataGL(
                std::shared_ptr<renderDetails::DrawObjectDataGL> mainDOD,
                std::shared_ptr<shadows::DrawObjectDataGL> shadowsDOD)
                : renderDetails::DrawObjectData{},
                  m_mainDOD{std::move(mainDOD)},
                  m_shadowsDOD{std::move(shadowsDOD)}
        {}

        std::shared_ptr<renderDetails::DrawObjectDataGL> m_mainDOD;
        std::shared_ptr<shadows::DrawObjectDataGL> m_shadowsDOD;
    };

    class RenderDetailsGL : public renderDetails::RenderDetailsGL {
    public:
        renderDetails::ReferenceGL loadNew(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<RenderLoaderGL> const &renderLoader,
                renderDetails::ParametersGL const &parameters,
                Config const &config)
        {
            auto rd = std::make_shared<RenderDetailsGL>(
                    parameters.useIntTexture, parameters.width, parameters.height);

            std::vector<graphicsGL::Framebuffer::ColorImageFormat> colorImageFormats;
            // for shadow mapping.
            if (m_useIntTexture) {
                colorImageFormats.emplace_back(GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT);
            } else {
                colorImageFormats.emplace_back(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
            }
            m_framebufferShadows = std::make_shared<graphicsGL::Framebuffer>(
                    m_surfaceWidth, m_surfaceHeight, colorImageFormats);

            auto refShadows = renderLoader->load(
                    gameRequester, renderLoader, shadows::RenderDetailsGL::name(), parameters);

            renderDetails::ParametersWithShadowsGL parametersWithShadows = {};
            parametersWithShadows.width = parameters.width;
            parametersWithShadows.height = parameters.height;
            parametersWithShadows.useIntTexture = parameters.useIntTexture;
            parametersWithShadows.shadowsFB = rd->m_framebufferShadows;

            auto refTexture = renderLoader->load(
                    gameRequester, renderLoader,
                    textureWithShadows::RenderDetailsGL::name(), parametersWithShadows);

            auto refColor = renderLoader->load(
                    gameRequester, renderLoader,
                    colorWithShadows::RenderDetailsGL::name(), parametersWithShadows);

            rd->m_shadowsRenderDetails = refShadows.renderDetails;
            rd->m_textureRenderDetails = refTexture.renderDetails;
            rd->m_colorRenderDetails = refColor.renderDetails;

            return createReference(std::move(rd), refTexture, refColor, refShadows);
        }

        renderDetails::RenderDetailsGL loadExisting(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<RenderLoaderGL> const &renderLoader,
                std::shared_ptr<renderDetails::RenderDetailsGL> rdBase,
                renderDetails::ParametersGL const &parameters,
                Config const &config)
        {
            auto rd = dynamic_cast<RenderDetailsGL*>(rdBase.get());
            if (rd == nullptr) {
                throw std::runtime_error("Invalid render details type.")
            }

            auto refShadows = renderLoader->load(
                    gameRequester, renderLoader, shadows::RenderDetailsGL::name(), parameters);

            renderDetails::ParametersWithShadowsGL parametersWithShadows = {};
            parametersWithShadows.width = parameters.width;
            parametersWithShadows.height = parameters.height;
            parametersWithShadows.useIntTexture = parameters.useIntTexture;
            parametersWithShadows.shadowsFB = rd->m_framebufferShadows;

            auto refTexture = renderLoader->load(
                    gameRequester, renderLoader, textureWithShadows::RenderDetailsGL::name(),
                    parametersWithShadows);

            auto refColor = renderLoader->load(
                    gameRequester, renderLoader, colorWithShadows::RenderDetailsGL::name(),
                    parametersWithShadows);

            rd->m_shadowsRenderDetails = refShadows.renderDetails;
            rd->m_textureRenderDetails = refTexture.renderDetails;
            rd->m_colorRenderDetails = refColor.renderDetails;

            return createReference(std::move(rd), refTexture, refColor, refShadows);
        }

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
        bool m_useIntTexture;
        std::shared_ptr<graphicsGL::Framebuffer> m_framebufferShadows;
        std::shared_ptr<shadows::RenderDetailsGL> m_shadowsRenderDetails;
        std::shared_ptr<textureWithShadows::RenderDetailsGL> m_textureRenderDetails;
        std::shared_ptr<colorWithShadows::RenderDetailsGL> m_colorRenderDetails;

        renderDetails::ReferenceGL createReference(
                std::shared_ptr<renderDetails::RenderDetailsGL> rd,
                renderDetails::ReferenceGL const &refTexture,
                renderDetails::ReferenceGL const &refColor,
                renderDetails::ReferenceGL const &refShadows)
        {
            renderDetails::ReferenceGL ref = {};
            auto cod = std::make_shared<CommonObjectDataGL>(refTexture.commonObjectData,
                    refColor.commonObjectData, refShadows.commonObjectData);
            ref.createDrawObjectData = renderDetails::ReferenceGL::CreateDrawObjectData{
                [createDODTexture(refTexture.createDrawObjectData),
                 createDODColor(refColor.createDrawObjectData),
                 createDODShadows(refShadows.createDrawObjectData)] (
                         std::shared_ptr<levelDrawer::TextureData> textureData,
                         glm::mat4 modelMatrix) ->
                         std::shared_ptr<renderDetails::DrawObjectData>
                {
                    std::shared_ptr<renderDetails::DrawObjectData> dodMain;

                    if (textureData) {
                        dodMain = createDODTexture(std::move(textureData), std::move(modelMatrix));
                    } else {
                        dodMain = createDODColor(std::move(textureData), std::move(modelMatrix));
                    }

                    auto dodShadows = createDODShadows(std::shared_ptr<levelDrawer::TextureData>(),
                            modelMatrix);

                    return std::make_shared<DrawObjectDataGL>(dodMain, dodShadows);
                }
            };

        }

        RenderDetailsGL(bool useIntTexture, uint32_t inWidth, uint32_t inHeight)
                : renderDetails::RenderDetailsGL{inWidth, inHeight},
                m_useIntTexture(useIntTexture)
        {}
    };
}
#endif // AMAZING_LABYRINTH_TEXTUREWITHSHADOWS_RENDER_DETAILS_DATA_GL_HPP
