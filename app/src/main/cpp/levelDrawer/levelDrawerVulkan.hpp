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
#include <array>

#include "levelDrawerGraphics.hpp"
#include "../graphicsVulkan.hpp"
#include "modelTable/modelTableVulkan.hpp"
#include "textureTable/textureTableVulkan.hpp"
#include "drawObjectTable/drawObjectTable.hpp"
#include "../renderLoader/renderLoaderVulkan.hpp"

namespace renderDetails {
    class RenderDetailsVulkan;
    class CommonObjectData;
    class DrawObjectDataVulkan;
    class ParametersVulkan;
    using ReferenceVulkan = Reference<RenderDetailsVulkan, CommonObjectData, DrawObjectDataVulkan>;
}

namespace levelDrawer {
    struct DrawObjectVulkanTraits {
        using RenderDetailsType = renderDetails::RenderDetailsVulkan;
        using CommonObjectDataType = renderDetails::CommonObjectData;
        using RenderDetailsReferenceType = renderDetails::ReferenceVulkan;
        using ModelDataType = ModelDataVulkan;
        using TextureDataType = TextureDataVulkan;
        using DrawObjectDataType = renderDetails::DrawObjectDataVulkan;
    };

    using DrawObjectTableVulkan = DrawObjectTable<DrawObjectVulkanTraits>;
    struct LevelDrawerVulkanTraits {
        using RenderLoaderType = RenderLoaderVulkan;
        using RenderDetailsType = renderDetails::RenderDetailsVulkan;
        using CommonObjectDataType = renderDetails::CommonObjectData;
        using RenderDetailsReferenceType = renderDetails::ReferenceVulkan;
        using ModelTableType = ModelTableVulkan;
        using TextureTableType = TextureTableVulkan;
        using DrawObjectTableType = DrawObjectTableVulkan;
        using DrawArgumentType = VkCommandBuffer;
    };

    using LevelDrawerVulkan = LevelDrawerGraphics<LevelDrawerVulkanTraits>;
}
#endif // AMAZING_LABYRINTH_LEVEL_DRAWER_VULKAN_HPP
