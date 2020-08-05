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
#include "../../renderLoader/registerVulkan.hpp"

namespace normalMap {
    char constexpr const *RenderDetailsVulkan::SHADER_SIMPLE_FRAG_FILE;
    char constexpr const *RenderDetailsVulkan::SHADER_NORMAL_VERT_FILE;

    /* for accessing data other than the vertices from the shaders */
    void DescriptorSetLayout::createDescriptorSetLayout() {
        /* MVP matrix */
        VkDescriptorSetLayoutBinding uboLayoutBinding = {};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;

        /* only accessing the MVP matrix from the vertex shader */
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        VkDescriptorSetLayout descriptorSetLayoutRaw;
        if (vkCreateDescriptorSetLayout(m_device->logicalDevice().get(), &layoutInfo, nullptr,
                                        &descriptorSetLayoutRaw) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout for depth texture!");
        }

        auto const &capDevice = m_device;
        auto deleter = [capDevice](VkDescriptorSetLayout descriptorSetLayoutRaw) {
            vkDestroyDescriptorSetLayout(capDevice->logicalDevice().get(), descriptorSetLayoutRaw, nullptr);
        };

        m_descriptorSetLayout.reset(descriptorSetLayoutRaw, deleter);
    }

    /* descriptor set for the MVP matrix and texture samplers */
    void
    DrawObjectDataVulkan::updateDescriptorSet(std::shared_ptr<vulkan::Device> const &inDevice,
                                              std::shared_ptr<CommonObjectDataVulkan> const &)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = m_uniformBuffer->buffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof (PerObjectUBO);

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_descriptorSet->descriptorSet().get();
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pNext = nullptr;
        descriptorWrite.pImageInfo = nullptr; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(inDevice->logicalDevice().get(), 1, &descriptorWrite, 0, nullptr);
    }

    void RenderDetailsVulkan::addDrawCmdsToCommandBuffer(
            VkCommandBuffer const &commandBuffer,
            size_t descriptorSetID,
            std::shared_ptr<renderDetails::CommonObjectData> const &,
            std::shared_ptr<levelDrawer::DrawObjectTableVulkan> const &drawObjTable,
            std::set<levelDrawer::ZValueReference>::iterator beginZValRefs,
            std::set<levelDrawer::ZValueReference>::iterator endZValRefs)
    {
        initializeCommandBufferDrawObjects(
                commandBuffer, descriptorSetID, m_pipeline, nullptr,
                drawObjTable, beginZValRefs, endZValRefs, true);
    }

    renderDetails::ReferenceVulkan RenderDetailsVulkan::createReference(
            std::shared_ptr<RenderDetailsVulkan> rd,
            std::shared_ptr<CommonObjectDataVulkan> cod)
    {
        renderDetails::ReferenceVulkan ref;
        ref.createDrawObjectData = renderDetails::ReferenceVulkan::CreateDrawObjectData(
                [rd, cod] (
                        std::shared_ptr<renderDetails::DrawObjectDataVulkan> const &sharingDOD,
                           std::shared_ptr<levelDrawer::TextureData> const &,
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
                        auto projView = cod->getProjViewForRender();
                        perObjectUbo.mvp = projView.first * projView.second * modelMatrix;
                        perObjectUbo.modelMatrix = modelMatrix;
                        modelMatrixBuffer->copyRawTo(&perObjectUbo,
                                                     sizeof(DrawObjectDataVulkan::PerObjectUBO));
                    }

                    auto descriptorSet = rd->descriptorPools()->allocateDescriptor();
                    return std::make_shared<DrawObjectDataVulkan>(
                            rd->device(), cod, std::move(descriptorSet), std::move(modelMatrixBuffer),
                            modelMatrix);
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
            std::shared_ptr<RenderLoaderVulkan> const &,
            std::shared_ptr<renderDetails::Parameters> const &parametersBase)
    {
        auto parameters =
                dynamic_cast<renderDetails::ParametersWithSurfaceWidthHeightAtDepthVulkan*>(parametersBase.get());
        if (parameters == nullptr) {
            throw std::runtime_error("Invalid render details parameter type.");
        }

        m_pipeline.reset();

        m_surfaceWidth = parameters->width;
        m_surfaceHeight = parameters->height;

        m_pipeline = std::make_shared<vulkan::Pipeline>(
                gameRequester, m_device,
                VkExtent2D{m_surfaceWidth, m_surfaceHeight},
                parameters->renderPass, m_descriptorPools, getBindingDescription(),
                getAttributeDescriptions(),
                SHADER_NORMAL_VERT_FILE, SHADER_SIMPLE_FRAG_FILE, nullptr);
    }

    RegisterVulkan<renderDetails::RenderDetailsVulkan, RenderDetailsVulkan, Config> registerVulkan;

}