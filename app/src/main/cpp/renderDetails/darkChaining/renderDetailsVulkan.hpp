/**
 * Copyright 2022 Cerulean Quasar. All Rights Reserved.
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
#ifndef AMAZING_LABYRINTH_DARKCHAINING_RENDER_DETAILS_VULKAN_HPP
#define AMAZING_LABYRINTH_DARKCHAINING_RENDER_DETAILS_VULKAN_HPP

#include <memory>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../darkObject//renderDetailsVulkan.hpp"
#include "../renderDetailsVulkan.hpp"
#include "../../levelDrawer/drawObjectTable/drawObjectTableVulkan.hpp"
#include "../../levelDrawer/common.hpp"

namespace darkChaining {
    size_t constexpr numberShadowMaps = renderDetails::numberOfShadowMapsDarkMaze;

    class RenderDetailsVulkan;

    class CommonObjectDataVulkan : public renderDetails::CommonObjectData {
        friend RenderDetailsVulkan;
    public:
        std::pair<glm::mat4, glm::mat4> getProjViewForLevel() {
            return m_darkObject->getProjViewForLevel();
        }

        std::shared_ptr<darkObject::CommonObjectDataVulkan> const &darkObject() {
            return m_darkObject;
        }

        std::shared_ptr<shadows::CommonObjectDataVulkan> const &shadowsCOD(size_t i) {
            return m_shadowsCODs[i];
        }

        void update(renderDetails::Parameters const &parametersBase) override {
            auto parameters = dynamic_cast<renderDetails::ParametersPerspective const &>(parametersBase);

            m_darkObject->update(parameters);

            // shadows CODs
            renderDetails::ParametersPerspective parametersShadows{};
            renderDetails::ReferenceVulkan refShadows;

            for (size_t i = 0; i < numberShadowMaps; i++) {
                renderDetails::darkInitializeShadowMapParameters(parametersShadows, parameters, i, i==0);

                m_shadowsCODs[i]->update(parametersShadows);
            }

            m_shadowMapsNeedRender = true;
        }

        CommonObjectDataVulkan(std::shared_ptr<darkObject::CommonObjectDataVulkan> darkObjectCOD,
                               std::array<std::shared_ptr<shadows::CommonObjectDataVulkan>, numberShadowMaps> shadowsCODs)
        // The near plane and far plane are unused for Shadows Chaining
                : renderDetails::CommonObjectData(renderDetails::Parameters{}),
                  m_darkObject(std::move(darkObjectCOD)),
                  m_shadowsCODs(std::move(shadowsCODs)),
                  m_shadowMapsNeedRender(false),
                  m_holeShadowMapsNeedRender(true)
        {}

        ~CommonObjectDataVulkan() override = default;
    private:
        /* the COD for the Dark Object Render Details */
        std::shared_ptr<darkObject::CommonObjectDataVulkan> m_darkObject;

        /* CODs for the shadow maps for the ball, first one is for the up direction, then circle around
         * clockwise assigning numbers.  Then the hole follows in the same manor.
         */
        std::array<std::shared_ptr<shadows::CommonObjectDataVulkan>, numberShadowMaps> m_shadowsCODs;

        /* set to true each time an update occurs. Then in addPreRenderPassCmdsToCommandBuffer,
         * set to false before rendering shadow maps.
         */
        bool m_shadowMapsNeedRender;

        /* the hole shadow maps only need to be rendered once.  Start out set to true, then
         * once the hole shadow maps are rendered, set it to false.
         */
        bool m_holeShadowMapsNeedRender;
    };

    class DrawObjectDataVulkan : public renderDetails::DrawObjectDataVulkan {
    public:
        bool hasTexture() override { return m_mainDrawObjectData->hasTexture(); }

        std::shared_ptr<vulkan::Buffer> const &bufferModelMatrix() override {
            return m_mainDrawObjectData->bufferModelMatrix();
        }

        glm::mat4 modelMatrix(uint32_t id) override {
            if (id == renderDetails::MODEL_MATRIX_ID_MAIN) {
                return m_mainDrawObjectData->modelMatrix(id);
            } else {
                return m_shadowsDrawObjectsData[id - renderDetails::MODEL_MATRIX_ID_SHADOWS]->modelMatrix(id);
            }
        }

        bool updateTextureData(
                std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
                std::shared_ptr<levelDrawer::TextureDataVulkan> const &textureData) override
        {
            auto cod = dynamic_cast<CommonObjectDataVulkan*>(commonObjectData.get());
            return m_mainDrawObjectData->updateTextureData(cod->darkObject(), textureData);
        }

        void updateModelMatrixNoBufferUpdate(glm::mat4 const &modelMatrix) override {
            m_mainDrawObjectData->updateModelMatrixNoBufferUpdate(modelMatrix);
            for (auto &shadowsDOD : m_shadowsDrawObjectsData) {
                shadowsDOD->updateModelMatrixNoBufferUpdate(modelMatrix);
            }
        }

        void update(glm::mat4 const &modelMatrix) override {
            // the main draw object data and the shadows draw object data share a buffer.
            // So it is only necessary to update one.
            m_mainDrawObjectData->update(modelMatrix);
            for (auto &shadowsDOD : m_shadowsDrawObjectsData) {
                shadowsDOD->updateModelMatrixNoBufferUpdate(modelMatrix);
            }
        }

        std::shared_ptr<vulkan::DescriptorSet> const &descriptorSet(uint32_t id) override {
            switch (id) {
                case 0:
                    return m_mainDrawObjectData->descriptorSet(id);
                default:
                    return m_shadowsDrawObjectsData[id-1]->descriptorSet(id > 0 ? 1 : 0);
            }
        }

        DrawObjectDataVulkan(
                std::shared_ptr<renderDetails::DrawObjectDataVulkan> inMainDrawObjectData,
                std::array<std::shared_ptr<renderDetails::DrawObjectDataVulkan>, numberShadowMaps> inShadowsDrawObjectsData)
                : renderDetails::DrawObjectDataVulkan(),
                m_mainDrawObjectData{std::move(inMainDrawObjectData)},
                m_shadowsDrawObjectsData{std::move(inShadowsDrawObjectsData)}
        {}

        ~DrawObjectDataVulkan() override = default;
    private:
        std::shared_ptr<renderDetails::DrawObjectDataVulkan> m_mainDrawObjectData;
        std::array<std::shared_ptr<renderDetails::DrawObjectDataVulkan>, numberShadowMaps> m_shadowsDrawObjectsData;
    };

    class RenderDetailsVulkan : public renderDetails::RenderDetailsVulkan {
    public:
        std::string nameString() override { return m_renderDetailsName; }

        static renderDetails::ReferenceVulkan loadNew(
            char const *name,
            std::vector<char const *> const &shaders,
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
            std::shared_ptr<vulkan::Device> const &inDevice,
            std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &parameters);

        static renderDetails::ReferenceVulkan loadExisting(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
            std::shared_ptr<renderDetails::RenderDetailsVulkan> rdBase,
            std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &parameters);

        bool structuralChangeNeeded(std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails) override
        {
            // only check object with shadows because the shadows render details is going to
            // give the same answer as object with shadows with regards to the size.  The render
            // pass attachments are always the same for the shadows render pass that would be
            // stored in this render details.
            return m_darkObjectRenderDetails->structuralChangeNeeded(surfaceDetails);
        }

        void addPreRenderPassCmdsToCommandBuffer(
                VkCommandBuffer const &commandBuffer,
                size_t /* descriptor set ID, not used */,
                levelDrawer::CommonObjectDataList const &commonObjectDataList,
                levelDrawer::DrawObjectTableVulkanList const &drawObjTableList,
                std::set<levelDrawer::ZValueReference> const &starterZValues,
                std::set<levelDrawer::ZValueReference> const &levelZValues,
                std::set<levelDrawer::ZValueReference> const &finisherZValues) override;

        void addDrawCmdsToCommandBuffer(
                VkCommandBuffer const &commandBuffer,
                size_t descriptorSetID,
                std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
                std::shared_ptr<levelDrawer::DrawObjectTableVulkan> const &drawObjTable,
                std::set<levelDrawer::ZValueReference>::iterator beginZValRefs,
                std::set<levelDrawer::ZValueReference>::iterator endZValRefs,
                std::string const &renderDetailsName) override;

        std::shared_ptr<vulkan::Device> const &device() override { return m_darkObjectRenderDetails->device(); }

        RenderDetailsVulkan(
                char const *name,
                std::shared_ptr<vulkan::Device> const &inDevice,
                std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails)
            : renderDetails::RenderDetailsVulkan{surfaceDetails->surfaceWidth, surfaceDetails->surfaceHeight},
                m_renderDetailsName{name},
                m_device{inDevice}
        {
            createShadowResources(surfaceDetails);
        }

        ~RenderDetailsVulkan() override = default;

    private:
        // use less precision for the shadow buffer
        static float constexpr shadowsSizeMultiplier = 0.25f;

        char const *m_renderDetailsName;
        std::shared_ptr<vulkan::Device> m_device;
        std::shared_ptr<renderDetails::RenderDetailsVulkan> m_darkObjectRenderDetails;

        std::shared_ptr<vulkan::ImageView> m_depthImageViewShadows;
        std::array<std::shared_ptr<vulkan::ImageView>, numberShadowMaps> m_shadowsColorAttachments;
        std::array<std::shared_ptr<vulkan::ImageSampler>, numberShadowMaps> m_samplersShadows;
        std::array<std::shared_ptr<vulkan::RenderPass>, numberShadowMaps> m_renderPassesShadows;
        std::shared_ptr<renderDetails::RenderDetailsVulkan> m_shadowsRenderDetails;
        std::array<std::shared_ptr<vulkan::Framebuffer>, numberShadowMaps> m_framebuffersShadows;

        static std::pair<uint32_t, uint32_t> getShadowsFramebufferDimensions(std::pair<uint32_t, uint32_t> const &dimensions) {
            uint32_t width = static_cast<uint32_t>(std::floor(dimensions.first * shadowsSizeMultiplier));

            // use width to make the height of the shadow map image because we do not need something really tall
            // and for most devices the height is much bigger than the width.  Also, multiply by the shadow size multiplier
            // twice so that it is even smaller...
            uint32_t height = static_cast<uint32_t>(std::floor(dimensions.first * shadowsSizeMultiplier * shadowsSizeMultiplier));
            return std::make_pair(width, height);
        }

        static renderDetails::ReferenceVulkan createReference(
                std::shared_ptr<RenderDetailsVulkan> rd,
                renderDetails::ReferenceVulkan const &refShadows,
                renderDetails::ReferenceVulkan const &refObjectWithShadows,
                std::array<std::shared_ptr<shadows::CommonObjectDataVulkan>, numberShadowMaps> shadowCODs);

        std::shared_ptr<vulkan::SurfaceDetails> createShadowSurfaceDetails(
                std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails)
        {
            vulkan::SurfaceDetails shadowSurface{};
            auto wh = getShadowsFramebufferDimensions(std::make_pair(surfaceDetails->surfaceWidth, surfaceDetails->surfaceHeight));
            shadowSurface.surfaceWidth = wh.first;
            shadowSurface.surfaceHeight = wh.second;
            shadowSurface.preTransform = surfaceDetails->preTransform;

            /* vulkan requires that the pipeline be used with a **compatible** render pass to the
             * render pass it was created with.  All of the render passes for the shadow maps are
             * essentially the same except the have a different color attachment (of the same size).
             * So it is ok to use just the first shadow map render pass to create the surface details
             * (which are eventually used to create the pipeline.
             */
            shadowSurface.renderPass = m_renderPassesShadows[0];
            return std::make_shared<vulkan::SurfaceDetails>(std::move(shadowSurface));
        }

        static renderDetails::ReferenceVulkan loadHelper(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
                std::shared_ptr<RenderDetailsVulkan> rd,
                std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails,
                std::shared_ptr<renderDetails::Parameters> const &parameters);

        void createShadowResources(std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails);
    };
}

#endif // AMAZING_LABYRINTH_DARKCHAINING_RENDER_DETAILS_VULKAN_HPP
