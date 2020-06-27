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
#include "renderDetailsDataVulkan.hpp"

VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription = {};

    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);

    /* move to the next data entry after each vertex.  VK_VERTEX_INPUT_RATE_INSTANCE
     * moves to the next data entry after each instance, but we are not using instanced
     * rendering
     */
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {};

    attributeDescriptions.resize(4);

    /* position */
    attributeDescriptions[0].binding = 0; /* binding description to use */
    attributeDescriptions[0].location = 0; /* matches the location in the vertex shader */
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    /* color */
    attributeDescriptions[1].binding = 0; /* binding description to use */
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    /* texture coordinate */
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    /* normal vector */
    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex, normal);
    return attributeDescriptions;
}

/* for accessing data other than the vertices from the shaders */
void AmazingLabyrinthDescriptorSetLayout::createDescriptorSetLayout() {
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
    auto deleter = [capDevice](VkDescriptorSetLayout descriptorSetLayoutRaw) {
        vkDestroyDescriptorSetLayout(capDevice->logicalDevice().get(), descriptorSetLayoutRaw, nullptr);
    };

    m_descriptorSetLayout.reset(descriptorSetLayoutRaw, deleter);
}

/* for accessing data other than the vertices from the shaders */
void AmazingLabyrinthShadowsDescriptorSetLayout::createDescriptorSetLayout() {
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
        vkDestroyDescriptorSetLayout(capDevice->logicalDevice().get(), descriptorSetLayoutRaw, nullptr);
    };

    m_descriptorSetLayout.reset(descriptorSetLayoutRaw, deleter);
}

