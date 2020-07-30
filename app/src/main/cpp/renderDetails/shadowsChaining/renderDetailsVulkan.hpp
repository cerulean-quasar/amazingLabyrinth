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
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../objectWithShadows/renderDetailsVulkan.hpp"
#include "../renderDetailsVulkan.hpp"
#include "../../levelDrawer/drawObjectTable/drawObjectTableVulkan.hpp"
#include "config.hpp"

namespace shadowsChaining {
    class RenderDetailsVulkan;

    class CommonObjectDataVulkan : public renderDetails::CommonObjectData {
        friend RenderDetailsVulkan;
    public:
        std::pair<glm::mat4, glm::mat4> getProjViewForLevel() override {
            return m_objectWithShadowsCOD->getProjViewForLevel();
        }

        glm::mat4 getViewLightSource() override {
            return m_shadowsCOD->view();
        }

        glm::vec3 getLightSource() override {
            return m_shadowsCOD->viewPoint();
        }

        std::shared_ptr<objectWithShadows::CommonObjectDataVulkan> const &objectWithShadowsCOD() {
            return m_objectWithShadowsCOD;
        }

        ~CommonObjectDataVulkan() override = default;
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

        bool updateTextureData(
                std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
                std::shared_ptr<levelDrawer::TextureDataVulkan> const &textureData) override
        {
            auto cod = dynamic_cast<CommonObjectDataVulkan*>(commonObjectData.get());
            return m_mainDrawObjectData->updateTextureData(cod->objectWithShadowsCOD(), textureData);
        }

        void update(glm::mat4 const &modelMatrix) override {
            // the main draw object data and the shadows draw object data share a buffer.
            // So it is only necessary to update one.
            m_mainDrawObjectData->update(modelMatrix);
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
        std::shared_ptr<renderDetails::DrawObjectDataVulkan> m_mainDrawObjectData;
        std::shared_ptr<renderDetails::DrawObjectDataVulkan> m_shadowsDrawObjectData;
    };

    class RenderDetailsVulkan : public renderDetails::RenderDetailsVulkan {
    public:
        std::string nameString() override { return name(); }
        static char const *name() { return shadowsChainingRenderDetailsName; }

        static renderDetails::ReferenceVulkan loadNew(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
            std::shared_ptr<vulkan::Device> const &inDevice,
            std::shared_ptr<renderDetails::Parameters> const &parameters,
            Config const &config);

        static renderDetails::ReferenceVulkan loadExisting(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
            std::shared_ptr<renderDetails::RenderDetailsVulkan> rdBase,
            std::shared_ptr<renderDetails::Parameters> const &parameters,
            Config const &config);

        void reload(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
                std::shared_ptr<renderDetails::Parameters> const &parameters) override;

        void addPreRenderPassCmdsToCommandBuffer(
                VkCommandBuffer const &commandBuffer,
                size_t /* descriptor set ID, not used */,
                renderDetails::CommonObjectDataList const &commonObjectDataList,
                renderDetails::DrawObjectTableVulkanList const &drawObjTableList,
                renderDetails::DrawObjRefsForDrawList const &drawObjectsIndicesList) override;

        void addDrawCmdsToCommandBuffer(
                VkCommandBuffer const &commandBuffer,
                size_t descriptorSetID,
                std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
                std::shared_ptr<renderDetails::DrawObjectTableVulkan> const &drawObjTable,
                std::vector<renderDetails::DrawObjReference> const &drawObjectsIndices) override;

        std::shared_ptr<vulkan::Device> const &device() override { return m_objectWithShadowsRenderDetails->device(); }

        RenderDetailsVulkan(
                uint32_t inWidth,
                uint32_t inHeight)
                : renderDetails::RenderDetailsVulkan{inWidth, inHeight}
        {}

        ~RenderDetailsVulkan() override = default;

    private:
        // use less precision for the shadow buffer
        static float constexpr shadowsSizeMultiplier = 0.5f;

        std::shared_ptr<vulkan::Device> m_device;
        std::shared_ptr<renderDetails::RenderDetailsVulkan> m_objectWithShadowsRenderDetails;

        std::shared_ptr<vulkan::ImageView> m_depthImageViewShadows;
        std::shared_ptr<vulkan::ImageView> m_shadowsColorAttachment;
        std::shared_ptr<vulkan::ImageSampler> m_samplerShadows;
        std::shared_ptr<vulkan::RenderPass> m_renderPassShadows;
        std::shared_ptr<renderDetails::RenderDetailsVulkan> m_shadowsRenderDetails;
        std::shared_ptr<vulkan::Framebuffer> m_framebufferShadows;

        static uint32_t getShadowsFramebufferDimension(uint32_t dimension) {
            return static_cast<uint32_t>(std::floor(dimension * shadowsSizeMultiplier));
        }

        static renderDetails::ReferenceVulkan createReference(
                std::shared_ptr<renderDetails::RenderDetailsVulkan> rd,
                renderDetails::ReferenceVulkan const &refShadows,
                renderDetails::ReferenceVulkan const &refObjectWithShadows,
                std::shared_ptr<vulkan::ImageSampler> const &shadowsImageSampler);

        static std::shared_ptr<renderDetails::Parameters> createShadowParameters(
                RenderDetailsVulkan const *rd,
                renderDetails::ParametersVulkan const *parameters)
        {
            renderDetails::ParametersVulkan shadowParameters;
            shadowParameters.width = getShadowsFramebufferDimension(parameters->width);
            shadowParameters.height = getShadowsFramebufferDimension(parameters->height);
            shadowParameters.preTransform = parameters->preTransform;
            shadowParameters.renderPass = rd->m_renderPassShadows;
            return std::make_shared<renderDetails::ParametersVulkan>(shadowParameters);
        }

        void createShadowResources(renderDetails::ParametersVulkan const *parameters);
    };
}

#endif // AMAZING_LABYRINTH_SHADOWSCHAINING_RENDER_DETAILS_VULKAN_HPP
