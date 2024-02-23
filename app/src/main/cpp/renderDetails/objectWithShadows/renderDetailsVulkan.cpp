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
#include <memory>

#include "../../graphicsVulkan.hpp"
#include "renderDetailsVulkan.hpp"
#include "../../renderLoader/registerVulkan.hpp"

namespace objectWithShadows {
    /* descriptor set for the MVP matrix and texture samplers */
    void DrawObjectDataVulkan::colorUpdateDescriptorSet(
            std::shared_ptr<vulkan::Device> const &inDevice,
            std::shared_ptr<CommonObjectDataVulkan> const &cod)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = m_uniformBuffer->buffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof (PerObjectUBO);

        std::array<VkWriteDescriptorSet, 4> descriptorWrites = {};
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
        commonInfo.buffer = cod->cameraBuffer()->buffer();
        commonInfo.offset = 0;
        commonInfo.range = cod->cameraBufferSize();

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = getVkType<>(m_descriptorSet->descriptorSet().get());
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &commonInfo;

        VkDescriptorBufferInfo bufferLightingSource = {};
        bufferLightingSource.buffer = cod->lightingBuffer()->buffer();
        bufferLightingSource.offset = 0;
        bufferLightingSource.range = cod->lightingBufferSize();

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = getVkType<>(m_descriptorSet->descriptorSet().get());
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pBufferInfo = &bufferLightingSource;

        VkDescriptorImageInfo shadowInfo = {};
        shadowInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        shadowInfo.imageView = getVkType<>(cod->shadowsSampler()->imageView()->imageView().get());
        shadowInfo.sampler = getVkType<>(cod->shadowsSampler()->sampler().get());

        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = getVkType<>(m_descriptorSet->descriptorSet().get());
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pImageInfo = &shadowInfo;

        vkUpdateDescriptorSets(inDevice->logicalDevice().get(),
                               static_cast<uint32_t>(descriptorWrites.size()),
                               descriptorWrites.data(), 0, nullptr);
    }

    /* descriptor set for the MVP matrix and texture samplers */
    void DrawObjectDataVulkan::textureUpdateDescriptorSet(
            std::shared_ptr<vulkan::Device> const &inDevice,
            std::shared_ptr<CommonObjectDataVulkan> const &cod,
            std::shared_ptr<levelDrawer::TextureDataVulkan> const &textureData)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = m_uniformBuffer->buffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof (PerObjectUBO);

        std::array<VkWriteDescriptorSet, 5> descriptorWrites = {};
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
        commonInfo.buffer = cod->cameraBuffer()->buffer();
        commonInfo.offset = 0;
        commonInfo.range = cod->cameraBufferSize();

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = getVkType<>(m_descriptorSet->descriptorSet().get());
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &commonInfo;

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = getVkType<>(textureData->sampler()->imageView()->imageView().get());
        imageInfo.sampler = getVkType<>(textureData->sampler()->sampler().get());

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = getVkType<>(m_descriptorSet->descriptorSet().get());
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &imageInfo;

        VkDescriptorBufferInfo bufferLightingSource = {};
        bufferLightingSource.buffer = cod->lightingBuffer()->buffer();
        bufferLightingSource.offset = 0;
        bufferLightingSource.range = cod->lightingBufferSize();

        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = getVkType<>(m_descriptorSet->descriptorSet().get());
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pBufferInfo = &bufferLightingSource;

        VkDescriptorImageInfo shadowInfo = {};
        shadowInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        shadowInfo.imageView = getVkType<>(cod->shadowsSampler()->imageView()->imageView().get());
        shadowInfo.sampler = getVkType<>(cod->shadowsSampler()->sampler().get());

        descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[4].dstSet = getVkType<>(m_descriptorSet->descriptorSet().get());
        descriptorWrites[4].dstBinding = 4;
        descriptorWrites[4].dstArrayElement = 0;
        descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[4].descriptorCount = 1;
        descriptorWrites[4].pImageInfo = &shadowInfo;

        vkUpdateDescriptorSets(inDevice->logicalDevice().get(),
                               static_cast<uint32_t>(descriptorWrites.size()),
                               descriptorWrites.data(), 0, nullptr);
    }

    /* for accessing data other than the vertices from the shaders */
    void TextureDescriptorSetLayout::createDescriptorSetLayout() {
        /* model matrix - different for each object */
        VkDescriptorSetLayoutBinding modelMatrixBinding = {};
        modelMatrixBinding.binding = 0;
        modelMatrixBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        modelMatrixBinding.descriptorCount = 1;

        /* only accessing the MVP matrix from the vertex shader */
        modelMatrixBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        modelMatrixBinding.pImmutableSamplers = nullptr; // Optional

        /* view and projection matrix - the same for all objects */
        VkDescriptorSetLayoutBinding commonDataBinding = {};
        commonDataBinding.binding = 1;
        commonDataBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        commonDataBinding.descriptorCount = 1;
        commonDataBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        commonDataBinding.pImmutableSamplers = nullptr; // Optional

        /* image sampler */
        VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
        samplerLayoutBinding.binding = 2;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        samplerLayoutBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding lightingSourceBinding = {};
        lightingSourceBinding.binding = 3;
        lightingSourceBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        lightingSourceBinding.descriptorCount = 1;
        lightingSourceBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        lightingSourceBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding samplerShadows = {};
        samplerShadows.binding = 4;
        samplerShadows.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerShadows.descriptorCount = 1;
        samplerShadows.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        samplerShadows.pImmutableSamplers = nullptr;

        std::array<VkDescriptorSetLayoutBinding, 5> bindings = {modelMatrixBinding,
                                                                commonDataBinding,
                                                                samplerLayoutBinding,
                                                                lightingSourceBinding,
                                                                samplerShadows};

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

    /* for accessing data other than the vertices from the shaders */
    void ColorDescriptorSetLayout::createDescriptorSetLayout() {
        /* model matrix - different for each object */
        VkDescriptorSetLayoutBinding modelMatrixBinding = {};
        modelMatrixBinding.binding = 0;
        modelMatrixBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        modelMatrixBinding.descriptorCount = 1;

        /* only accessing the MVP matrix from the vertex shader */
        modelMatrixBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        modelMatrixBinding.pImmutableSamplers = nullptr; // Optional

        /* view and projection matrix - the same for all objects */
        VkDescriptorSetLayoutBinding commonDataBinding = {};
        commonDataBinding.binding = 1;
        commonDataBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        commonDataBinding.descriptorCount = 1;
        commonDataBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        commonDataBinding.pImmutableSamplers = nullptr; // Optional

        VkDescriptorSetLayoutBinding lightingSourceBinding = {};
        lightingSourceBinding.binding = 2;
        lightingSourceBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        lightingSourceBinding.descriptorCount = 1;
        lightingSourceBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        lightingSourceBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding samplerShadows = {};
        samplerShadows.binding = 3;
        samplerShadows.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerShadows.descriptorCount = 1;
        samplerShadows.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        samplerShadows.pImmutableSamplers = nullptr;

        std::array<VkDescriptorSetLayoutBinding, 4> bindings = {modelMatrixBinding,
                                                                commonDataBinding,
                                                                lightingSourceBinding,
                                                                samplerShadows};

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

    renderDetails::ReferenceVulkan RenderDetailsVulkan::createReference(
            std::shared_ptr<RenderDetailsVulkan> rd,
            std::shared_ptr<CommonObjectDataVulkan> cod)
    {
        renderDetails::ReferenceVulkan ref;
        ref.createDrawObjectData = renderDetails::ReferenceVulkan::CreateDrawObjectData(
                [rd, cod] (std::shared_ptr<renderDetails::DrawObjectDataVulkan> const &sharingDOD,
                           std::shared_ptr<levelDrawer::TextureDataVulkan> const &textureData,
                           glm::mat4 const &modelMatrix) ->
                        std::shared_ptr<renderDetails::DrawObjectDataVulkan>
                {
                    std::shared_ptr<vulkan::Buffer> modelMatrixBuffer{};
                    if (sharingDOD) {
                        modelMatrixBuffer = sharingDOD->bufferModelMatrix();
                    } else {
                        modelMatrixBuffer = renderDetails::createUniformBuffer(
                                rd->device(), sizeof (DrawObjectDataVulkan::PerObjectUBO));
                        DrawObjectDataVulkan::PerObjectUBO perObjectUbo{};
                        perObjectUbo.modelMatrix = modelMatrix;
                        modelMatrixBuffer->copyRawTo(
                                &perObjectUbo, sizeof(DrawObjectDataVulkan::PerObjectUBO));
                    }

                    std::shared_ptr<vulkan::DescriptorSet> ds;
                    if (textureData) {
                        ds = rd->m_descriptorPoolsTexture->allocateDescriptor();
                    } else {
                        ds = rd->m_descriptorPoolsColor->allocateDescriptor();
                    }

                    return std::make_shared<DrawObjectDataVulkan>(
                            rd->device(),
                            cod,
                            textureData,
                            std::move(ds),
                            std::move(modelMatrixBuffer),
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

    void RenderDetailsVulkan::addDrawCmdsToCommandBuffer(
            VkCommandBuffer const &commandBuffer,
            size_t descriptorSetID,
            std::shared_ptr<renderDetails::CommonObjectDataBase> const &,
            std::shared_ptr<levelDrawer::DrawObjectTableVulkan> const &drawObjTable,
            std::set<levelDrawer::ZValueReference>::iterator beginZValRefs,
            std::set<levelDrawer::ZValueReference>::iterator endZValRefs,
            renderDetails::Description const &)
    {
        initializeCommandBufferDrawObjects(
                commandBuffer, descriptorSetID, m_pipelineColor,
                m_pipelineTexture, drawObjTable, beginZValRefs, endZValRefs);
    }

    void RenderDetailsVulkan::reload(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails)
    {
        m_pipelineColor.reset();
        m_pipelineTexture.reset();

        m_surfaceWidth = surfaceDetails->surfaceWidth;
        m_surfaceHeight = surfaceDetails->surfaceHeight;

        m_pipelineTexture = std::make_shared<vulkan::Pipeline>(
                gameRequester, m_device, VkExtent2D{m_surfaceWidth, m_surfaceHeight},
                surfaceDetails->renderPass, m_descriptorPoolsTexture,
                getBindingDescription(),
                getAttributeDescriptions(),
                m_vertexShader, m_textureShader, nullptr);

        m_pipelineColor = std::make_shared<vulkan::Pipeline>(
                gameRequester, m_device, VkExtent2D{m_surfaceWidth, m_surfaceHeight},
                surfaceDetails->renderPass, m_descriptorPoolsColor,
                getBindingDescription(),
                getAttributeDescriptions(),
                m_vertexShader, m_colorShader, m_pipelineTexture);
    }

    char constexpr const *SHADER_VERT_VK_FILE = "shaders/shader.vert.spv";
    char constexpr const *TEXTURE_SHADER_FRAG_VK_FILE = "shaders/shader.frag.spv";
    char constexpr const *COLOR_SHADER_FRAG_VK_FILE = "shaders/colorShader.frag.spv";
    RegisterVulkan<renderDetails::RenderDetailsVulkan, RenderDetailsVulkan> registerVulkan(
            {renderDetails::DrawingStyle::standard, {renderDetails::Features::color, renderDetails::Features::texture, renderDetails::Features::shadows}},
            std::vector<char const *>{SHADER_VERT_VK_FILE, TEXTURE_SHADER_FRAG_VK_FILE, COLOR_SHADER_FRAG_VK_FILE});
}
