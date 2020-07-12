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

#include "../../levelDrawer/levelDrawerVulkan.hpp"
#include "../objectWithShadows/renderDetailsVulkan.hpp"
#include "../renderDetailsVulkan.hpp"
#include "../../renderLoader/renderLoaderVulkan.hpp"
#include "config.hpp"

namespace shadowsChaining {
    class RenderDetailsVulkan;

    class CommonObjectDataVulkan : renderDetails::CommonObjectData {
        friend RenderDetailsVulkan;
    private:
        std::shared_ptr<objectWithShadows::CommonObjectDataVulkan> m_objectWithShadowsCOD;
        std::shared_ptr<shadows::CommonObjectDataVulkan> m_shadowsCOD;
    };

    class DrawObjectDataVulkan : public renderDetails::DrawObjectDataVulkan {
    public:
        bool hasTexture() override { return m_mainDrawObjectData->hasTexture(); }

        std::shared_ptr<vulkan::Buffer> const &bufferModelMatrix() override {
            return m_mainDrawObjectData->bufferModelMatrix();
        }

        void update(glm::mat4 const &modelMatrix) override {
            // the main draw object data and the shadows draw object data share a buffer.
            // So it is only necessary to update one.
            m_mainDrawObjectData.update(modelMatrix);
        }

        std::shared_ptr<vulkan::DescriptorSet> const &descriptorSet(uint32_t id) override {
            switch (id) {
                case 0:
                    return m_mainDrawObjectData->descriptorSet(id);
                case 1:
                default:
                    return m_shadowsDrawObjectData->descriptorSet(id);
            }
        }

        DrawObjectDataVulkan(
                std::shared_ptr<renderDetails::DrawObjectDataVulkan> inMainDrawObjectData,
                std::shared_ptr<renderDetails::DrawObjectDataVulkan> inShadowsDrawObjectData)
                : renderDetails::DrawObjectDataVulkan(),
                m_mainDrawObjectData{std::move(inMainDrawObjectData)},
                m_shadowsDrawObjectData{std::move(inShadowsDrawObjectData)}
        {}

        ~DrawObjectDataVulkan() override = default;
    private:
        std::shared_ptr<objectWithShadows::DrawObjectDataVulkan> m_mainDrawObjectData;
        std::shared_ptr<shadows::DrawObjectDataVulkan> m_shadowsDrawObjectData;
    };

    class RenderDetailsVulkan : public renderDetails::RenderDetailsVulkan {
    public:
        static char const *name() { return m_name; }

        static renderDetails::ReferenceVulkan loadNew(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
            std::shared_ptr<vulkan::Device> const &inDevice,
            renderDetails::ParametersVulkan const &parameters,
            Config const &config);

        static renderDetails::ReferenceVulkan loadExisting(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
            std::shared_ptr<renderDetails::RenderDetailsVulkan> const &rdBase,
            renderDetails::ParametersVulkan const &parameters,
            Config const &config);

        void addDrawCmdsToCommandBuffer(
            VkCommandBuffer const &commandBuffer,
            VkFrameBuffer const &frameBuffer,
            size_t descriptorSetID,
            levelDrawer::LevelDrawerVulkan::CommonObjectDataList const &commonObjectDataList,
            levelDrawer::LevelDrawerVulkan::DrawObjectTables const &drawObjTable,
            levelDrawer::LevelDrawerVulkan::IndicesForDrawing const &drawObjectsIndicesList) override;

    private:
        static char constexpr const *m_name = "shadowsChaining";

        // use less precision for the shadow buffer
        static float constexpr shadowsSizeMultiplier = 0.5f;

        std::shared_ptr<vulkan::Device> m_device;
        std::shared_ptr<objectWithShadows:::RenderDetailsVulkan> m_objectWithShadowsRenderDetails;

        std::shared_ptr<vulkan::ImageView> m_depthImageViewShadows;
        std::shared_ptr<vulkan::ImageView> m_shadowsColorAttachment;
        std::shared_ptr<vulkan::ImageSampler> m_samplerShadows;
        std::shared_ptr<shadows::RenderDetailsVulkan> m_shadowsRenderDetails;
        std::shared_ptr<vulkan::Framebuffer> m_framebufferShadows;

        RenderDetailsVulkan(
            uint32_t inWidth,
            uint32_t inHeight)
            : renderDetails::RenderDetailsVulkan{inWidth, inHeight}
        {}

        static uint32_t getShadowsFramebufferDimension(uint32_t dimension) {
            return static_cast<uint32_t>(std::floor(dimension * shadowsSizeMultiplier));
        }

        static renderDetails::ReferenceVulkan createReference(
                std::shared_ptr<RenderDetailsVulkan> rd,
                renderDetails::ReferenceVulkan const &refShadows,
                renderDetails::ReferenceVulkan const &refObjectWithShadows,
                std::shared_ptr<vulkan::ImageSampler> const &shadowsImageSampler);

        static renderDetails::ParametersVulkan createShadowParameters(
                std::shared_ptr<RenderDetailsVulkan> const &rd,
                renderDetails::ParametersVulkan const &parameters)
        {
            renderDetails::ParametersVulkan shadowParameters;
            shadowParameters.width = getShadowsFramebufferDimension(parameters.width);
            shadowParameters.height = getShadowsFramebufferDimension(parameters.height);
            shadowParameters.preTransform = parameters.preTransform;
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
            return std::move(shadowParameters);
        }
    };
}

#endif // AMAZING_LABYRINTH_SHADOWSCHAINING_RENDER_DETAILS_VULKAN_HPP
