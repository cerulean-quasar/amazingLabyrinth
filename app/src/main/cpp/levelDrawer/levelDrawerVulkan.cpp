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
#include <memory>

#include "../graphicsVulkan.hpp"
#include "levelDrawerVulkan.hpp"
#include "levelDrawerGraphics.hpp"

namespace levelDrawer {
    template <>
    void LevelDrawerGraphics<LevelDrawerVulkanTraits>::draw(
            LevelDrawerVulkanTraits::DrawArgumentType info)
    {
        auto rulesList = getDrawRules();
        for (auto const &rulesIndex : std::vector<size_t>{LEVEL, STARTER, FINISHER}) {
            for (auto const &rule : rulesList[rulesIndex]) {
                rule.renderDetails->addPreRenderPassCmdsToCommandBuffer(
                        info.cmdBuffer, 0, )
            }
        }
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        /* pInheritanceInfo is only used if flags includes:
         * VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
         * because that would mean that there is a secondary command buffer that will be used
         * in a single render pass
         */
        beginInfo.pInheritanceInfo = nullptr; // Optional

        /* this call will reset the command buffer.  its not possible to append commands at
         * a later time.
         */
        vkBeginCommandBuffer(info.cmdBuffer, &beginInfo);

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {m_bgColor.r, m_bgColor.g, m_bgColor.b, m_bgColor.a};
        clearValues[1].depthStencil = {1.0f, 0};

        /* begin the render pass: drawing starts here*/
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_neededForDrawing.renderPass->renderPass().get();
        renderPassInfo.framebuffer = info.framebuffer;
        /* size of the render area */
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = info.extent;

        /* the color value to use when clearing the image with VK_ATTACHMENT_LOAD_OP_CLEAR,
         * using black with 0% opacity
         */
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        /* begin recording commands - start by beginning the render pass.
         * none of these functions returns an error (they return void).  There will be no error
         * handling until recording is done.
         */
        vkCmdBeginRenderPass(info.cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);


    }

    template <>
    LevelDrawerGraphics<LevelDrawerVulkanTraits>::LevelDrawerGraphics(
            LevelDrawerVulkanTraits::NeededForDrawingType neededForDrawing,
            std::shared_ptr<LevelDrawerVulkanTraits::RenderLoaderType> inRenderLoader,
            std::shared_ptr<GameRequester> inGameRequester)
            : m_modelTable(neededForDrawing.m_device, neededForDrawing.m_commandPool),
            m_textureTable(neededForDrawing.m_device, neededForDrawing.m_commandPool),
            m_drawObjectTableList{
                std::make_shared<LevelDrawerVulkanTraits::DrawObjectTableType>(),
                std::make_shared<LevelDrawerVulkanTraits::DrawObjectTableType>(),
                std::make_shared<LevelDrawerVulkanTraits::DrawObjectTableType>()},
            m_renderLoader(std::move(inRenderLoader)),
            m_gameRequester(std::move(inGameRequester)),
            m_neededForDrawing(std::move(neededForDrawing)),
            m_bgColor{0.0f, 0.0f, 0.0f, 1.0f}
    {}
}