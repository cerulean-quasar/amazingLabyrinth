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

namespace textureWithShadows {
/* for accessing data other than the vertices from the shaders */
    void DescriptorSetLayout::createDescriptorSetLayout() {
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
            vkDestroyDescriptorSetLayout(capDevice->logicalDevice().get(), descriptorSetLayoutRaw,
                                         nullptr);
        };

        m_descriptorSetLayout.reset(descriptorSetLayoutRaw, deleter);
    }
}