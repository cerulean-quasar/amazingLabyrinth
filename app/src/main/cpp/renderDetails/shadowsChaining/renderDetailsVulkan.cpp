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
#include <memory>

#include "renderDetailsVulkan.hpp"

namespace shadowsChaining {
    renderDetails::ReferenceVulkan RenderDetailsVulkan::loadNew(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
            std::shared_ptr<vulkan::Device> const &inDevice,
            renderDetails::ParametersVulkan const &parameters,
            Config const &config)
    {
            // initialize main render details
            auto rd = std::make_shared<RenderDetailsVulkan>(parameters.width, parameters.height);
            rd->m_device = inDevice;

            // shadow resources
            rd->m_depthImageViewShadows = std::make_shared<vulkan::ImageView>(
                    vulkan::ImageFactory::createDepthImage(
                            inDevice,
                            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                            getShadowsFramebufferWidth(parameters.width),
                            getShadowsFramebufferHeigth(parameters.height)),
                    VK_IMAGE_ASPECT_DEPTH_BIT);
            rd->m_shadowsColorAttachment = vulkan::ImageView::createImageViewAndImage(
                    inDevice,
                    getShadowsFramebufferWidth(parameters.width),
                    getShadowsFramebufferHeigth(parameters.height),
                    VK_FORMAT_R32G32B32A32_SFLOAT,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    VK_IMAGE_ASPECT_COLOR_BIT);
            rd->m_samplerShadows = std::make_shared<vulkan::ImageSampler>(
                    inDevice, rd->m_shadowsColorAttachment,
                    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                    VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);

            auto shadowParameters = createShadowParameters(rd, parameters);

            // shadows render details
            auto refShadows = renderLoader->load(
                    gameRequester, shadows::RenderDetailsVulkan::name(), shadowParameters);

            // shadows framebuffer
            rd->m_framebufferShadows = std::make_shared<vulkan::Framebuffer>(
                    inDevice, refShadows.renderDetails->renderPass(),
                    std::vector<std::shared_ptr<vulkan::ImageView>>{rd->m_shadowsColorAttachment,
                                                                    rd->m_depthImageViewShadows},
                    shadowParameters.width, shadowParameters.height);

            // texture render details
            auto refTexture = renderLoader->load(
                    gameRequester, textureWithShadows::RenderDetailsVulkan::name(), parameters);

            // color render details
            auto refColor = renderLoader->load(
                    gameRequester, colorWithShadows::RenderDetailsVulkan::name(), parameters);

            rd->m_textureRenderDetails = refTexture.renderDetails;
            rd->m_colorRenderDetails = refColor.renderDetails;
            rd->m_shadowsRenderDetails = refShadows.renderDetails;

            return createReference(std::move(rd), refShadows, refTexture, refColor);
    }

    renderDetails::ReferenceVulkan RenderDetailsVulkan::loadExisting(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
            std::shared_ptr<renderDetails::RenderDetailsVulkan> const &rdBase,
            renderDetails::ParametersVulkan const &parameters,
            Config const &config)
    {
        auto rd = dynamic_cast<RenderDetailsVulkan*>(rdBase.get());
        if (rd == nullptr) {
            throw std::runtime_error("Invalid render details type.")
        }
        auto shadowParameters = createShadowParameters(rd, parameters);

        // shadows render details
        auto refShadows = renderLoader->load(
            gameRequester, renderLoader, shadows::RenderDetailsVulkan::name(), shadowParameters);

        // object with shadows render details
        auto refObjectWithShadows = renderLoader->load(
            gameRequester, renderLoader, objectWithShadows::RenderDetailsVulkan::name(), parameters);

        rd->m_objectWithShadowsRenderDetails = refTexture.renderDetails;
        rd->m_shadowsRenderDetails = refShadows.renderDetails;

        return createReference(std::move(rdBase), refShadows, refObjectWithShadows);
    }

    void RenderDetailsVulkan::addDrawCmdsToCommandBuffer(
            VkCommandBuffer const &commandBuffer,
            VkFrameBuffer const &frameBuffer,
            size_t /* descriptor set ID, not used */,
            levelDrawer::LevelDrawerVulkan::CommonObjectDataList const &commonObjectDataList,
            levelDrawer::LevelDrawerVulkan::DrawObjectTableList const &drawObjTableList,
            levelDrawer::LevelDrawerVulkan::IndicesList const &drawObjectsIndicesList)
    {
        // only do shadows for the level itself
        levelDrawer::LevelDrawerVulkan::DrawObjectTableList shadowsDrawObjTableList =
                { nullptr, drawObjTableList[1], nullptr };
        levelDrawer::LevelDrawerVulkan::IndicesForList shadowsDrawObjectsIndicesList =
                { std::vector<size_t>{}, drawObjectsIndicesList[1], std::vector<size_t>{} };
        m_shadowsRenderDetails->addDrawCmdsToCommandBuffer(
                commandBuffer, m_framebufferShadows, 1 /* shadows ID */, commonObjectDataList,
                shadowsDrawObjTableList, shadowsDrawObjectsIndicesList);

        m_objectWithShadowsRenderDetails->addDrawCmdsToCommandBuffer(
                commandBuffer, frameBuffer, 0 /* main render details ID */, commonObjectDataList,
                drawObjTableList, drawObjectsIndicesList);
    }

    renderDetails::ReferenceVulkan RenderDetailsVulkan::createReference(
            std::shared_ptr<RenderDetailsVulkan> rd,
            renderDetails::ReferenceVulkan const &refShadows,
            renderDetails::ReferenceVulkan const &refObjectWithShadows,
            std::shared_ptr<vulkan::ImageSampler> const &shadowsImageSampler)
    {
        auto cod = std::make_shared<CommonObjectDataVulkan>();
        cod->m_objectWithShadowsCOD = refObjectWithShadows.commonObjectData;
        cod->m_shadowsCOD = refShadows.commonObjectData;

        // set the shadows image sampler
        auto codObjectWithShadows = dynamic_cast<objectWithShadows::CommonObjectDataVulkan*>(refObjectWithShadows.commonObjectData.get());
        if (codObjectWithShadows == nullptr) {
            throw std::runtime_error("Invalid common object data");
        }
        codObjectWithShadows->setShadowsImageSampler(shadowsImageSampler);

        renderDetails::ReferenceVulkan ref;
        ref.renderDetails = rd;
        ref.commonObjectData = cod;
        ref.createDrawObjectData = renderDetails::ReferenceVulkan::CreateDrawObjectData(
                [createDODShadows(refShadows.createDrawObjectData),
                createDODObjectWithShadow(refObjectWithShadows.createDrawObjectData)] (
                        std::shared_ptr<renderDetails::DrawObjectDataVulkan> const &sharingDOD,
                        std::shared_ptr<levelDrawer::TextureData> const &textureData,
                        glm::mat4 const &modelMatrix) ->
                        std::shared_ptr<renderDetails::DrawObjectDataVulkan>
                {
                    auto dodMain = createDODObjectWithShadow(sharingDOD, textureData, modelMatrix);
                    auto dodShadows = createDODShadows(dodMain,
                                                       std::shared_ptr<levelDrawer::TextureData>(), modelMatrix);

                    return std::make_shared<DrawObjectDataVulkan>(dodMain, dodShadows);
                }
        );

        ref.getProjViewForLevel = renderDetails::ReferenceVulkan::GetProjViewForLevel(
                [getPVObjectWithShadows(refObjectWithShadows.getProjViewForLevel)]() ->
                    renderDetails::ProjectionView
                {
                    return getPVObjectWithShadows();
                });

        return std::move(ref);
    }

} // namespace shadowsChaining