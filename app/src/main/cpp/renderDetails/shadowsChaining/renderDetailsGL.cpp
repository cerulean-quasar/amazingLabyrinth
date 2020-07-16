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
            renderDetails::ParametersGL const &parameters,
            Config const &config)
    {
        auto rd = std::make_shared<RenderDetailsGL>(
                parameters.useIntTexture, parameters.width, parameters.height);

        createFramebuffer(rd.get(), parameters);

        auto refShadows = renderLoader->load(
                gameRequester, shadows::RenderDetailsGL::name(), parameters);

        renderDetails::ParametersWithShadowsGL parametersWithShadows = {};
        parametersWithShadows.width = parameters.width;
        parametersWithShadows.height = parameters.height;
        parametersWithShadows.useIntTexture = parameters.useIntTexture;
        parametersWithShadows.shadowsFB = rd->m_framebufferShadows;

        auto refObjectWithShadows = renderLoader->load(
                gameRequester, objectWithShadows::RenderDetailsGL::name(), parametersWithShadows);

        rd->m_shadowsRenderDetails = refShadows.renderDetails;
        rd->m_objectWithShadowsRenderDetails = refObjectWithShadows.renderDetails;

        return createReference(std::move(rd), refObjectWithShadows, refShadows);
    }

    renderDetails::ReferenceGL RenderDetailsGL::loadExisting(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &renderLoader,
            std::shared_ptr<renderDetails::RenderDetailsGL> rdBase,
            renderDetails::ParametersGL const &parameters,
            Config const &config)
    {
        auto rd = dynamic_cast<RenderDetailsGL*>(rdBase.get());
        if (rd == nullptr) {
            throw std::runtime_error("Invalid render details type.");
        }

        if (parameters.useIntTexture != rd->m_useIntTexture) {
            rd->m_useIntTexture = parameters.useIntTexture;
            createFramebuffer(rd, parameters);
        }

        auto refShadows = renderLoader->load(
                gameRequester, shadows::RenderDetailsGL::name(), parameters);

        renderDetails::ParametersWithShadowsGL parametersWithShadows = {};
        parametersWithShadows.width = parameters.width;
        parametersWithShadows.height = parameters.height;
        parametersWithShadows.useIntTexture = parameters.useIntTexture;
        parametersWithShadows.shadowsFB = rd->m_framebufferShadows;

        auto refObjectWithShadows = renderLoader->load(
                gameRequester, objectWithShadows::RenderDetailsGL::name(), parametersWithShadows);

        rd->m_shadowsRenderDetails = refShadows.renderDetails;
        rd->m_objectWithShadowsRenderDetails = refObjectWithShadows.renderDetails;

        return createReference(std::move(rdBase), refObjectWithShadows, refShadows);
    }

    void RenderDetailsGL::createFramebuffer(
            RenderDetailsGL *rd,
            renderDetails::ParametersGL const &parameters)
    {
        std::vector<graphicsGL::Framebuffer::ColorImageFormat> colorImageFormats;

        if (rd->m_useIntTexture) {
            colorImageFormats.emplace_back(GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT);
        } else {
            colorImageFormats.emplace_back(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
        }

        rd->m_framebufferShadows = std::make_shared<graphicsGL::Framebuffer>(
                rd->m_surfaceWidth, rd->m_surfaceHeight, colorImageFormats);
    }

    renderDetails::ReferenceGL RenderDetailsGL::createReference(
            std::shared_ptr<renderDetails::RenderDetailsGL> rd,
            renderDetails::ReferenceGL const &refObjectWithShadows,
            renderDetails::ReferenceGL const &refShadows)
    {
        renderDetails::ReferenceGL ref = {};
        auto cod = std::make_shared<CommonObjectDataGL>(refObjectWithShadows.commonObjectData,
                                                        refShadows.commonObjectData);
        ref.createDrawObjectData = renderDetails::ReferenceGL::CreateDrawObjectData{
                [createDODObjectWithShadows(refObjectWithShadows.createDrawObjectData),
                        createDODShadows(refShadows.createDrawObjectData)] (
                        std::shared_ptr<renderDetails::DrawObjectDataGL> sharingDOD,
                        std::shared_ptr<levelDrawer::TextureData> textureData,
                        glm::mat4 modelMatrix) ->
                        std::shared_ptr<renderDetails::DrawObjectData>
                {
                    auto dodMain = createDODObjectWithShadows(sharingDOD, std::move(textureData),
                                                              std::move(modelMatrix));
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


        return std::move(ref);
    }

    void RenderDetailsGL::draw(
            uint32_t /* unused model matrix ID */,
            renderDetails::DrawTypes<levelDrawer::DrawObjectTableGL>::CommonObjectDataList const &commonObjectDataList,
            renderDetails::DrawTypes<levelDrawer::DrawObjectTableGL>::DrawObjectTableList const &drawObjTableList,
            renderDetails::DrawTypes<levelDrawer::DrawObjectTableGL>::IndicesForDrawList const &drawObjectsIndicesList)
    {
        // only do shadows for the level itself
        renderDetails::DrawTypes<levelDrawer::DrawObjectTableGL>::DrawObjectTableList shadowsDrawObjTableList =
                { nullptr, drawObjTableList[levelDrawer::LevelDrawer::ObjectType::LEVEL], nullptr };
        renderDetails::DrawTypes<levelDrawer::DrawObjectTableGL>::IndicesForDrawList shadowsDrawObjectsIndicesList =
                { std::vector<size_t>{},
                  drawObjectsIndicesList[levelDrawer::LevelDrawer::ObjectType::LEVEL],
                  std::vector<size_t>{} };

        // get the shadows common object data
        auto codLevel = dynamic_cast<CommonObjectDataGL*>(
                commonObjectDataList[levelDrawer::LevelDrawer::ObjectType::LEVEL].get());
        renderDetails::DrawTypes<levelDrawer::DrawObjectTableGL>::CommonObjectDataList shadowsCODList = {
                nullptr, codLevel->m_shadowsCOD, nullptr };

        glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferShadows->fbo());
        checkGraphicsError();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepthf(1.0f);
        checkGraphicsError();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        checkGraphicsError();

        m_shadowsRenderDetails->draw(MODEL_MATRIX_ID_SHADOWS,
                shadowsCODList, shadowsDrawObjTableList, shadowsDrawObjectsIndicesList);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        checkGraphicsError();

        // get the shadows common object data
        auto codStarter = dynamic_cast<CommonObjectDataGL*>(
                commonObjectDataList[levelDrawer::LevelDrawer::ObjectType::STARTER].get());
        auto codFinisher = dynamic_cast<CommonObjectDataGL*>(
                commonObjectDataList[levelDrawer::LevelDrawer::ObjectType::FINISHER].get());
        renderDetails::DrawTypes<levelDrawer::DrawObjectTableGL>::CommonObjectDataList mainCODList = {
                codStarter->m_objectWithShadowsCOD, codLevel->m_objectWithShadowsCOD, codFinisher->m_objectWithShadowsCOD };

        m_objectWithShadowsRenderDetails->draw(MODEL_MATRIX_ID_MAIN,
                mainCODList, drawObjTableList, drawObjectsIndicesList);
    }
}