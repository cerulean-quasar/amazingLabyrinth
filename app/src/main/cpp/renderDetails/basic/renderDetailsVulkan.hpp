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
#include "../../graphicsVulkan.hpp"
#include "../../levelDrawer/modelTable/modelTableVulkan.hpp"
#include "renderDetailsData.hpp"

namespace renderDetails {
    class RenderDetailsVulkan : public RenderDetails {
    public:
        void draw(
            vulkan::VkCommandBuffer commandBuffer,
            DrawObjectTableVulkan const &drawObjTable,
            NidekTabkeVulkan const &modelTable,
            std::vector<size_t> const &drawObjectsIndexList) = 0;

        ~RenderDetailsVulkan() override  = default;
    protected:
        VkVertexInputBindingDescription getBindingDescription();

        std::vector <VkVertexInputAttributeDescription> getAttributeDescriptions();
    };
}
#endif // AMAZING_LABYRINTH_RENDER_DETAILS_VULKAN_HPP
