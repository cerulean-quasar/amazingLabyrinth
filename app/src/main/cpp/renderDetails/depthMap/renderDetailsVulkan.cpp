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

#include <array>
#include "../../graphicsVulkan.hpp"
#include "renderDetailsVulkan.hpp"
#include "../../renderLoader/registerVulkan.hpp"

namespace depthMap {
    /* for accessing data other than the vertices from the shaders */
    void DescriptorSetLayout::createDescriptorSetLayout() {
        /* Common Object Data */
        std::array<VkDescriptorSetLayoutBinding, 2> uboLayoutBinding = {};
        uboLayoutBinding[0].binding = 0;
        uboLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding[0].descriptorCount = 1;

        /* only accessing the common object data from the vertex shader */
        uboLayoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding[0].pImmutableSamplers = nullptr; // Optional

        /* the model matrix */
        uboLayoutBinding[1].binding = 1;
        uboLayoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding[1].descriptorCount = 1;

        /* only accessing the model matrix from the vertex shader */
        uboLayoutBinding[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding[1].pImmutableSamplers = nullptr; // Optional

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = uboLayoutBinding.size();
        layoutInfo.pBindings = uboLayoutBinding.data();

        VkDescriptorSetLayout descriptorSetLayoutRaw;
        if (vkCreateDescriptorSetLayout(m_device->logicalDevice().get(), &layoutInfo, nullptr,
                                        &descriptorSetLayoutRaw) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout for depth texture!");
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
                                              std::shared_ptr<CommonObjectDataVulkan> const &cod)
    {
        VkDescriptorBufferInfo bufferInfoCOD = {};
        bufferInfoCOD.buffer = cod->buffer()->buffer();
        bufferInfoCOD.offset = 0;
        bufferInfoCOD.range = cod->bufferSize();

        std::array<VkWriteDescriptorSet, 2> descriptorWrite = {};
        descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[0].dstSet = getVkType<>(m_descriptorSet->descriptorSet().get());
        descriptorWrite[0].dstBinding = 0;
        descriptorWrite[0].dstArrayElement = 0;
        descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite[0].descriptorCount = 1;
        descriptorWrite[0].pBufferInfo = &bufferInfoCOD;
        descriptorWrite[0].pNext = nullptr;
        descriptorWrite[0].pImageInfo = nullptr; // Optional
        descriptorWrite[0].pTexelBufferView = nullptr; // Optional

        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = m_uniformBuffer->buffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof (PerObjectUBO);

        descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[1].dstSet = getVkType<>(m_descriptorSet->descriptorSet().get());
        descriptorWrite[1].dstBinding = 1;
        descriptorWrite[1].dstArrayElement = 0;
        descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite[1].descriptorCount = 1;
        descriptorWrite[1].pBufferInfo = &bufferInfo;
        descriptorWrite[1].pNext = nullptr;
        descriptorWrite[1].pImageInfo = nullptr; // Optional
        descriptorWrite[1].pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(inDevice->logicalDevice().get(), descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
    }

    void RenderDetailsVulkan::addDrawCmdsToCommandBuffer(
            VkCommandBuffer const &commandBuffer,
            size_t descriptorSetID,
            std::shared_ptr<renderDetails::CommonObjectData> const &,
            std::shared_ptr<levelDrawer::DrawObjectTableVulkan> const &drawObjTable,
            std::set<levelDrawer::ZValueReference>::iterator beginZValRefs,
            std::set<levelDrawer::ZValueReference>::iterator endZValRefs,
            std::string const &)
    {
        initializeCommandBufferDrawObjects(
                commandBuffer, descriptorSetID, m_pipeline, nullptr,
                drawObjTable, beginZValRefs, endZValRefs);
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
                        modelMatrixBuffer =
                                renderDetails::createUniformBuffer(rd->device(),
                                                                sizeof (DrawObjectDataVulkan::PerObjectUBO));
                        DrawObjectDataVulkan::PerObjectUBO perObjectUbo{};
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
                m_vertShader, m_fragShader, nullptr);
    }

    char constexpr const *SHADER_SIMPLE_FRAG_VK_FILE = "shaders/simple.frag.spv";
    char constexpr const *SHADER_LINEAR_DEPTH_VERT_VK_FILE ="shaders/linearDepth.vert.spv";
    RegisterVulkan<renderDetails::RenderDetailsVulkan, RenderDetailsVulkan> registerVulkan{
        depthMapRenderDetailsName,
        std::vector<char const *>{SHADER_LINEAR_DEPTH_VERT_VK_FILE, SHADER_SIMPLE_FRAG_VK_FILE}};

}