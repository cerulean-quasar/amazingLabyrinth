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

#include "../../levelDrawer/textureTable/textureLoader.hpp"
#include "../../graphicsVulkan.hpp"
#include "../../levelDrawer/drawObjectTable/drawObject.hpp"

namespace shadows {
    struct PerObjectUBO {
        glm::mat4 modelMatrix;
    };

    /* for passing data other than the vertex data to the vertex shader */
    class DrawObjectDataVulkan : public DrawObjectData {
    public:
        void update(glm::mat4 const &modelMatrix) override {
            PerObjectUBO ubo{};
            ubo.modelMatrix = modelMatrix;
            m_uniformBuffer->copyRawTo(&ubo, sizeof(ubo));
        }

        inline std::shared_ptr<vulkan::DescriptorSet> const &descriptorSet() { return m_descriptorSet; }

        DrawObjectDataVulkan(std::shared_ptr<vulkan::Device> const &inDevice,
                             std::shared_ptr<vulkan::DescriptorPools> const &descriptorPools,
                             std::shared_ptr<vulkan::Buffer> const &inCommonUBO,
                             glm::mat4 const &modelMatrix)
                : m_descriptorSet{descriptorPools->allocateDescriptor()},
                  m_uniformBuffer{createUniformBuffer(inDevice, sizeof(PerObjectUBO))},
                  m_commonUBO{inCommonUBO}
        {
            PerObjectUBO ubo;
            ubo.modelMatrix = modelMatrix;
            m_uniformBuffer->copyRawTo(&ubo, sizeof(ubo));
            updateDescriptorSet(inDevice);
        }

        ~DrawObjectDataVulkan() override = default;

    private:
        std::shared_ptr<vulkan::DescriptorSet> m_descriptorSet;
        std::shared_ptr<vulkan::Buffer> m_uniformBuffer;
        std::shared_ptr<vulkan::Buffer> m_commonUBO;

        void updateDescriptorSet(std::shared_ptr<vulkan::Device> const &inDevice);
    };
}
#endif // AMAZING_LABYRINTH_DRAW_OBJECT_DATA_VULKAN_HPP