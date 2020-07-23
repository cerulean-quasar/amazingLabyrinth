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
#include "drawObjectTable/drawObjectTableVulkan.hpp"

namespace renderDetails {
    class RenderDetailsVulkan;
    class CommonObjectData;
    class DrawObjectDataVulkan;
    class ParametersVulkan;
    using ReferenceVulkan = Reference<RenderDetailsVulkan, CommonObjectData, DrawObjectDataVulkan>;
}

class RenderLoaderVulkan;
namespace levelDrawer {
    struct NeededForDrawingVulkan {
        std::shared_ptr<vulkan::Device> device;
        std::shared_ptr<vulkan::CommandPool> commandPool;
    };

    struct DrawArgumentVulkan {
        VkCommandBuffer cmdBuffer;
        VkFramebuffer framebuffer;
        std::shared_ptr<vulkan::RenderPass> renderPass;
        VkExtent2D extent;
    };

    using DrawObjectTableVulkan = DrawObjectTable<DrawObjectVulkanTraits>;
    struct LevelDrawerVulkanTraits {
        using DrawRuleType = DrawObjectTableVulkan::DrawRule;
        using RenderLoaderType = RenderLoaderVulkan;
        using RenderDetailsType = renderDetails::RenderDetailsVulkan;
        using CommonObjectDataType = renderDetails::CommonObjectData;
        using RenderDetailsReferenceType = renderDetails::ReferenceVulkan;
        using ModelTableType = ModelTableVulkan;
        using TextureTableType = TextureTableVulkan;
        using DrawObjectTableType = DrawObjectTableVulkan;
        using DrawArgumentType = DrawArgumentVulkan;
        using NeededForDrawingType = NeededForDrawingVulkan;
    };

    using LevelDrawerVulkan = LevelDrawerGraphics<LevelDrawerVulkanTraits>;

    template <>
    void LevelDrawerGraphics<LevelDrawerVulkanTraits>::draw(
            LevelDrawerVulkanTraits::DrawArgumentType const &info);

    template <>
    void LevelDrawerGraphics<LevelDrawerVulkanTraits>::drawToBuffer(
            std::string const &renderDetailsName,
            ModelsTextures const &modelsTextures,
            std::vector<glm::mat4> const &modelMatrix,
            float width,
            float height,
            uint32_t nbrSamplesForWidth,
            float farthestDepth,
            float nearestDepth,
            std::vector<float> &results);

    template <>
    LevelDrawerGraphics<LevelDrawerVulkanTraits>::LevelDrawerGraphics(
            LevelDrawerVulkanTraits::NeededForDrawingType neededForDrawing,
            std::shared_ptr<LevelDrawerVulkanTraits::RenderLoaderType> inRenderLoader,
            std::shared_ptr<GameRequester> inGameRequester);
}
#endif // AMAZING_LABYRINTH_LEVEL_DRAWER_VULKAN_HPP
