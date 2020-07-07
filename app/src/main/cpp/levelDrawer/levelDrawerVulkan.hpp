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
#ifndef AMAZING_LABYRINTH_LEVEL_DRAWER_VULKAN_HPP
#define AMAZING_LABYRINTH_LEVEL_DRAWER_VULKAN_HPP

#include <memory>
#include <string>
#include "levelDrawerGraphics.hpp"
#include "../graphicsVulkan.hpp"
#include "modelTable/modelTableVulkan.hpp"
#include "textureTable/textureTableVulkan.hpp"
#include "../renderDetails/renderDetailsVulkan.hpp"
#include "drawObjectTable/drawObjectTable.hpp"
#include "../renderLoader/renderLoaderVulkan.hpp"

namespace levelDrawer {
    struct DrawObjectVulkanTraits {
        using RenderDetailsType = renderDetails::RenderDetailsVulkan;
        using CommonObjectDataType = renderDetails::CommonObjectDataVulkan;
        using RenderDetailsReferenceType = RenderDetailsReference<RenderDetailsType, CommonObjectDataType>;
        using ModelDataType = ModelDataVulkan;
        using TextureDataType = TextureDataVulkan;
        using DrawObjectDataType = renderDetails::DrawObjectDataVulkan;
    };

    struct LevelDrawerVulkanTraits {
        using RenderLoaderType = RenderLoaderVulkan;
        using RenderDetailsType = renderDetails::RenderDetailsVulkan;
        using CommonObjectDataType = renderDetails::CommonObjectDataVulkan;
        using RenderDetailsReferenceType = RenderDetailsReference<RenderDetailsType, CommonObjectDataType>;
        using ModelTableType = ModelTableVulkan;
        using TextureTableType = TextureTableVulkan;
        using DrawObjectTableType = DrawObjectTable<DrawObjectVulkanTraits>;
        using DrawArgumentType = VkCommandBuffer;
    };

    using LevelDrawerVulkan = LevelDrawerGraphics<LevelDrawerVulkanTraits>;
}
#endif // AMAZING_LABYRINTH_LEVEL_DRAWER_VULKAN_HPP
