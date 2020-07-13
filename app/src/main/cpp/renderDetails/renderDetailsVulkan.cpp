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

#include <vector>
#include "../graphicsVulkan.hpp"
#include "renderDetailsVulkan.hpp"

namespace renderDetails {
    VkVertexInputBindingDescription RenderDetailsVulkan::getBindingDescription() {
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

    std::vector <VkVertexInputAttributeDescription> RenderDetailsVulkan::getAttributeDescriptions() {
        std::vector <VkVertexInputAttributeDescription> attributeDescriptions = {};

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

    void RenderDetailsVulkan::initializeCommandBufferDrawObjects(
        DrawIfHasTexture drawIf,
        size_t descriptorSetID,
        std::shared_ptr<vulkan::Pipeline> const &pipeline,
        VkCommandBuffer const &commandBuffer,
        std::shared_ptr<levelDrawer::DrawObjectTableVulkan> const &drawObjectTable,
        std::vector<size_t> const &drawObjectsIndices)
    {
        if (!drawObjectTable || drawObjectsIndices.empty()) {
            return;
        }

        VkDeviceSize offsets[1] = {0};

        for (auto const &index : drawObjectsIndices) {
            auto const &drawObj = drawObjectTable->drawObject(index);
            auto const &modelData = drawObj->modelData();
            auto const &textureData = drawObj->textureData();

            if (drawIf != DrawIfHasTexture::BOTH &&
                ((drawIf == DrawIfHasTexture::ONLY_IF_NO_TEXTURE && textureData) ||
                (drawIf == DrawIfHasTexture::ONLY_IF_TEXTURE && !textureData)))
            {
                continue;
            }

            VkBuffer vertexBuffer = modelData->vertexBuffer().cbuffer();
            VkBuffer indexBuffer = modelData->indexBuffer().cbuffer();
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

            size_t nbrModelMatrices = drawObj->numberObjectsData();
            for (size_t i = 0; i < nbrModelMatrices; i++) {
                auto drawObjData = drawObj->objData(i);

                /* The MVP matrix and texture samplers */
                VkDescriptorSet descriptorSet = drawObjData->descriptorSet(
                        descriptorSetID).get();
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        pipeline->layout().get(), 0, 1, &descriptorSet, 0,
                                        nullptr);

                /* indexed draw command:
                 * parameter 1 - Command buffer for the draw command
                 * parameter 2 - the number of indices (the vertex count)
                 * parameter 3 - the instance count, use 1 because we are not using instanced rendering
                 * parameter 4 - offset into the index buffer
                 * parameter 5 - offset to add to the indices in the index buffer
                 * parameter 6 - offset for instance rendering
                 */
                vkCmdDrawIndexed(commandBuffer, modelData->numberIndices(), 1, 0, 0, 0);
            }
        }
    }

}