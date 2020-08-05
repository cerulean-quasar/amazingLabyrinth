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
            std::shared_ptr<renderDetails::Parameters> const &parametersBase,
            Config const &)
    {
        auto parameters = dynamic_cast<renderDetails::ParametersGL*>(parametersBase.get());
        if (parameters == nullptr) {
            throw std::runtime_error("Invalid render details parameter type.");
        }

        auto rd = std::make_shared<RenderDetailsGL>(
                parameters->useIntTexture, parameters->width, parameters->height);

        createFramebuffer(rd.get());

        auto refShadows = renderLoader->load(
                gameRequester, shadows::RenderDetailsGL::name(), parametersBase);

        renderDetails::ParametersWithShadowsGL parametersWithShadows = {};
        parametersWithShadows.width = parameters->width;
        parametersWithShadows.height = parameters->height;
        parametersWithShadows.useIntTexture = parameters->useIntTexture;
        parametersWithShadows.shadowsFB = rd->m_framebufferShadows;

        auto refObjectWithShadows = renderLoader->load(
                gameRequester, objectWithShadows::RenderDetailsGL::name(),
                std::make_shared<renderDetails::ParametersWithShadowsGL>(parametersWithShadows));

        rd->m_shadowsRenderDetails = refShadows.renderDetails;
        rd->m_objectWithShadowsRenderDetails = refObjectWithShadows.renderDetails;

        return createReference(std::move(rd), refObjectWithShadows, refShadows);
    }

    renderDetails::ReferenceGL RenderDetailsGL::loadExisting(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &renderLoader,
            std::shared_ptr<renderDetails::RenderDetailsGL> rdBase,
            std::shared_ptr<renderDetails::Parameters> const &parametersBase,
            Config const &)
    {
        auto parameters = dynamic_cast<renderDetails::ParametersGL*>(parametersBase.get());
        if (parameters == nullptr) {
            throw std::runtime_error("Invalid render details parameter type.");
        }

        auto rd = dynamic_cast<RenderDetailsGL*>(rdBase.get());
        if (rd == nullptr) {
            throw std::runtime_error("Invalid render details type.");
        }

        if (parameters->useIntTexture != rd->m_useIntTexture) {
            rd->m_useIntTexture = parameters->useIntTexture;
            createFramebuffer(rd);
        }

        auto refShadows = renderLoader->load(
                gameRequester, shadows::RenderDetailsGL::name(), parametersBase);

        renderDetails::ParametersWithShadowsGL parametersWithShadows = {};
        parametersWithShadows.width = parameters->width;
        parametersWithShadows.height = parameters->height;
        parametersWithShadows.useIntTexture = parameters->useIntTexture;
        parametersWithShadows.shadowsFB = rd->m_framebufferShadows;

        auto refObjectWithShadows = renderLoader->load(
                gameRequester, objectWithShadows::RenderDetailsGL::name(),
                std::make_shared<renderDetails::ParametersWithShadowsGL>(parametersWithShadows));

        rd->m_shadowsRenderDetails = refShadows.renderDetails;
        rd->m_objectWithShadowsRenderDetails = refObjectWithShadows.renderDetails;

        return createReference(std::move(rdBase), refObjectWithShadows, refShadows);
    }

    void RenderDetailsGL::createFramebuffer(
            RenderDetailsGL *rd)
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
            std::set<levelDrawer::ZValueReference> const &starterZValues,
            std::set<levelDrawer::ZValueReference> const &levelZValues,
            std::set<levelDrawer::ZValueReference> const &finisherZValues)
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
                levelZValues);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        checkGraphicsError();
    }

    void RenderDetailsGL::draw(
            uint32_t /* unused model matrix ID */,
            std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
            std::shared_ptr<renderDetails::DrawObjectTableGL> const &drawObjTable,
            std::vector<renderDetails::DrawObjReference> const &drawObjectsIndices)
    {
        // get the shadows common object data
        auto cod = dynamic_cast<CommonObjectDataGL*>(commonObjectData.get());

        m_objectWithShadowsRenderDetails->draw(MODEL_MATRIX_ID_MAIN,
                cod->objectWithShadowsCOD(), drawObjTable, drawObjectsIndices);
    }

    RegisterGL<renderDetails::RenderDetailsGL, RenderDetailsGL, Config> registerGL;
}
