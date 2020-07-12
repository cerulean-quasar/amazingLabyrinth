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

        // texture render details
        auto refTexture = renderLoader->load(
            gameRequester, renderLoader, textureWithShadows::RenderDetailsVulkan::name(), parameters);

        // color render details
        auto refColor = renderLoader->load(
            gameRequester, renderLoader, colorWithShadows::RenderDetailsVulkan::name(), parameters);

        rd->m_textureRenderDetails = refTexture.renderDetails;
        rd->m_colorRenderDetails = refColor.renderDetails;
        rd->m_shadowsRenderDetails = refShadows.renderDetails;

        return createReference(std::move(rdBase), refShadows, refTexture, refColor);
    }

    void RenderDetailsVulkan::addDrawCmdsToCommandBuffer(
        VkCommandBuffer const &commandBuffer,
        VkFrameBuffer const &frameBuffer,
        size_t descriptorSetID,
        levelDrawer::LevelDrawerVulkan::CommonObjectDataList const &commonObjectDataList,
        levelDrawer::LevelDrawerVulkan::DrawObjectTables const &drawObjTableList,
        levelDrawer::LevelDrawerVulkan::IndicesForDrawing const &drawObjectsIndicesList))
    {
        /* begin the render pass: drawing starts here*/
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = pipeline->renderPass()->renderPass().get();
        renderPassInfo.framebuffer = framebuffer;
        /* size of the render area */
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = pipeline->extent();

        /* the color value to use when clearing the image with VK_ATTACHMENT_LOAD_OP_CLEAR,
         * using black with 0% opacity
         */
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        /* begin recording commands - start by beginning the render pass.
         * none of these functions returns an error (they return void).  There will be no error
         * handling until recording is done.
         */
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        /* bind the graphics pipeline to the command buffer, the second parameter tells Vulkan
         * that we are binding to a graphics pipeline.
         */
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          pipeline->pipeline().get());

        // the objects that stay static.
        initializeCommandBufferDrawObjects(commandBuffer, m_levelSequence->levelStaticObjsData(),
                                           pipeline, doShadows);

        // the objects that move.
        initializeCommandBufferDrawObjects(commandBuffer, m_levelSequence->levelDynObjsData(),
                                           pipeline, doShadows);

        // only do shadows for the level itself

        // the level starter
        if (!doShadows) {
            initializeCommandBufferDrawObjects(commandBuffer,
                                               m_levelSequence->starterStaticObjsData(), pipeline,
                                               doShadows);
            initializeCommandBufferDrawObjects(commandBuffer, m_levelSequence->starterDynObjsData(),
                                               pipeline, doShadows);
        }

        // the level finisher objects.
        if (m_levelSequence->needFinisherObjs() && !doShadows) {
            initializeCommandBufferDrawObjects(commandBuffer, m_levelSequence->finisherObjsData(),
                                               pipeline, doShadows);
        }

        vkCmdEndRenderPass(commandBuffer);
    }
} // namespace shadowsChaining