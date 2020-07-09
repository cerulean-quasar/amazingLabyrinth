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

#ifndef AMAZING_LABYRINTH_RENDER_DETAILS_VULKAN_HPP
#define AMAZING_LABYRINTH_RENDER_DETAILS_VULKAN_HPP

#include <memory>
#include <vector>
#include <glm/glm.hpp>

#include "../graphicsVulkan.hpp"
#include "../levelDrawer/modelTable/modelTableVulkan.hpp"
#include "renderDetails.hpp"

namespace renderDetails {
    struct ParametersVulkan {
        std::vector<vulkan::RenderPass::ImageAttachmentInfo> colorImageInfo;
        vulkan::RenderPass::ImageAttachmentInfo depthImageInfo;
        glm::mat4 preTransform;
        uint32_t width;
        uint32_t height;

        virtual ~ParametersVulkan() = default;
    };

    class DrawObjectDataVulkan : public DrawObjectData {
    public:
        struct PerObjectUBO {
            glm::mat4 modelMatrix;
        };

        void update(glm::mat4 const &modelMatrix) override {
            PerObjectUBO ubo{};
            ubo.model = modelMatrix;
            m_uniformBuffer->copyRawTo(&ubo, sizeof (ubo));
        }

        virtual std::shared_ptr<vulkan::Buffer> const &bufferModelMatrix() = 0;
        virtual std::shared_ptr<vulkan::DescriptorSet> const &descriptorSet(uint32_t id) = 0;

        ~DrawObjectDataVulkan() override = default;
    };

    class RenderDetailsVulkan : public RenderDetails {
    public:
        virtual std::shared_ptr<vulkan::DescriptorPools> const &descriptorPools() = 0;
        virtual std::shared_ptr<vulkan::Device> const &device() = 0;
        void draw(
            vulkan::VkCommandBuffer commandBuffer,
            DrawObjectTableVulkan const &drawObjTable,
            std::vector<size_t> const &drawObjectsIndexList) = 0;

        RenderDetailsVulkan(uint32_t width, uint32_t height)
            : RenderDetails(width, height)
        {}

        ~RenderDetailsVulkan() override  = default;
    protected:
        VkVertexInputBindingDescription getBindingDescription();

        std::vector <VkVertexInputAttributeDescription> getAttributeDescriptions();
    };

    using ReferenceVulkan = renderDetails::RenderDetailsReference<RenderDetailsVulkan, renderDetails::CommonObjectData, renderDetails::DrawObjectDataVulkan>;
}
#endif // AMAZING_LABYRINTH_RENDER_DETAILS_VULKAN_HPP
