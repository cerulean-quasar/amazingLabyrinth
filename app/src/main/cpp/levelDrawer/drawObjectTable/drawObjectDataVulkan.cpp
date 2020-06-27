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

#include "../commonVulkan.hpp"
#include "drawObjectDataVulkan.hpp"

/* descriptor set for the MVP matrix and texture samplers */
void DrawObjectDataVulkan::updateDescriptorSet(std::shared_ptr<vulkan::Device> const &inDevice) {
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = m_uniformBuffer->buffer();
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(PerObjectUBO);

    std::array<VkWriteDescriptorSet, 5> descriptorWrites = {};
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
    commonInfo.buffer = m_commonUBO->buffer();
    commonInfo.offset = 0;
    commonInfo.range = sizeof(CommonUBO);

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = m_descriptorSet->descriptorSet().get();
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &commonInfo;

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_sampler->imageView()->imageView().get();
    imageInfo.sampler = m_sampler->sampler().get();

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = m_descriptorSet->descriptorSet().get();
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pImageInfo = &imageInfo;

    VkDescriptorBufferInfo bufferLightingSource = {};
    bufferLightingSource.buffer = m_uniformBufferLighting->buffer();
    bufferLightingSource.offset = 0;
    bufferLightingSource.range = sizeof(glm::vec3);

    descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[3].dstSet = m_descriptorSet->descriptorSet().get();
    descriptorWrites[3].dstBinding = 3;
    descriptorWrites[3].dstArrayElement = 0;
    descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[3].descriptorCount = 1;
    descriptorWrites[3].pBufferInfo = &bufferLightingSource;

    VkDescriptorImageInfo shadowInfo = {};
    shadowInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    shadowInfo.imageView = m_shadows->imageView()->imageView().get();
    shadowInfo.sampler = m_shadows->sampler().get();

    descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[4].dstSet = m_descriptorSet->descriptorSet().get();
    descriptorWrites[4].dstBinding = 4;
    descriptorWrites[4].dstArrayElement = 0;
    descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[4].descriptorCount = 1;
    descriptorWrites[4].pImageInfo = &shadowInfo;

    vkUpdateDescriptorSets(inDevice->logicalDevice().get(),
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);

    descriptorWrites[0].dstSet = m_descriptorSetShadows->descriptorSet().get();
    descriptorWrites[1].dstSet = m_descriptorSetShadows->descriptorSet().get();
    vkUpdateDescriptorSets(inDevice->logicalDevice().get(), 2,
                           descriptorWrites.data(), 0, nullptr);
}
