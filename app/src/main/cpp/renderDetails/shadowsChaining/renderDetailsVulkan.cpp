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
#include "../../renderLoader/renderLoaderVulkan.hpp"

namespace shadowsChaining {
    renderDetails::ReferenceVulkan RenderDetailsVulkan::loadNew(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
            std::shared_ptr<vulkan::Device> const &inDevice,
            std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &,
            Config const &)
    {
        // initialize main render details
        auto rd = std::make_shared<RenderDetailsVulkan>(inDevice, surfaceDetails);

        auto shadowsSurfaceDetails = rd->createShadowSurfaceDetails(surfaceDetails);

        // shadows render details
        auto refShadows = renderLoader->load(
                gameRequester, shadows::RenderDetailsVulkan::name(), shadowsSurfaceDetails, nullptr);

        renderDetails::ParametersWithShadowsVulkan params{};
        params.shadowsSampler = rd->m_samplerShadows;
        auto parameters = std::make_shared<renderDetails::ParametersWithShadowsVulkan>(params);

        // main render details
        auto refMain = renderLoader->load(
                gameRequester, objectWithShadows::RenderDetailsVulkan::name(),
                surfaceDetails, parameters);

        rd->m_objectWithShadowsRenderDetails = refMain.renderDetails;
        rd->m_shadowsRenderDetails = refShadows.renderDetails;

        return createReference(std::move(rd), refShadows, refMain);
    }

    renderDetails::ReferenceVulkan RenderDetailsVulkan::loadExisting(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
            std::shared_ptr<renderDetails::RenderDetailsVulkan> rdBase,
            std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &,
            Config const &)
    {
        auto rd = std::dynamic_pointer_cast<RenderDetailsVulkan>(rdBase);
        if (rd == nullptr) {
            throw std::runtime_error("Invalid render details type.");
        }

        if (rd->structuralChangeNeeded(surfaceDetails)) {
            rd->createShadowResources(surfaceDetails);
        }

        auto shadowSurfaceDetails = rd->createShadowSurfaceDetails(surfaceDetails);

        // shadows render details
        auto refShadows = renderLoader->load(
            gameRequester, shadows::RenderDetailsVulkan::name(), shadowSurfaceDetails, nullptr);

        renderDetails::ParametersWithShadowsVulkan params{};
        params.shadowsSampler = rd->m_samplerShadows;
        auto parameters = std::make_shared<renderDetails::ParametersWithShadowsVulkan>(params);

        // object with shadows render details
        auto refObjectWithShadows = renderLoader->load(
            gameRequester, objectWithShadows::RenderDetailsVulkan::name(), surfaceDetails,
            parameters);

        rd->m_objectWithShadowsRenderDetails = refObjectWithShadows.renderDetails;
        rd->m_shadowsRenderDetails = refShadows.renderDetails;

        return createReference(std::move(rd), refShadows, refObjectWithShadows);
    }

    void RenderDetailsVulkan::addPreRenderPassCmdsToCommandBuffer(
            VkCommandBuffer const &commandBuffer,
            size_t /* descriptor set ID, not used */,
            levelDrawer::CommonObjectDataList const &commonObjectDataList,
            levelDrawer::DrawObjectTableVulkanList const &drawObjTableList,
            std::set<levelDrawer::ZValueReference> const &/* starter Z Value references - unused */,
            std::set<levelDrawer::ZValueReference> const &levelZValues,
            std::set<levelDrawer::ZValueReference> const &/* finisher Z Value references - unused */)
    {
        // The shadows rendering needs to occur before the main render pass.

        /* begin the shadows render pass */
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPassShadows->renderPass().get();
        renderPassInfo.framebuffer = m_framebufferShadows->framebuffer().get();
        /* size of the render area */
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = VkExtent2D{getShadowsFramebufferDimension(m_surfaceWidth),
                                                      getShadowsFramebufferDimension(m_surfaceHeight)};

        /* the color value to use when clearing the image with VK_ATTACHMENT_LOAD_OP_CLEAR,
         * using black with 0% opacity
         */
        std::vector<VkClearValue> clearValues{};
        clearValues.resize(2);
        clearValues[0].color = {1.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        /* begin recording commands - start by beginning the render pass.
         * none of these functions returns an error (they return void).  There will be no error
         * handling until recording is done.
         */
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // only do shadows for the level itself
        m_shadowsRenderDetails->addDrawCmdsToCommandBuffer(
                commandBuffer,
                renderDetails::MODEL_MATRIX_ID_SHADOWS /* shadows ID */,
                commonObjectDataList[levelDrawer::ObjectType::LEVEL],
                drawObjTableList[levelDrawer::ObjectType::LEVEL],
                levelZValues.begin(), levelZValues.end());

        vkCmdEndRenderPass(commandBuffer);
    }

    void RenderDetailsVulkan::addDrawCmdsToCommandBuffer(
            VkCommandBuffer const &commandBuffer,
            size_t /* unused descriptor set ID */,
            std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
            std::shared_ptr<levelDrawer::DrawObjectTableVulkan> const &drawObjTable,
            std::set<levelDrawer::ZValueReference>::iterator beginZValRefs,
            std::set<levelDrawer::ZValueReference>::iterator endZValRefs)
    {
        m_objectWithShadowsRenderDetails->addDrawCmdsToCommandBuffer(
                commandBuffer, renderDetails::MODEL_MATRIX_ID_MAIN /* main render details ID */,
                commonObjectData, drawObjTable, beginZValRefs, endZValRefs);
    }

    renderDetails::ReferenceVulkan RenderDetailsVulkan::createReference(
            std::shared_ptr<RenderDetailsVulkan> rd,
            renderDetails::ReferenceVulkan const &refShadows,
            renderDetails::ReferenceVulkan const &refObjectWithShadows)
    {
        auto cod = std::make_shared<CommonObjectDataVulkan>();
        cod->m_objectWithShadowsCOD = std::dynamic_pointer_cast<objectWithShadows::CommonObjectDataVulkan>(refObjectWithShadows.commonObjectData);
        if (cod->m_objectWithShadowsCOD == nullptr) {
            throw std::runtime_error("Invalid common object data");
        }

        cod->m_shadowsCOD = std::dynamic_pointer_cast<shadows::CommonObjectDataVulkan>(refShadows.commonObjectData);
        if (cod->m_shadowsCOD == nullptr) {
            throw std::runtime_error("Invalid common object data");
        }

        renderDetails::ReferenceVulkan ref;
        ref.renderDetails = rd;
        ref.commonObjectData = cod;
        ref.createDrawObjectData = renderDetails::ReferenceVulkan::CreateDrawObjectData(
                [createDODShadows(refShadows.createDrawObjectData),
                createDODObjectWithShadow(refObjectWithShadows.createDrawObjectData)] (
                        std::shared_ptr<renderDetails::DrawObjectDataVulkan> const &sharingDOD,
                        std::shared_ptr<levelDrawer::TextureDataVulkan> const &textureData,
                        glm::mat4 const &modelMatrix) ->
                        std::shared_ptr<renderDetails::DrawObjectDataVulkan>
                {
                    auto dodMain = createDODObjectWithShadow(sharingDOD, textureData, modelMatrix);
                    auto dodShadows = createDODShadows(
                            dodMain, std::shared_ptr<levelDrawer::TextureDataVulkan>(), modelMatrix);

                    return std::make_shared<DrawObjectDataVulkan>(dodMain, dodShadows);
                }
        );

        ref.getProjViewForLevel = renderDetails::ReferenceVulkan::GetProjViewForLevel(
                [getPVObjectWithShadows(refObjectWithShadows.getProjViewForLevel)]() ->
                        std::pair<glm::mat4, glm::mat4>
                {
                    return getPVObjectWithShadows();
                });

        return std::move(ref);
    }

    void RenderDetailsVulkan::createShadowResources(std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails) {
        m_framebufferShadows.reset();
        m_renderPassShadows.reset();
        m_samplerShadows.reset();
        m_shadowsColorAttachment.reset();
        m_depthImageViewShadows.reset();

        auto width = getShadowsFramebufferDimension(surfaceDetails->surfaceWidth);
        auto height = getShadowsFramebufferDimension(surfaceDetails->surfaceHeight);

        // shadow resources
        m_depthImageViewShadows = std::make_shared<vulkan::ImageView>(
                vulkan::ImageFactory::createDepthImage(
                        m_device, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, width, height),
                VK_IMAGE_ASPECT_DEPTH_BIT);

        m_shadowsColorAttachment = vulkan::ImageView::createImageViewAndImage(
                m_device,
                width,
                height,
                VK_FORMAT_R32G32B32A32_SFLOAT,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);
        m_samplerShadows = std::make_shared<vulkan::ImageSampler>(
                m_device, m_shadowsColorAttachment,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
        auto colorImageInfo =
                std::vector<vulkan::RenderPass::ImageAttachmentInfo>{vulkan::RenderPass::ImageAttachmentInfo{
                        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
                        m_shadowsColorAttachment->image()->format(),
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}};
        auto depthImageInfo =
                std::make_shared<vulkan::RenderPass::ImageAttachmentInfo>(
                        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
                        m_depthImageViewShadows->image()->format(),
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        m_renderPassShadows = vulkan::RenderPass::createDepthTextureRenderPass(
                m_device, colorImageInfo, depthImageInfo);

        // shadows framebuffer
        m_framebufferShadows = std::make_shared<vulkan::Framebuffer>(
                m_device, m_renderPassShadows,
                std::vector<std::shared_ptr<vulkan::ImageView>>{m_shadowsColorAttachment,
                                                                m_depthImageViewShadows},
                width, height);


    }

    RegisterVulkan<renderDetails::RenderDetailsVulkan, RenderDetailsVulkan, Config> registerVulkan;
} // namespace shadowsChaining