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
            std::shared_ptr<renderDetails::Parameters> const &parametersBase,
            Config const &)
    {
        auto parameters = dynamic_cast<renderDetails::ParametersVulkan*>(parametersBase.get());
        if (parameters == nullptr) {
            throw std::runtime_error("Invalid render details parameter type.");
        }

        // initialize main render details
        auto rd = std::make_shared<RenderDetailsVulkan>(parameters->width, parameters->height);
        rd->m_device = inDevice;

        rd->createShadowResources(parameters);

        auto shadowParameters = createShadowParameters(rd.get(), parameters);

        // shadows render details
        auto refShadows = renderLoader->load(
                gameRequester, shadows::RenderDetailsVulkan::name(), shadowParameters);

        // shadows framebuffer
        rd->m_framebufferShadows = std::make_shared<vulkan::Framebuffer>(
                inDevice, rd->m_renderPassShadows,
                std::vector<std::shared_ptr<vulkan::ImageView>>{rd->m_shadowsColorAttachment,
                                                                rd->m_depthImageViewShadows},
                shadowParameters->width, shadowParameters->height);

        // main render details
        auto refMain = renderLoader->load(
                gameRequester, objectWithShadows::RenderDetailsVulkan::name(), parametersBase);

        rd->m_objectWithShadowsRenderDetails = refMain.renderDetails;
        rd->m_shadowsRenderDetails = refShadows.renderDetails;

        return createReference(std::move(rd), refShadows, refMain, rd->m_samplerShadows);
    }

    renderDetails::ReferenceVulkan RenderDetailsVulkan::loadExisting(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
            std::shared_ptr<renderDetails::RenderDetailsVulkan> rdBase,
            std::shared_ptr<renderDetails::Parameters> const &parametersBase,
            Config const &)
    {
        auto parameters = dynamic_cast<renderDetails::ParametersVulkan*>(parametersBase.get());
        if (parameters == nullptr) {
            throw std::runtime_error("Invalid render details parameter type.");
        }

        auto rd = dynamic_cast<RenderDetailsVulkan*>(rdBase.get());
        if (rd == nullptr) {
            throw std::runtime_error("Invalid render details type.");
        }
        auto shadowParameters = createShadowParameters(rd, parameters);

        // shadows render details
        auto refShadows = renderLoader->load(
            gameRequester, shadows::RenderDetailsVulkan::name(), shadowParameters);

        // object with shadows render details
        auto refObjectWithShadows = renderLoader->load(
            gameRequester, objectWithShadows::RenderDetailsVulkan::name(), parametersBase);

        rd->m_objectWithShadowsRenderDetails = refObjectWithShadows.renderDetails;
        rd->m_shadowsRenderDetails = refShadows.renderDetails;

        return createReference(std::move(rdBase), refShadows, refObjectWithShadows, rd->m_samplerShadows);
    }

    void RenderDetailsVulkan::reload(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
            std::shared_ptr<renderDetails::Parameters> const &parametersBase)
    {
        auto parameters = dynamic_cast<renderDetails::ParametersVulkan*>(parametersBase.get());
        if (parameters == nullptr) {
            throw std::runtime_error("Invalid render details parameter type.");
        }

        m_surfaceWidth = parameters->width;
        m_surfaceHeight = parameters->height;

        auto shadowParameters = createShadowParameters(this, parameters);
        m_shadowsRenderDetails->reload(gameRequester, renderLoader, shadowParameters);
        m_objectWithShadowsRenderDetails->reload(gameRequester, renderLoader, parametersBase);
    }

    void RenderDetailsVulkan::addPreRenderPassCmdsToCommandBuffer(
            VkCommandBuffer const &commandBuffer,
            size_t /* descriptor set ID, not used */,
            renderDetails::CommonObjectDataList const &commonObjectDataList,
            renderDetails::DrawObjectTableVulkanList const &drawObjTableList,
            renderDetails::IndicesForDrawList const &drawObjectsIndicesList)
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
                1 /* shadows ID */,
                commonObjectDataList[levelDrawer::ObjectType::LEVEL],
                drawObjTableList[levelDrawer::ObjectType::LEVEL],
                drawObjectsIndicesList[levelDrawer::ObjectType::LEVEL]);

        vkCmdEndRenderPass(commandBuffer);
    }

    void RenderDetailsVulkan::addDrawCmdsToCommandBuffer(
            VkCommandBuffer const &commandBuffer,
            size_t /* unused descriptor set ID */,
            std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
            std::shared_ptr<renderDetails::DrawObjectTableVulkan> const &drawObjTable,
            std::vector<size_t> const &drawObjectsIndices)
    {
        m_objectWithShadowsRenderDetails->addDrawCmdsToCommandBuffer(
                commandBuffer, 0 /* main render details ID */, commonObjectData,
                drawObjTable, drawObjectsIndices);
    }

    renderDetails::ReferenceVulkan RenderDetailsVulkan::createReference(
            std::shared_ptr<renderDetails::RenderDetailsVulkan> rd,
            renderDetails::ReferenceVulkan const &refShadows,
            renderDetails::ReferenceVulkan const &refObjectWithShadows,
            std::shared_ptr<vulkan::ImageSampler> const &shadowsImageSampler)
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

        // set the shadows image sampler
        cod->m_objectWithShadowsCOD->setShadowsImageSampler(shadowsImageSampler);

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

    void RenderDetailsVulkan::createShadowResources(renderDetails::ParametersVulkan const *parameters) {
        m_renderPassShadows.reset();
        m_samplerShadows.reset();
        m_shadowsColorAttachment.reset();
        m_depthImageViewShadows.reset();

        // shadow resources
        m_depthImageViewShadows = std::make_shared<vulkan::ImageView>(
                vulkan::ImageFactory::createDepthImage(
                        m_device,
                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                        getShadowsFramebufferDimension(parameters->width),
                        getShadowsFramebufferDimension(parameters->height)),
                VK_IMAGE_ASPECT_DEPTH_BIT);

        /*
        m_depthImageViewShadows->image()->transitionImageLayout(
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                m_commandPool);
                */ // todo: is this required?

        m_shadowsColorAttachment = vulkan::ImageView::createImageViewAndImage(
                m_device,
                getShadowsFramebufferDimension(parameters->width),
                getShadowsFramebufferDimension(parameters->height),
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
    }

    RegisterVulkan<renderDetails::RenderDetailsVulkan, RenderDetailsVulkan, Config> registerVulkan();
} // namespace shadowsChaining