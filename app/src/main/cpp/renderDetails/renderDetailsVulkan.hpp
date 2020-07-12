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
#include "../levelDrawer/levelDrawerVulkan.hpp"
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
        virtual bool hasTexture() = 0;

        virtual std::shared_ptr<vulkan::Buffer> const &bufferModelMatrix() = 0;
        virtual std::shared_ptr<vulkan::DescriptorSet> const &descriptorSet(uint32_t id) = 0;

        ~DrawObjectDataVulkan() override = default;
    };

    class RenderDetailsVulkan : public RenderDetails {
    public:
        virtual void addDrawCmdsToCommandBuffer(
            VkCommandBuffer const &commandBuffer,
            VkFrameBuffer const &frameBuffer,
            size_t descriptorSetID,
            levelDrawer::LevelDrawerVulkan::CommonObjectDataList const &commonObjectDataList,
            levelDrawer::LevelDrawerVulkan::DrawObjectTables const &drawObjTable,
            levelDrawer::LevelDrawerVulkan::IndicesForDrawing const &drawObjectsIndicesList) = 0;

        void initializeCommandBufferDrawObjects(
            size_t descriptorSetID,
            VkCommandBuffer const &commandBuffer,
            levelDrawer::DrawObjectTableVulkan const &drawObjectTable,
            std::vector<size_t> const &drawObjectsIndices);

        RenderDetailsVulkan(uint32_t width, uint32_t height)
            : RenderDetails(width, height)
        {}

        ~RenderDetailsVulkan() override  = default;
    protected:
        virtual std::shared_ptr<vulkan::Pipeline> const &pipeline() = 0;

        VkVertexInputBindingDescription getBindingDescription();

        std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };

    using ReferenceVulkan = renderDetails::Reference<RenderDetailsVulkan, renderDetails::CommonObjectData, renderDetails::DrawObjectDataVulkan>;
}
#endif // AMAZING_LABYRINTH_RENDER_DETAILS_VULKAN_HPP
