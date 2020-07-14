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

#include "../../graphicsVulkan.hpp"
#include "renderDetailsVulkan.hpp"

namespace shadows {
    /* for accessing data other than the vertices from the shaders */
    void DescriptorSetLayout::createDescriptorSetLayout() {
        /* model matrix */
        VkDescriptorSetLayoutBinding perObject = {};
        perObject.binding = 0;
        perObject.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        perObject.descriptorCount = 1;
        perObject.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        perObject.pImmutableSamplers = nullptr; // Optional

        /* the projection, view, and view light source matrix */
        VkDescriptorSetLayoutBinding commonData = {};
        commonData.binding = 1;
        commonData.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        commonData.descriptorCount = 1;
        commonData.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        commonData.pImmutableSamplers = nullptr;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {perObject, commonData};

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        VkDescriptorSetLayout descriptorSetLayoutRaw;
        if (vkCreateDescriptorSetLayout(m_device->logicalDevice().get(), &layoutInfo, nullptr,
                                        &descriptorSetLayoutRaw) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        auto const &capDevice = m_device;
        auto deleter = [capDevice](VkDescriptorSetLayout descriptorSetLayoutRaw) {
            vkDestroyDescriptorSetLayout(capDevice->logicalDevice().get(), descriptorSetLayoutRaw,
                                         nullptr);
        };

        m_descriptorSetLayout.reset(descriptorSetLayoutRaw, deleter);
    }

    /* descriptor set for the MVP matrix and texture samplers */
    void
    DrawObjectDataVulkan::updateDescriptorSet(std::shared_ptr<vulkan::Device> const &inDevice,
            std::shared_ptr<CommonObjectDataVulkan> const &inCommonObjectData)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = m_uniformBuffer->buffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof (PerObjectUBO);

        std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_descriptorSet->descriptorSet().get();

        /* must be the same as the binding in the vertex shader */
        descriptorWrites[0].dstBinding = 0;

        /* index into the array of descriptors */
        descriptorWrites[0].dstArrayElement = 0;

        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        /* how many array elements you want to update */
        descriptorWrites[0].descriptorCount = 1;

        /* which one of these pointers needs to be used depends on which descriptorType we are
         * using.  pBufferInfo is for buffer based data, pImageInfo is used for image data, and
         * pTexelBufferView is used for decriptors that refer to buffer views.
         */
        descriptorWrites[0].pBufferInfo = &bufferInfo;
        descriptorWrites[0].pImageInfo = nullptr; // Optional
        descriptorWrites[0].pTexelBufferView = nullptr; // Optional

        VkDescriptorBufferInfo commonInfo = {};
        commonInfo.buffer = inCommonObjectData->cameraBuffer();
        commonInfo.offset = 0;
        commonInfo.range = inCommonObjectData->cameraBufferSize();

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_descriptorSet->descriptorSet().get();
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &commonInfo;

        vkUpdateDescriptorSets(inDevice->logicalDevice().get(), descriptorWrites.size(),
                               descriptorWrites.data(), 0, nullptr);
    }

    void RenderDetailsVulkan::addDrawCmdsToCommandBuffer(
            VkCommandBuffer const &commandBuffer,
            VkFrameBuffer const &frameBuffer,
            size_t descriptorSetID,
            levelDrawer::LevelDrawerVulkan::CommonObjectDataList const &commonObjectDataList,
            levelDrawer::LevelDrawerVulkan::DrawObjectTables const &drawObjTableList,
            levelDrawer::LevelDrawerVulkan::IndicesForDrawing const &drawObjectsIndicesList)
    {
        /* begin the render pass: drawing starts here*/
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass->renderPass().get();
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
                          m_pipeline->pipeline().get());

        // Draw from back to front in order to allow see through objects to show things under them.

        // Draw the level, then the level starter, then the finisher.
        for (auto &&index : std::vector<size_t>(levelDrawer::LevelDrawerVulkan::ObjectType::LEVEL,
                                                levelDrawer::LevelDrawerVulkan::ObjectType::STARTER,
                                                levelDrawer::LevelDrawerVulkan::ObjectType::FINISHER)) {
            initializeCommandBufferDrawObjects(
                    DrawIfHasTexture::BOTH, descriptorSetID, m_pipelineTexture,
                    commandBuffer, drawObjTableList[index], drawObjectsIndicesList[index]);
        }

        vkCmdEndRenderPass(commandBuffer);
    }

    static renderDetails::ReferenceVulkan RenderDetailsVulkan::createReference(
            std::shared_ptr<renderDetails::RenderDetailsVulkan> rd,
            std::shared_ptr<CommonObjectDataVulkan> cod)
    {
        renderDetails::ReferenceVulkan ref;
        ref.createDrawObjectData = renderDetails::ReferenceVulkan::CreateDrawObjectData(
                [rd, cod] (std::shared_ptr<renderDetails::DrawObjectDataVulkan> const &sharingDOD,
                           std::shared_ptr<TextureData> const &textureData,
                           glm::mat4 const &modelMatrix) ->
                        std::shared_ptr<renderDetails::DrawObjectDataVulkan>
                {
                    std::shared_ptr<vulkan::Buffer> modelMatrixBuffer{};
                    if (sharingDOD) {
                        modelMatrixBuffer = sharingDOD->bufferModelMatrix();
                    } else {
                        modelMatrixBuffer = createUniformBuffer(rd->device(),
                                                                sizeof (DrawObjectDataVulkan::PerObjectUBO));
                        DrawObjectDataVulkan::PerObjectUBO perObjectUbo{};
                        perObjectUbo.model = modelMatrix;
                        modelMatrixBuffer->copyRawTo(&perObjectUbo,
                                                     sizeof(DrawObjectDataVulkan::PerObjectUBO));
                    }

                    auto descriptorSet = rd->descriptorPools()->allocateDescriptor();
                    return std::make_shared<DrawObjectDataVulkan>(
                            rd->device(), cod, std::move(descriptorSet), std::move(modelMatrixBuffer));
                });

        ref.getProjViewForLevel = renderDetails::ReferenceVulkan::GetProjViewForLevel(
                [cod] () -> std::pair<glm::mat4, glm::mat4> {
                    return cod->getProjViewForLevel();
                });

        ref.renderDetails = std::move(rd);
        ref.commonObjectData = std::move(cod);

        return std::move(ref);
    }
}