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

#include "renderDetailsGL.hpp"

namespace darkChaining {
    renderDetails::ReferenceGL RenderDetailsGL::loadNew(
            renderDetails::Description const &description,
            std::vector<char const *> const &,
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &renderLoader,
            std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &parametersBase)
    {
        auto rd = std::make_shared<RenderDetailsGL>(description,
                surfaceDetails->useIntTexture, surfaceDetails->surfaceWidth,
                surfaceDetails->surfaceHeight);

        return loadHelper(gameRequester, renderLoader, rd, surfaceDetails, parametersBase);
    }

    renderDetails::ReferenceGL RenderDetailsGL::loadExisting(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &renderLoader,
            std::shared_ptr<renderDetails::RenderDetailsGL> rdBase,
            std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &parametersBase)
    {
        auto rd = std::dynamic_pointer_cast<RenderDetailsGL>(rdBase);
        if (rd == nullptr) {
            throw std::runtime_error("Invalid render details type.");
        }

        if (surfaceDetails->useIntTexture != rd->m_usesIntSurface ||
            rd->m_surfaceWidth != surfaceDetails->surfaceWidth ||
            rd->m_surfaceHeight != surfaceDetails->surfaceHeight)
        {
            rd->m_surfaceWidth = surfaceDetails->surfaceWidth;
            rd->m_surfaceHeight = surfaceDetails->surfaceHeight;
            rd->m_usesIntSurface = surfaceDetails->useIntTexture;
            rd->createFramebuffers();
        }

        return loadHelper(gameRequester, renderLoader, rd, surfaceDetails, parametersBase);
    }

    renderDetails::ReferenceGL RenderDetailsGL::loadHelper(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &renderLoader,
            std::shared_ptr<RenderDetailsGL> rd,
            std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &parametersBase)
    {
        auto parameters = dynamic_cast<renderDetails::ParametersPerspective*>(parametersBase.get());
        if (parameters == nullptr) {
            throw std::runtime_error("Invalid render details parameter type.");
        }

        renderDetails::ReferenceGL refShadows;
        std::array<std::shared_ptr<renderDetails::CommonObjectData>, numberShadowMaps> shadowsCODs = {};
        auto parametersShadows = std::make_shared<renderDetails::ParametersPerspective>();

        auto shadowSurfaceDetails = createShadowSurfaceDetails(surfaceDetails);

        renderDetails::Query shadowsRenderDetailsQuery{
            renderDetails::DrawingStyle::shadowMap,
            std::vector<renderDetails::Features>{},
            std::vector<renderDetails::Features>{}};
        for (size_t i = 0; i < numberShadowMaps; i++) {
            renderDetails::darkInitializeShadowMapParameters(*parametersShadows, *parameters, i, i==0);
            // We have to load the shadows render details multiple times to get the COD, but it is
            // not a performance problem, because after the render details is loaded the first time,
            // the next times, it is just looked up in a list, not really any work is done other than
            // creating the COD.
            refShadows = renderLoader->load(
                    gameRequester,
                    shadowsRenderDetailsQuery,
                    shadowSurfaceDetails,
                    parametersShadows);

            shadowsCODs[i] = refShadows.commonObjectData;
        }

        auto parms = std::make_shared<renderDetails::ParametersDarkObjectGL>(*parameters,
                                                                             rd->m_framebuffersShadows);

        auto const &description = rd->description();

        renderDetails::FeatureList featuresDarkObject = description.features();
        featuresDarkObject.setFeature(renderDetails::Features::chaining, false);

        auto refDarkObject = renderLoader->load(
                gameRequester,
                {description.drawingMethod(),
                    featuresDarkObject, renderDetails::FeatureList()},
                surfaceDetails,
                parms);

        rd->m_darkObjectRenderDetails = refDarkObject.renderDetails;
        rd->m_shadowsRenderDetails = refShadows.renderDetails;

        return createReference(std::move(rd), refDarkObject, refShadows, shadowsCODs);
    }

    void RenderDetailsGL::createFramebuffers()
    {
        std::vector<graphicsGL::Framebuffer::ColorImageFormat> colorImageFormats;

        if (m_usesIntSurface) {
            colorImageFormats.emplace_back(GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT);
        } else {
            colorImageFormats.emplace_back(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
        }

        for (auto &framebuffer : m_framebuffersShadows) {
            framebuffer = std::make_shared<graphicsGL::Framebuffer>(
                    m_surfaceWidth, m_surfaceHeight, colorImageFormats);
        }
    }

    renderDetails::ReferenceGL RenderDetailsGL::createReference(
            std::shared_ptr<renderDetails::RenderDetailsGL> rd,
            renderDetails::ReferenceGL const &refDarkObject,
            renderDetails::ReferenceGL const &refShadows,
            std::array<std::shared_ptr<renderDetails::CommonObjectData>, numberShadowMaps> shadowsCODs)
    {
        renderDetails::ReferenceGL ref = {};
        auto cod = std::make_shared<CommonObjectDataGL>(refDarkObject.commonObjectData,
                                                        shadowsCODs);
        ref.renderDetails = std::move(rd);
        ref.createDrawObjectData = renderDetails::ReferenceGL::CreateDrawObjectData{
                [createDODDarkObject(refDarkObject.createDrawObjectData),
                        createDODShadows(refShadows.createDrawObjectData)] (
                        std::shared_ptr<renderDetails::DrawObjectDataGL> const &sharingDOD,
                        std::shared_ptr<levelDrawer::TextureDataGL> const &textureData,
                        glm::mat4 const &modelMatrix) ->
                        std::shared_ptr<renderDetails::DrawObjectDataGL>
                {
                    auto dodMain = createDODDarkObject(sharingDOD, textureData, modelMatrix);
                    auto dodShadows = createDODShadows(dodMain, nullptr, modelMatrix);

                    return std::make_shared<DrawObjectDataGL>(std::move(dodMain), std::move(dodShadows));
                }
        };

        ref.getProjViewForLevel = renderDetails::ReferenceGL::GetProjViewForLevel(
                [getPVObjectWithShadows(refDarkObject.getProjViewForLevel)]() ->
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

        for (size_t i = 0; i < numberShadowMaps; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffersShadows[i]->fbo());
            checkGraphicsError();

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClearDepthf(1.0f);
            checkGraphicsError();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            checkGraphicsError();

            m_shadowsRenderDetails->draw(
                    renderDetails::MODEL_MATRIX_ID_SHADOWS,
                    codLevel->shadowsCOD(i),
                    drawObjTableList[levelDrawer::ObjectType::LEVEL],
                    levelZValues.begin(), levelZValues.end());

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            checkGraphicsError();
        }
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
        if (!cod) {
            throw std::runtime_error("Invalid common object data type");
        }

        m_darkObjectRenderDetails->draw(renderDetails::MODEL_MATRIX_ID_MAIN,
                cod->darkObject(), drawObjTable, beginZValRefs, endZValRefs);
    }

    RegisterGL<renderDetails::RenderDetailsGL, RenderDetailsGL> registerTextureGL(
            renderDetails::Description{renderDetails::DrawingStyle::dark,
                                       {renderDetails::Features::chaining,
                                                 renderDetails::Features::color,
                                                 renderDetails::Features::texture}},
            std::vector<char const *>{});
}
