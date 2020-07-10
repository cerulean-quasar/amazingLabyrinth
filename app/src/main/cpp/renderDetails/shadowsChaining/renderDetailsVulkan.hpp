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

#include "../textureWithShadows/renderDetailsVulkan.hpp"
#include "../colorWithShadows/renderDetailsVulkan.hpp"
#include "../renderDetailsVulkan.hpp"
#include "../../renderLoader/renderLoaderVulkan.hpp"
#include "config.hpp"

namespace shadowsChaining {
    class RenderDetailsVulkan;

    class CommonObjectDataVulkan : renderDetails::CommonObjectData {
        friend RenderDetailsVulkan;
    private:
        std::shared_ptr<textureWithShadows::CommonObjectDataVulkan> m_textureCOD;
        std::shared_ptr<colorWithShadows::CommonObjectDataVulkan> m_colorCOD;
        std::shared_ptr<shadows::CommonObjectDataVulkan> m_shadowsCOD;
    };

    class DrawObjectDataVulkan : public renderDetails::DrawObjectDataVulkan {
    public:
        bool hasTexture() { return m_hasTexture; }

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
        bool m_hasTexture;
        std::shared_ptr<renderDetails::DrawObjectDataVulkan> m_mainDrawObjectData;
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
            Config const &config)
        {
            // initialize main render details
            auto rd = std::make_shared<RenderDetailsVulkan>(parameters.width, parameters.height);
            rd->m_device = inDevice;

            // shadow resources
            rd->m_depthImageViewShadows = std::make_shared<vulkan::ImageView>(
                    vulkan::ImageFactory::createDepthImage(
                            inDevice,
                            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                            getShadowsFramebufferWidth(parameters.width),
                            getShadowsFramebufferHeigth(parameters.height)),
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

            auto shadowParameters = createShadowParameters(rd, parameters);

            // shadows render details
            auto refShadows = renderLoader->load(
                    gameRequester, shadows::RenderDetailsVulkan::name(), shadowParameters);

            // shadows framebuffer
            rd->m_framebufferShadows = std::make_shared<vulkan::Framebuffer>(
                    inDevice, refShadows.renderDetails->renderPass(),
                    std::vector<std::shared_ptr<vulkan::ImageView>>{rd->m_shadowsColorAttachment,
                                                                    rd->m_depthImageViewShadows},
                    shadowParameters.width, shadowParameters.height);

            // texture render details
            auto refTexture = renderLoader->load(
                    gameRequester, textureWithShadows::RenderDetailsVulkan::name(), parameters);

            // color render details
            auto refColor = renderLoader->load(
                    gameRequester, colorWithShadows::RenderDetailsVulkan::name(), parameters);

            rd->m_textureRenderDetails = refTexture.renderDetails;
            rd->m_colorRenderDetails = refColor.renderDetails;
            rd->m_shadowsRenderDetails = refShadows.renderDetails;

            return createReference(std::move(rd), refShadows, refTexture, refColor);
        }

        static renderDetails::ReferenceVulkan loadExisting(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
            std::shared_ptr<renderDetails::RenderDetailsVulkan> const &rdBase,
            renderDetails::ParametersVulkan const &parameters,
            Config const &config)
        {
            auto rd = dynamic_cast<RenderDetailsVulkan*>(rdBase.get());
            if (rd == nullptr) {
                throw std::runtime_error("Invalid render details type.")
            }
            auto shadowParameters = createShadowParameters(rd, parameters);

            // shadows render details
            auto refShadows = renderLoader->load(
                gameRequester, renderLoader, shadows::RenderDetailsVulkan::name(), shadowParameters);

            // texture render details
            auto refTexture = renderLoader->load(
                gameRequester, renderLoader, textureWithShadows::RenderDetailsVulkan::name(), parameters);

            // color render details
            auto refColor = renderLoader->load(
                gameRequester, renderLoader, colorWithShadows::RenderDetailsVulkan::name(), parameters);

            rd->m_textureRenderDetails = refTexture.renderDetails;
            rd->m_colorRenderDetails = refColor.renderDetails;
            rd->m_shadowsRenderDetails = refShadows.renderDetails;

            return createReference(std::move(rdBase), refShadows, refTexture, refColor);
        }

    private:
        static char constexpr const *m_name = "shadowsChaining";

        // use less precision for the shadow buffer
        static float constexpr shadowsSizeMultiplier = 0.5f;

        std::shared_ptr<vulkan::Device> m_device;
        std::shared_ptr<colorWithShadows::RenderDetailsVulkan> m_colorRenderDetails;
        std::shared_ptr<textureWithShadows::RenderDetailsVulkan> m_textureRenderDetails;

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
                renderDetails::ReferenceVulkan const &refTexture,
                renderDetails::ReferenceVulkan const &refColor,
                std::shared_ptr<vulkan::ImageSampler> const &shadowsImageSampler)
        {
            auto cod = std::make_shared<CommonObjectDataVulkan>();
            cod->m_textureCOD = refTexture.commonObjectData;
            cod->m_colorCOD = refColor.commonObjectData;
            cod->m_shadowsCOD = refShadows.commonObjectData;

            // set the shadows image sampler
            auto codTexture = dynamic_cast<textureWithShadows::CommonObjectDataVulkan*>(refTexture.commonObjectData.get());
            if (codTexture == nullptr) {
                throw std::runtime_error("Invalid common object data");
            }
            codTexture->setShadowsImageSampler(shadowsImageSampler);

            auto codColor = dynamic_cast<colorWithShadows::CommonObjectDataVulkan*>(refTexture.commonObjectData.get());
            if (codColor == nullptr) {
                throw std::runtime_error("Invalid common object data");
            }
            codColor->setShadowsImageSampler(rd->m_samplerShadows);

            renderDetails::ReferenceVulkan ref;
            ref.renderDetails = rd;
            ref.commonObjectData = cod;
            ref.createDrawObjectData = renderDetails::ReferenceVulkan::CreateDrawObjectData(
                    [createDODShadows(refShadows.createDrawObjectData),
                            createDODTexture(refTexture.createDrawObjectData),
                            createDODColor(refColor.createDrawObjectData)] (
                            std::shared_ptr<renderDetails::DrawObjectDataVulkan> const &sharingDOD,
                            std::shared_ptr<TextureData> const &textureData,
                            glm::mat4 const &modelMatrix) ->
                            std::shared_ptr<renderDetails::DrawObjectDataVulkan>
                    {
                        std::shared_ptr<renderDetails::DrawObjectDataVulkan> dodMain;
                        if (textureData) {
                            dodMain = createDODTexture(sharingDOD, textureData, modelMatrix);
                        } else {
                            dodMain = createDODColor(sharingDOD, textureData, modelMatrix);
                        }
                        auto dodShadows = createDODShadows(dodMain, std::shared_ptr<levelDrawer::TextureData>(), modelMatrix);

                        return std::make_shared<DrawObjectDataVulkan>(dodMain, dodShadows);
                    }
            )

            return std::move(ref);
        }

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
