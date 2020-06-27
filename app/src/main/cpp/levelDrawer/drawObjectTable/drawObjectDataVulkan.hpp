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
#ifndef AMAZING_LABYRINTH_DRAW_OBJECT_DATA_VULKAN_HPP
#define AMAZING_LABYRINTH_DRAW_OBJECT_DATA_VULKAN_HPP

#include <momory>
#include <glm/glm.h>
#include <boost/optional.hpp>
#include "../textureTable/textureLoader.hpp"
#include "../../graphicsVulkan.hpp"
#include "drawObject.hpp"

struct PerObjectUBO {
    glm::mat4 model;
};

/* for passing data other than the vertex data to the vertex shader */
class DrawObjectDataVulkan : DrawObjectData {
    void update(glm::mat4 const &modelMatrix) override {
        PerObjectUBO ubo{};
        ubo.model = modelMatrix;
        m_uniformBuffer->copyRawTo(&ubo, sizeof (ubo));
    }

    inline std::shared_ptr<vulkan::DescriptorSet> const &descriptorSetShadows() { return m_descriptorSetShadows; }
    inline std::shared_ptr<vulkan::DescriptorSet> const &descriptorSet() { return m_descriptorSet; }

    DrawObjectDataVulkan(std::shared_ptr<vulkan::Device> const &inDevice,
            std::shared_ptr<vulkan::DescriptorPools> const &descriptorPools,
            std::shared_ptr<vulkan::DescriptorPools> const &descriptorPoolsShadows,
            std::shared_ptr<vulkan::ImageSampler> const &inSampler,
            std::shared_ptr<vulkan::ImageSampler> const &inShadows,
            std::shared_ptr<vulkan::Buffer> const &inUniformBufferLighting,
            std::shared_ptr<vulkan::Buffer> const &inCommonUBO,
            PerObjectUBO const &ubo)
            : m_descriptorSet{descriptorPools->allocateDescriptor()},
              m_descriptorSetShadows{descriptorPoolsShadows->allocateDescriptor()},
              m_sampler{inSampler},
              m_shadows{inShadows},
              m_uniformBufferLighting{inUniformBufferLighting},
              m_uniformBuffer{createUniformBuffer(inDevice, sizeof (PerObjectUBO))},
              m_commonUBO{inCommonUBO}
    {
        m_uniformBuffer->copyRawTo(&ubo, sizeof (ubo));
        updateDescriptorSet(inDevice);
    }

    ~DrawObjectDataVulkan() override = default;
private:
    std::shared_ptr<vulkan::DescriptorSet> m_descriptorSet;
    std::shared_ptr<vulkan::DescriptorSet> m_descriptorSetShadows;
    std::shared_ptr<vulkan::ImageSampler> m_sampler;
    std::shared_ptr<vulkan::ImageSampler> m_shadows;
    std::shared_ptr<vulkan::Buffer> m_uniformBufferLighting;
    std::shared_ptr<vulkan::Buffer> m_uniformBuffer;
    std::shared_ptr<vulkan::Buffer> m_commonUBO;

    void updateDescriptorSet(std::shared_ptr<vulkan::Device> const &inDevice);
};
#endif // AMAZING_LABYRINTH_DRAW_OBJECT_DATA_VULKAN_HPP