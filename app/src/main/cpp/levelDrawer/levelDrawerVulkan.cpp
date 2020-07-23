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

#include "../graphicsVulkan.hpp"
#include "levelDrawerVulkan.hpp"
#include "levelDrawerGraphics.hpp"
#include "../renderDetails/renderDetailsVulkan.hpp"

namespace levelDrawer {
    template <>
    void LevelDrawerGraphics<LevelDrawerVulkanTraits>::draw(
            LevelDrawerVulkanTraits::DrawArgumentType const &info)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        /* pInheritanceInfo is only used if flags includes:
         * VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
         * because that would mean that there is a secondary command buffer that will be used
         * in a single render pass
         */
        beginInfo.pInheritanceInfo = nullptr; // Optional

        /* this call will reset the command buffer.  its not possible to append commands at
         * a later time.
         */
        vkBeginCommandBuffer(info.cmdBuffer, &beginInfo);

        auto rulesList = getDrawRules();

        // add the pre main draw commands to the command buffer.
        for (auto const &rule : rulesList) {
            rule.renderDetails->addPreRenderPassCmdsToCommandBuffer(
                    info.cmdBuffer, 0, rule.commonObjectData, m_drawObjectTableList,
                    rule.indicesPerLevelType);
        }

        // begin the main render pass
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = info.renderPass->renderPass().get();
        renderPassInfo.framebuffer = info.framebuffer;
        /* size of the render area */
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = info.extent;

        // the color value to use when clearing the image with VK_ATTACHMENT_LOAD_OP_CLEAR
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {m_bgColor.r, m_bgColor.g, m_bgColor.b, m_bgColor.a};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        /* begin recording commands - start by beginning the render pass.
         * none of these functions returns an error (they return void).  There will be no error
         * handling until recording is done.
         */
        vkCmdBeginRenderPass(info.cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // add the commands to the command buffer for the main draw.
        for (auto index : std::vector<ObjectType>{LEVEL, STARTER, FINISHER}) {
            for (auto const &rule : rulesList) {
                rule.renderDetails->addPreRenderPassCmdsToCommandBuffer(
                        info.cmdBuffer, 0, rule.commonObjectData, m_drawObjectTableList,
                        rule.indicesPerLevelType);
            }
        }

        // end the main render pass
        vkCmdEndRenderPass(info.cmdBuffer);

        // end the command buffer
        if (vkEndCommandBuffer(info.cmdBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    template <>
    void LevelDrawerGraphics<LevelDrawerVulkanTraits>::drawToBuffer(
            std::string const &renderDetailsName,
            ModelsTextures const &modelsTextures,
            std::vector<glm::mat4> const &modelMatrix,
            float width,
            float height,
            uint32_t nbrSamplesForWidth,
            float farthestDepth,
            float nearestDepth,
            std::vector<float> &results)
    {
        auto drawObjTable = std::make_shared<DrawObjectTableVulkan>();

        size_t i = 0;
        for (auto const &modelTexture : modelsTextures) {
            auto modelData = m_modelTable.addModel(m_gameRequester, modelTexture.first);
            std::shared_ptr<LevelDrawerVulkanTraits::TextureDataType> textureData{};
            if (modelTexture.second) {
                textureData = m_textureTable.addTexture(m_gameRequester, modelTexture.second);
            }
            auto objIndex = drawObjTable->addObject(modelData, textureData);
            addModelMatrixToDrawObjTable(drawObjTable, objIndex, modelMatrix[i++]);
        }

        uint32_t imageWidth = nbrSamplesForWidth;
        uint32_t imageHeight = static_cast<uint32_t>(std::floor((imageWidth * height)/width));
        auto depthView = std::make_shared<vulkan::ImageView>(
                vulkan::ImageFactory::createDepthImage(m_neededForDrawing.device, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                       imageWidth, imageHeight),
                VK_IMAGE_ASPECT_DEPTH_BIT);
        depthView->image()->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED,
                                                  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                  m_neededForDrawing.commandPool);

        auto colorImage = vulkan::ImageView::createImageViewAndImage(
                m_neededForDrawing.device,
                imageWidth,
                imageHeight,
                VK_FORMAT_R32G32B32A32_SFLOAT,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);

        std::vector<std::shared_ptr<vulkan::ImageView>> attachments = {colorImage, depthView};
        auto frameBuffer = std::make_shared<vulkan::Framebuffer>(
                m_neededForDrawing.device, renderPass, attachments, imageWidth, imageHeight);

        std::vector<vulkan::RenderPass::ImageAttachmentInfo> infos{};
        infos.emplace_back(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
                           colorImage->image()->format(),
                           VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        auto depthInfo = std::make_shared<vulkan::RenderPass::ImageAttachmentInfo>(
                VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                depthView->image()->format(),
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        auto renderPass = vulkan::RenderPass::createDepthTextureRenderPass(
                m_neededForDrawing.device, infos, depthInfo);

        std::vector<std::shared_ptr<vulkan::ImageView>> attachments = {colorImage, depthView};
        auto frameBuffer = std::make_shared<vulkan::Framebuffer>(
                m_neededForDrawing.device, renderPass, attachments, imageWidth, imageHeight);

        // parameters
        renderDetails::ParametersWithSurfaceWidthHeightAtDepthVulkan parameters{};
        parameters.renderPass = renderPass;
        parameters.width = imageWidth;
        parameters.height = imageHeight;
        parameters.preTransform = glm::mat4(1.0f);
        parameters.widthAtDepth = width;
        parameters.heightAtDepth = height;
        parameters.nearestDepth = nearestDepth;
        parameters.farthestDepth = farthestDepth;

        drawObjTable->loadRenderDetails(m_renderLoader.load(m_gameRequester, renderDetailsName, parameters));

        // start recording commands
        vulkan::CommandBuffer cmds{m_neededForDrawing.device, m_neededForDrawing.commandPool};
        cmds.begin();

        // add any pre main render pass commands
        auto rules = drawObjTable->getDrawRules();
        DrawObjectTableList drawObjTableList = {nullptr, drawObjTable, nullptr};
        CommonObjectDataList commonObjectDataList = {nullptr, rules[0].commonObjectData, nullptr};
        IndicesForDrawList indicesForDrawList = {nullptr, rules[0].drawObjectIndices, nullptr};
        rules[0].renderDetails->addPreRenderPassCmdsToCommandBuffer(
                cmds.commandBuffer().get(), 0, commonObjectDataList, drawObjTableList,
                indicesForDrawList);

        /* begin the render pass: drawing starts here*/
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass->renderPass().get();
        renderPassInfo.framebuffer = frameBuffer->framebuffer().get();
        /* size of the render area */
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = {imageWidth, imageHeight};

        /* the color value to use when clearing the image with VK_ATTACHMENT_LOAD_OP_CLEAR,
         * using black with 0% opacity
         */
        glm::vec4 clearColorValue = m_bgColor;
        rules[0].renderDetails->overrideClearColor(clearColorValue);
        std::array<VkClearValue, 2> clearValues = {};
        // matching what OpenGL is doing with the clear buffers.
        clearValues[0].color = {clearColorValue.r, clearColorValue.g, clearColorValue.b, clearColorValue.a};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();

        /* begin recording commands - start by beginning the render pass.
         * none of these functions returns an error (they return void).  There will be no error
         * handling until recording is done.
         */
        vkCmdBeginRenderPass(cmds.commandBuffer().get(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        rules.renderDetails->addDrawCmdsToCommandBuffer(cmds.commandBuffer().get(),
                0, rules.commonObjectData, drawObjTable, rules.drawObjectIndices);

        // end the main render pass
        vkCmdEndRenderPass(cmds.commandBuffer());

        // end the command buffer
        cmds.end();

        // copy the image data into a buffer accessible to the CPU
        std::vector<float> imageData{};
        imageData.resize(imageWidth * imageHeight * 4);

        vulkan::Buffer buffer{m_neededForDrawing.device,
                              imageWidth * imageHeight * sizeof (float) * 4,
                              VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
        colorDepthImage->image()->copyImageToBuffer(buffer, m_commandPool);
        buffer.copyRawFrom(imageData.data(), imageData.size() * sizeof (float) * 4);

        rules->renderDetails->postProcessImageBuffer(rules.commonObjectData, imageData, results);
    }

    template <>
    LevelDrawerGraphics<LevelDrawerVulkanTraits>::LevelDrawerGraphics(
            LevelDrawerVulkanTraits::NeededForDrawingType neededForDrawing,
            std::shared_ptr<LevelDrawerVulkanTraits::RenderLoaderType> inRenderLoader,
            std::shared_ptr<GameRequester> inGameRequester)
            : m_modelTable(neededForDrawing.device, neededForDrawing.m_commandPool),
            m_textureTable(neededForDrawing.device, neededForDrawing.m_commandPool),
            m_drawObjectTableList{
                std::make_shared<LevelDrawerVulkanTraits::DrawObjectTableType>(),
                std::make_shared<LevelDrawerVulkanTraits::DrawObjectTableType>(),
                std::make_shared<LevelDrawerVulkanTraits::DrawObjectTableType>()},
            m_renderLoader(std::move(inRenderLoader)),
            m_gameRequester(std::move(inGameRequester)),
            m_neededForDrawing(std::move(neededForDrawing)),
            m_bgColor{0.0f, 0.0f, 0.0f, 1.0f}
    {}
}