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
#include "../levelDrawer/drawObjectTable/drawObjectTable.hpp"
#include "../levelDrawer/drawObjectTable/drawObjectTableVulkan.hpp"

class RenderLoaderVulkan;
namespace renderDetails {
    using DrawObjectTableVulkan = levelDrawer::DrawObjectTable<levelDrawer::DrawObjectVulkanTraits>;
    using DrawObjectTableVulkanList = std::array<std::shared_ptr<DrawObjectTableVulkan>, nbrDrawObjectTables>;

    std::shared_ptr<vulkan::Buffer> createUniformBuffer(
            std::shared_ptr<vulkan::Device> const &device, size_t bufferSize);

    struct ParametersVulkan : public Parameters {
        std::shared_ptr<vulkan::RenderPass> renderPass;
        glm::mat4 preTransform;

        virtual ~ParametersVulkan() = default;
    };

    struct ParametersWithSurfaceWidthHeightAtDepthVulkan : public ParametersVulkan {
        float widthAtDepth;
        float heightAtDepth;
        float nearestDepth;
        float farthestDepth;

        ~ParametersWithSurfaceWidthHeightAtDepthVulkan() override = default;
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
        // This function is called to add commands to the command buffer that occur before
        // the main render pass.  Most Render Details don't don't do anything for this function.
        virtual void addPreRenderPassCmdsToCommandBuffer(
                VkCommandBuffer const &/* unused command buffer */,
                size_t /* descriptor set ID, not used */,
                CommonObjectDataList const &/* common object data */,
                DrawObjectTableVulkanList const &/* draw object table */,
                IndicesForDrawList const &/* draw indices */)
        {}

        // For postprocessing results written to an image buffer whose contents are put in input.
        // This function is for render details that produce results to be used by the CPU.  Most
        // render details don't need this.
        virtual void postProcessImageBuffer(
                std::shared_ptr<renderDetails::CommonObjectData> const &,
                std::vector<float> const &input,
                std::vector<float> &results)
        {
            results.resize(input.size());
            std::copy(input.begin(), input.end(), results.begin());
        }

        // This function is called to add draw commands to the command buffer after the main render
        // pass has started.
        virtual void addDrawCmdsToCommandBuffer(
                VkCommandBuffer const &commandBuffer,
                size_t descriptorSetID,
                std::shared_ptr<CommonObjectData> const &commonObjectData,
                std::shared_ptr<DrawObjectTableVulkan> const &drawObjTable,
                std::vector<size_t> const &drawObjectsIndices) = 0;

        enum DrawIfHasTexture {
            ONLY_IF_NO_TEXTURE,
            ONLY_IF_TEXTURE,
            BOTH
        };

        void initializeCommandBufferDrawObjects(
            DrawIfHasTexture drawIf,
            size_t descriptorSetID,
            std::shared_ptr<vulkan::Pipeline> const &pipeline,
             VkCommandBuffer const &commandBuffer,
            std::shared_ptr<DrawObjectTableVulkan> const &drawObjectTable,
            std::vector<size_t> const &drawObjectsIndices);

        virtual bool overrideClearColor(glm::vec4 &) {
            return false;
        }

        virtual void reload(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
                std::shared_ptr<renderDetails::Parameters> const &parameters) = 0;

        virtual std::shared_ptr<vulkan::Device> const &device() = 0;

        RenderDetailsVulkan(uint32_t width, uint32_t height)
                : RenderDetails(width, height)
        {}

        ~RenderDetailsVulkan() override  = default;
    protected:
        VkVertexInputBindingDescription getBindingDescription();

        std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };

    using ReferenceVulkan = renderDetails::Reference<RenderDetailsVulkan, levelDrawer::TextureDataVulkan, renderDetails::DrawObjectDataVulkan>;
}
#endif // AMAZING_LABYRINTH_RENDER_DETAILS_VULKAN_HPP
