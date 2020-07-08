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
#ifndef AMAZING_LABYRINTH_SHADOWSCHAINING_RENDER_DETAILS_VULKAN_HPP
#define AMAZING_LABYRINTH_SHADOWSCHAINING_RENDER_DETAILS_VULKAN_HPP

#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../textureWithShadows/renderDetailsDataVulkan.hpp"
#include "../renderDetailsVulkan.hpp"
#include "../../renderLoader/renderLoaderVulkan.hpp"

namespace chainingShadows {
    class RenderDetailsVulkan;

    class CommonObjectDataVulkan : renderDetails::CommonObjectData {
        friend RenderDetailsVulkan;
    private:
        std::shared_ptr<textureWithShadows::CommonObjectDataVulkan> m_textureCOD;
        std::shared_ptr<colorWithShadows::CommonObjectDataVulkan> m_colorCOD;
        std::shared_ptr<shadows::CommonObjectDataVulkan> m_shadowsCOD;
    };

    class DrawObjectDataVulkan : renderDetails::DrawObjectDataVulkan {
        friend RenderDetailsVulkan;
    private:
        bool m_hasTexture;
        std::shared_ptr<renderDetails::DrawObjectDataVulkan> m_mainDrawObjectData;
        std::shared_ptr<shadows::DrawObjectDataVulkan> m_shadowsDrawObjectData;
    };

    class RenderDetailsVulkan : public renderDetails::RenderDetailsVulkan {
    public:
        static renderDetails::ReferenceVulkan loadNew(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
            std::shared_ptr<vulkan::Device> const &inDevice,
            renderDetails::RenderDetailsParametersVulkan const &parameters)
        {
            // initialize main render details
            auto rd = std::make_shared<RenderDetailsVulkan>();
            rd->m_width = parameters.width;
            rd->m_height = parameters.height;
            rd->m_device = inDevice;
            renderDetails::RenderDetailsParametersVulkan shadowParameters;
            shadowParameters.width = getShadowsFramebufferDimension(parameters.width);
            shadowParameters.height = getShadowsFramebufferDimension(parameters.height);
            shadowParameters.preTransform = parameters.preTransform;

            rd->m_depthImageViewShadows = std::make_shared<vulkan::ImageView>(
                    vulkan::ImageFactory::createDepthImage(
                            inDevice,
                            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                            shadowParameters.width,
                            shadowParameters.height),
                    VK_IMAGE_ASPECT_DEPTH_BIT);
            rd->m_shadowsColorAttachment = vulkan::ImageView::createImageViewAndImage(
                    inDevice,
                    getShadowsFramebufferWidth(parameters.width),
                    getShadowsFramebufferHeigth(parameters.height),
                    VK_FORMAT_R32G32B32A32_SFLOAT,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    VK_IMAGE_ASPECT_COLOR_BIT);
            rd->m_samplerShadows = std::make_shared<vulkan::ImageSampler>(
                    inDevice, rd->m_shadowsColorAttachment,
                    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                    VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);

            shadowParameters.colorImageInfo =
                    std::vector<vulkan::RenderPass::ImageAttachmentInfo>{vulkan::RenderPass::ImageAttachmentInfo{
                            VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
                            rd->m_shadowsColorAttachment->image()->format(),
                            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}};
            shadowParameters.depthImageInfo =
                    std::make_shared<vulkan::RenderPass::ImageAttachmentInfo>(
                            VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
                            rd->m_depthImageViewShadows->image()->format(),
                            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

            auto refShadows = renderLoader->load(gameRequester,
                    shadows::RenderDetailsVulkan::name(), shadowParameters);

            rd->m_framebufferShadows = std::make_shared<vulkan::Framebuffer>(
                    inDevice, refShadows.renderDetails->renderPass(),
                    std::vector<std::shared_ptr<vulkan::ImageView>>{rd->m_shadowsColorAttachment,
                                                                    rd->m_depthImageViewShadows},
                    shadowParameters.width, shadowParameters.height);

            auto refTexture = renderLoader->load(gameRequester,
                    textureWithShadows::RenderDetailsVulkan::name(), parameters);
            auto refColor = renderLoader->load(gameRequester,
                    colorWithShadows::RenderDetailsVulkan::name(), parameters);

            auto cod = std::make_shared<CommonObjectDataVulkan>();
            cod->m_textureCOD = refTexture.commonObjectData;
            cod->m_colorCOD = refColor.commonObjectData;
            cod->m_shadowsCOD = refShadows.commonObjectData;

            renderDetails::ReferenceVulkan ref;
            ref.renderDetails = rd;
            ref.commonObjectData = cod;

            return std::move(ref);
        }

    private:
        // use less precision for the shadow buffer
        static float constexpr shadowsSizeMultiplier = 0.5f;

        uint32_t m_width;
        uint32_t m_height;
        std::shared_ptr<vulkan::Device> m_device;
        std::shared_ptr<colorWithShadows::RenderDetailsVulkan> m_colorRenderDetails;
        std::shared_ptr<textureWithShadows::RenderDetailsVulkan> m_textureRenderDetails;

        std::shared_ptr<vulkan::ImageView> m_depthImageViewShadows;
        std::shared_ptr<vulkan::ImageView> m_shadowsColorAttachment;
        std::shared_ptr<vulkan::ImageSampler> m_samplerShadows;
        std::shared_ptr<shadows::RenderDetailsVulkan> m_shadowsRenderDetails;
        std::shared_ptr<vulkan::Framebuffer> m_framebufferShadows;

        RenderDetailsVulkan() = default;

        static uint32_t getShadowsFramebufferDimension(uint32_t dimension) {
            return static_cast<uint32_t>(std::floor(dimension * shadowsSizeMultiplier));
        }
    };
}

#endif // AMAZING_LABYRINTH_SHADOWSCHAINING_RENDER_DETAILS_VULKAN_HPP
