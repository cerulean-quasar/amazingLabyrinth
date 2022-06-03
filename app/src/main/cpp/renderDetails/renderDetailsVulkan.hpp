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

#ifndef AMAZING_LABYRINTH_RENDER_DETAILS_VULKAN_HPP
#define AMAZING_LABYRINTH_RENDER_DETAILS_VULKAN_HPP

#include <memory>
#include <vector>
#include <set>
#include <glm/glm.hpp>

#include "../graphicsVulkan.hpp"
#include "../levelDrawer/modelTable/modelTableVulkan.hpp"
#include "renderDetails.hpp"
#include "../levelDrawer/drawObjectTable/drawObjectTable.hpp"
#include "../levelDrawer/drawObjectTable/drawObjectTableVulkan.hpp"
#include "../levelDrawer/common.hpp"

class RenderLoaderVulkan;
namespace renderDetails {
    std::shared_ptr<vulkan::Buffer> createUniformBuffer(
            std::shared_ptr<vulkan::Device> const &device, size_t bufferSize);

    struct ParametersObjectWithShadowsVulkan : public ParametersObjectWithShadows {
        std::shared_ptr<vulkan::ImageSampler> shadowsSampler;

        ParametersObjectWithShadowsVulkan(ParametersObjectWithShadows const *parameters, std::shared_ptr<vulkan::ImageSampler> sampler)
            : ParametersObjectWithShadows(*parameters),
            shadowsSampler(std::move(sampler)) {
        }
        virtual ~ParametersObjectWithShadowsVulkan() = default;
    };

    class DrawObjectDataVulkan : public DrawObjectData {
    public:
        virtual bool hasTexture() = 0;

        virtual bool updateTextureData(
                std::shared_ptr<CommonObjectData> const &,
                std::shared_ptr<levelDrawer::TextureDataVulkan> const &)
        {
            // The default behavior for this function should be that it fails (returns false).
            // For certain Render Details, this function will succeed.  It means that it was
            // possible to update the descriptor set with the new texture value making a change
            // in texture possible without reallocating the descriptor or reallocating the memory
            // for the uniform buffers used.
            return false;
        }

        // used to update the model matrix but not the buffer.  This function will be used if
        // the buffer is shared with another DrawObjectData (as in objectWithShadows and shadows).
        virtual void updateModelMatrixNoBufferUpdate(glm::mat4 const &modelMatrix) = 0;

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
                levelDrawer::CommonObjectDataList const &/* common object data */,
                levelDrawer::DrawObjectTableVulkanList const &/* draw object table */,
                std::set<levelDrawer::ZValueReference> const & /* starter */,
                std::set<levelDrawer::ZValueReference> const & /* level */,
                std::set<levelDrawer::ZValueReference> const & /* finisher */)
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
                std::shared_ptr<levelDrawer::DrawObjectTableVulkan> const &drawObjTable,
                std::set<levelDrawer::ZValueReference>::iterator beginZValRefs,
                std::set<levelDrawer::ZValueReference>::iterator endZValRefs) = 0;

        void initializeCommandBufferDrawObjects(
                VkCommandBuffer const &commandBuffer,
                size_t descriptorSetID,
                std::shared_ptr<vulkan::Pipeline> const &colorPipeline,
                std::shared_ptr<vulkan::Pipeline> const &texturePipeline,
                std::shared_ptr<levelDrawer::DrawObjectTableVulkan> const &drawObjectTable,
                std::set<levelDrawer::ZValueReference>::iterator beginZValRefs,
                std::set<levelDrawer::ZValueReference>::iterator endZValRefs,
                bool useVertexNormals = false);

        virtual bool overrideClearColor(glm::vec4 &) {
            return false;
        }

        bool structuralChangeNeededHelper(
                std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails,
                std::shared_ptr<vulkan::RenderPass> const &existingRenderPass)
        {
            if (surfaceDetails->surfaceWidth != m_surfaceWidth ||
                surfaceDetails->surfaceHeight != m_surfaceHeight) {
                return true;
            }

            if (existingRenderPass.get() == surfaceDetails->renderPass.get()) {
                return false;
            }

            return surfaceDetails->renderPass->isCompatible(existingRenderPass);
        }

        virtual bool structuralChangeNeeded(
                std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails) = 0;

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
