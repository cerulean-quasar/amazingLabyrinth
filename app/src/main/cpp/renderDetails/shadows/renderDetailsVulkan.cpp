/**
 * Copyright 2022 Cerulean Quasar. All Rights Reserved.
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
#include "../../renderLoader/registerVulkan.hpp"

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
        auto deleter = [capDevice](VkDescriptorSetLayout_CQ *descriptorSetLayoutRaw) {
            vulkan::deleteVkDescriptorSetLayout_CQ(capDevice, descriptorSetLayoutRaw);
        };

        m_descriptorSetLayout.reset(vulkan::createVkDescriptorSetLayout_CQ(descriptorSetLayoutRaw), deleter);
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
        descriptorWrites[0].dstSet = getVkType<>(m_descriptorSet->descriptorSet().get());

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
        commonInfo.buffer = inCommonObjectData->cameraBuffer()->buffer();
        commonInfo.offset = 0;
        commonInfo.range = inCommonObjectData->cameraBufferSize();

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = getVkType<>(m_descriptorSet->descriptorSet().get());
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
            size_t descriptorSetID,
            std::shared_ptr<renderDetails::CommonObjectData> const &,
            std::shared_ptr<levelDrawer::DrawObjectTableVulkan> const &drawObjTable,
            std::set<levelDrawer::ZValueReference>::iterator beginZValRefs,
            std::set<levelDrawer::ZValueReference>::iterator endZValRefs,
            std::string const &renderDetailsName)
    {
        initializeCommandBufferDrawObjects(
                commandBuffer, descriptorSetID, m_pipeline, nullptr,
                drawObjTable, beginZValRefs, endZValRefs, false, renderDetailsName);
    }

    renderDetails::ReferenceVulkan RenderDetailsVulkan::createReference(
            std::shared_ptr<RenderDetailsVulkan> rd,
            std::shared_ptr<CommonObjectDataVulkan> cod)
    {
        renderDetails::ReferenceVulkan ref;
        ref.createDrawObjectData = renderDetails::ReferenceVulkan::CreateDrawObjectData(
                [rd, cod] (std::shared_ptr<renderDetails::DrawObjectDataVulkan> const &sharingDOD,
                           std::shared_ptr<levelDrawer::TextureDataVulkan> const &,
                           glm::mat4 const &modelMatrix) ->
                        std::shared_ptr<renderDetails::DrawObjectDataVulkan>
                {
                    std::shared_ptr<vulkan::Buffer> modelMatrixBuffer{};
                    if (sharingDOD) {
                        modelMatrixBuffer = sharingDOD->bufferModelMatrix();
                    } else {
                        modelMatrixBuffer = renderDetails::createUniformBuffer(rd->device(),
                                                                sizeof (DrawObjectDataVulkan::PerObjectUBO));
                        DrawObjectDataVulkan::PerObjectUBO perObjectUbo{};
                        perObjectUbo.modelMatrix = modelMatrix;
                        modelMatrixBuffer->copyRawTo(&perObjectUbo,
                                                     sizeof(DrawObjectDataVulkan::PerObjectUBO));
                    }

                    auto descriptorSet = rd->descriptorPools()->allocateDescriptor();
                    return std::make_shared<DrawObjectDataVulkan>(
                            rd->device(), cod, std::move(descriptorSet),
                            std::move(modelMatrixBuffer), modelMatrix);
                });

        ref.getProjViewForLevel = renderDetails::ReferenceVulkan::GetProjViewForLevel(
                [cod] () -> std::pair<glm::mat4, glm::mat4> {
                    return cod->getProjViewForLevel();
                });

        ref.renderDetails = std::move(rd);
        ref.commonObjectData = std::move(cod);

        return std::move(ref);
    }

    void RenderDetailsVulkan::reload(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails)
    {
        m_pipeline.reset();

        m_surfaceWidth = surfaceDetails->surfaceWidth;
        m_surfaceHeight = surfaceDetails->surfaceHeight;

        m_pipeline = std::make_shared<vulkan::Pipeline>(
                gameRequester, m_device,
                VkExtent2D{m_surfaceWidth, m_surfaceHeight},
                surfaceDetails->renderPass, m_descriptorPools, getBindingDescription(),
                getAttributeDescriptions(),
                m_vertexShader, m_fragShader, nullptr,
                VK_CULL_MODE_FRONT_BIT);
    }

    char constexpr const *SHADOW_VERT_FILE = "shaders/depthShader.vert.spv";
    char constexpr const *SHADER_SIMPLE_FRAG_FILE = "shaders/simple.frag.spv";
    RegisterVulkan<renderDetails::RenderDetailsVulkan, RenderDetailsVulkan> registerVulkan(
            shadowsRenderDetailsName,
            std::vector<char const *>{SHADOW_VERT_FILE, SHADER_SIMPLE_FRAG_FILE});
}