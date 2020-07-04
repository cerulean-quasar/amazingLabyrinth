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
}