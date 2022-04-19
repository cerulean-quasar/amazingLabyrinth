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

#include <vector>
#include "../graphicsVulkan.hpp"
#include "renderDetailsVulkan.hpp"

namespace renderDetails {
    std::shared_ptr<vulkan::Buffer> createUniformBuffer(
            std::shared_ptr<vulkan::Device> const &device, size_t bufferSize) {
        return std::shared_ptr<vulkan::Buffer>{new vulkan::Buffer{device, bufferSize,
                                                                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};
    }

    VkVertexInputBindingDescription RenderDetailsVulkan::getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};

        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(levelDrawer::Vertex);

        /* move to the next data entry after each vertex.  VK_VERTEX_INPUT_RATE_INSTANCE
         * moves to the next data entry after each instance, but we are not using instanced
         * rendering
         */
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    std::vector <VkVertexInputAttributeDescription> RenderDetailsVulkan::getAttributeDescriptions() {
        std::vector <VkVertexInputAttributeDescription> attributeDescriptions = {};

        attributeDescriptions.resize(4);

        /* position */
        attributeDescriptions[0].binding = 0; /* binding description to use */
        attributeDescriptions[0].location = 0; /* matches the location in the vertex shader */
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(levelDrawer::Vertex, pos);

        /* color */
        attributeDescriptions[1].binding = 0; /* binding description to use */
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(levelDrawer::Vertex, color);

        /* texture coordinate */
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(levelDrawer::Vertex, texCoord);

        /* normal vector */
        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(levelDrawer::Vertex, normal);
        return attributeDescriptions;
    }

    void RenderDetailsVulkan::initializeCommandBufferDrawObjects(
            VkCommandBuffer const &commandBuffer,
            size_t descriptorSetID,
            std::shared_ptr<vulkan::Pipeline> const &colorPipeline,
            std::shared_ptr<vulkan::Pipeline> const &texturePipeline,
            std::shared_ptr<levelDrawer::DrawObjectTableVulkan> const &drawObjectTable,
            std::set<levelDrawer::ZValueReference>::iterator beginZValRefs,
            std::set<levelDrawer::ZValueReference>::iterator endZValRefs,
            bool useVertexNormals)
    {
        if (!drawObjectTable || beginZValRefs == endZValRefs) {
            return;
        }

        VkDeviceSize offsets[1] = {0};

        std::pair<levelDrawer::DrawObjReference, levelDrawer::DrawObjDataReference> prev;
        uint32_t nbrIndices = 0;
        bool usingColorPipeline = true;
        for (auto it = beginZValRefs; it != endZValRefs; it++) {
            auto const &drawObj = drawObjectTable->drawObject(it->drawObjectReference);
            if (nbrIndices == 0 ||
                it->drawObjectReference != prev.first || it->drawObjectDataReference != prev.second)
            {
                auto const &modelData = drawObj->modelData();
                auto const &textureData = drawObj->textureData();

                if (nbrIndices == 0) {
                    /* bind the graphics pipeline to the command buffer, the second parameter tells Vulkan
                     * that we are binding to a graphics pipeline.
                     */
                    if (texturePipeline && textureData) {
                        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                          getVkType<>(texturePipeline->pipeline().get()));
                        usingColorPipeline = false;
                    } else {
                        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                          getVkType<>(colorPipeline->pipeline().get()));
                        usingColorPipeline = true;
                    }
                } else if (texturePipeline && textureData && usingColorPipeline) {
                    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      getVkType<>(texturePipeline->pipeline().get()));
                    usingColorPipeline = false;
                } else if (texturePipeline && !textureData && !usingColorPipeline) {
                    // if we have a texture pipeline, we need to check to see if we should switch
                    // to the color pipeline.  If we only have one pipeline (the color pipeline),
                    // then we are already bound to it (when nbrIndices was 0), so no need to bind
                    // here again.
                    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      getVkType<>(colorPipeline->pipeline().get()));
                    usingColorPipeline = true;
                }

                VkBuffer vertexBuffer = useVertexNormals ?
                                        modelData->vertexBufferWithVertexNormals()->cbuffer() :
                                        modelData->vertexBuffer()->cbuffer();

                VkBuffer indexBuffer = useVertexNormals ?
                                       modelData->indexBufferWithVertexNormals()->cbuffer() :
                                       modelData->indexBuffer()->cbuffer();

                nbrIndices = useVertexNormals ?
                                      modelData->numberIndicesWithVertexNormals() :
                                      modelData->numberIndices();

                vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);
                vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

                prev.first = it->drawObjectReference;
                prev.second = it->drawObjectDataReference.get();
            }

            auto const &drawObjData = drawObj->objData(it->drawObjectDataReference.get());

            VkPipelineLayout pipelineLayout =
                    usingColorPipeline ? getVkType<>(colorPipeline->layout().get()) : getVkType<>(texturePipeline->layout().get());

            /* The MVP matrix and texture samplers */
            VkDescriptorSet descriptorSet =
                    getVkType<>(drawObjData->descriptorSet(descriptorSetID)->descriptorSet().get());
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipelineLayout, 0, 1, &descriptorSet, 0,
                                    nullptr);

            /* indexed draw command:
             * parameter 1 - Command buffer for the draw command
             * parameter 2 - the number of indices (the vertex count)
             * parameter 3 - the instance count, use 1 because we are not using instanced rendering
             * parameter 4 - offset into the index buffer
             * parameter 5 - offset to add to the indices in the index buffer
             * parameter 6 - offset for instance rendering
             */
            vkCmdDrawIndexed(commandBuffer, nbrIndices, 1, 0, 0, 0);
        }
    }
}