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

#include "renderDetailsGL.hpp"

namespace shadowsChaining {
    renderDetails::ReferenceGL RenderDetailsGL::loadNew(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &renderLoader,
            std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &,
            Config const &)
    {
        auto rd = std::make_shared<RenderDetailsGL>(
                surfaceDetails->useIntTexture, surfaceDetails->surfaceWidth, surfaceDetails->surfaceHeight);

        auto refShadows = renderLoader->load(
                gameRequester, shadows::RenderDetailsGL::name(), surfaceDetails, nullptr);

        renderDetails::ParametersWithShadowsGL parametersWithShadows = {};
        parametersWithShadows.shadowsFB = rd->m_framebufferShadows;

        auto refObjectWithShadows = renderLoader->load(
                gameRequester, objectWithShadows::RenderDetailsGL::name(), surfaceDetails,
                std::make_shared<renderDetails::ParametersWithShadowsGL>(parametersWithShadows));

        rd->m_shadowsRenderDetails = refShadows.renderDetails;
        rd->m_objectWithShadowsRenderDetails = refObjectWithShadows.renderDetails;

        return createReference(std::move(rd), refObjectWithShadows, refShadows);
    }

    renderDetails::ReferenceGL RenderDetailsGL::loadExisting(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &renderLoader,
            std::shared_ptr<renderDetails::RenderDetailsGL> rdBase,
            std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &,
            Config const &)
    {
        auto rd = dynamic_cast<RenderDetailsGL*>(rdBase.get());
        if (rd == nullptr) {
            throw std::runtime_error("Invalid render details type.");
        }

        if (surfaceDetails->useIntTexture != rd->m_useIntTexture ||
            rd->m_surfaceWidth != surfaceDetails->surfaceWidth ||
            rd->m_surfaceHeight != surfaceDetails->surfaceHeight)
        {
            rd->m_surfaceWidth = surfaceDetails->surfaceWidth;
            rd->m_surfaceHeight = surfaceDetails->surfaceHeight;
            rd->m_useIntTexture = surfaceDetails->useIntTexture;
            rd->createFramebuffer();
        }

        auto refShadows = renderLoader->load(
                gameRequester, shadows::RenderDetailsGL::name(), surfaceDetails, nullptr);

        renderDetails::ParametersWithShadowsGL parametersWithShadows = {};
        parametersWithShadows.shadowsFB = rd->m_framebufferShadows;

        auto refObjectWithShadows = renderLoader->load(
                gameRequester, objectWithShadows::RenderDetailsGL::name(), surfaceDetails,
                std::make_shared<renderDetails::ParametersWithShadowsGL>(parametersWithShadows));

        rd->m_shadowsRenderDetails = refShadows.renderDetails;
        rd->m_objectWithShadowsRenderDetails = refObjectWithShadows.renderDetails;

        return createReference(std::move(rdBase), refObjectWithShadows, refShadows);
    }

    void RenderDetailsGL::createFramebuffer()
    {
        std::vector<graphicsGL::Framebuffer::ColorImageFormat> colorImageFormats;

        if (m_useIntTexture) {
            colorImageFormats.emplace_back(GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT);
        } else {
            colorImageFormats.emplace_back(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
        }

        m_framebufferShadows = std::make_shared<graphicsGL::Framebuffer>(
                m_surfaceWidth, m_surfaceHeight, colorImageFormats);
    }

    renderDetails::ReferenceGL RenderDetailsGL::createReference(
            std::shared_ptr<renderDetails::RenderDetailsGL> rd,
            renderDetails::ReferenceGL const &refObjectWithShadows,
            renderDetails::ReferenceGL const &refShadows)
    {
        renderDetails::ReferenceGL ref = {};
        auto cod = std::make_shared<CommonObjectDataGL>(refObjectWithShadows.commonObjectData,
                                                        refShadows.commonObjectData);
        ref.renderDetails = std::move(rd);
        ref.createDrawObjectData = renderDetails::ReferenceGL::CreateDrawObjectData{
                [createDODObjectWithShadows(refObjectWithShadows.createDrawObjectData),
                        createDODShadows(refShadows.createDrawObjectData)] (
                        std::shared_ptr<renderDetails::DrawObjectDataGL> const &sharingDOD,
                        std::shared_ptr<levelDrawer::TextureDataGL> const &textureData,
                        glm::mat4 const &modelMatrix) ->
                        std::shared_ptr<renderDetails::DrawObjectDataGL>
                {
                    auto dodMain = createDODObjectWithShadows(sharingDOD, textureData, modelMatrix);
                    auto dodShadows = createDODShadows(sharingDOD, nullptr, modelMatrix);

                    return std::make_shared<DrawObjectDataGL>(std::move(dodMain), std::move(dodShadows));
                }
        };

        ref.getProjViewForLevel = renderDetails::ReferenceGL::GetProjViewForLevel(
                [getPVObjectWithShadows(refObjectWithShadows.getProjViewForLevel)]() ->
                        std::pair<glm::mat4, glm::mat4>
                {
                    return getPVObjectWithShadows();
                });

        ref.commonObjectData = std::move(cod);

        return std::move(ref);
    }

    void RenderDetailsGL::preMainDraw(
            uint32_t /* unused matrix ID */,
            levelDrawer::CommonObjectDataList const &commonObjectDataList,
            levelDrawer::DrawObjectTableGList const &drawObjTableList,
            std::set<levelDrawer::ZValueReference> const &,
            std::set<levelDrawer::ZValueReference> const &levelZValues,
            std::set<levelDrawer::ZValueReference> const &)
    {
        // get the shadows common object data
        auto codLevel = dynamic_cast<CommonObjectDataGL*>(
                commonObjectDataList[levelDrawer::ObjectType::LEVEL].get());

        glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferShadows->fbo());
        checkGraphicsError();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepthf(1.0f);
        checkGraphicsError();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        checkGraphicsError();

        m_shadowsRenderDetails->draw(
                renderDetails::MODEL_MATRIX_ID_SHADOWS,
                codLevel->shadowsCOD(),
                drawObjTableList[levelDrawer::ObjectType::LEVEL],
                levelZValues.begin(), levelZValues.end());

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        checkGraphicsError();
    }

    void RenderDetailsGL::draw(
            uint32_t /* unused model matrix ID */,
            std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
            std::shared_ptr<levelDrawer::DrawObjectTableGL> const &drawObjTable,
            std::set<levelDrawer::ZValueReference>::iterator beginZValRefs,
            std::set<levelDrawer::ZValueReference>::iterator endZValRefs)
    {
        // get the shadows common object data
        auto cod = dynamic_cast<CommonObjectDataGL*>(commonObjectData.get());

        m_objectWithShadowsRenderDetails->draw(renderDetails::MODEL_MATRIX_ID_MAIN,
                cod->objectWithShadowsCOD(), drawObjTable, beginZValRefs, endZValRefs);
    }

    RegisterGL<renderDetails::RenderDetailsGL, RenderDetailsGL, Config> registerGL;
}
