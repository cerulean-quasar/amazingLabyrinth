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
#ifndef AMAZING_LABYRINTH_TEXTUREWITHSHADOWS_RENDER_DETAILS_DATA_VULKAN_HPP
#define AMAZING_LABYRINTH_TEXTUREWITHSHADOWS_RENDER_DETAILS_DATA_VULKAN_HPP

#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../graphicsVulkan.hpp"
#include "../../levelDrawer/drawObjectTable/drawObjectTable.hpp"
#include "../renderDetails.hpp"
#include "../shadows/renderDetailsVulkan.hpp"
#include "config.hpp"
#include "../renderDetailsVulkan.hpp"

namespace textureWithShadows {
    class RenderDetailsDataVulkan;

    class CommonObjectDataVulkan : public renderDetails::CommonObjectDataPerspective {
        friend RenderDetailsDataVulkan;
    public:
        void setShadowsImageSampler(std::shared_ptr<vulkan::ImageSampler> shadowsImageSampler) {
            m_shadowsSampler = std::move(shadowsImageSampler);
        }

        std::shared_ptr<vulkan::Buffer> const &cameraBuffer() { return m_cameraBuffer; }
        uint32_t cameraBufferSize() { return sizeof (CommonVertexUBO); }
        std::shared_ptr<vulkan::Buffer> const &lightingBuffer() { return m_lightingSourceBuffer; }
        uint32_t lightingBufferSize() { return sizeof (CommonFragmentUBO); }
        std::shared_ptr<vulkan::ImageSampler> const &shadowsSampler() { return m_shadowsSampler; }

        glm::mat4 getPerspectiveMatrixForLevel() override {
            /* perspective matrix: takes the perspective projection, the aspect ratio, near and far
             * view planes.
             */
            return getPerspectiveMatrix(m_viewAngle, m_aspectRatio, m_nearPlane, m_farPlane,
                                        false, true);
        }

        ~CommonObjectDataVulkan() override = default;
    protected:
        void update() override {
            doUpdate();
        }

    private:
        struct CommonVertexUBO {
            glm::mat4 proj;
            glm::mat4 view;
            glm::mat4 lightViewMatrix;
        };

        struct CommonFragmentUBO {
            glm::vec3 lightingSource;
        };

        std::shared_ptr<vulkan::Buffer> m_cameraBuffer;
        std::shared_ptr<vulkan::Buffer> m_lightingSourceBuffer;
        std::shared_ptr<vulkan::ImageSampler> m_shadowsSampler;
        glm::mat4 m_preTransform;

        CommonObjectDataVulkan(
            std::shared_ptr<shadows::CommonObjectDataVulkan> shadowsCOD,
            std::shared_ptr<vulkan::Buffer> cameraBuffer,
            std::shared_ptr<vulkan::Buffer> lightingSourceBuffer,
            glm::mat4 const &preTransform,
            float aspectRatio,
            Config const &config)
            : renderDetails::CommonObjectData(
                config.viewAngle, aspectRatio, config.nearPlane, config.farPlane,
                config.viewPoint, config.lookAt, config.up),
            m_shadowsCOD{std::move(shadowsCOD)},
            m_cameraBuffer{std::move(cameraBuffer)},
            m_lightingSourceBuffer{std::move(lightingSourceBuffer)},
            m_preTransform{preTransform}
        {
            doUpdate();
        }

        void doUpdate() {
            CommonVertexUBO commonUbo;
            commonUbo.proj = m_preTransform *
                             getPerspectiveMatrix(m_viewAngle, m_aspectRatio, m_nearPlane, m_farPlane, true, true);

            // eye at the m_viewPoint, looking at the m_lookAt position, pointing up is m_up.
            commonUbo.view = glm::lookAt(m_viewPoint, m_lookAt, m_up);
            commonUbo.lightViewMatrix = m_shadowsCOD->view();

            m_cameraBuffer->copyRawTo(&commonUbo, sizeof(commonUbo));

            CommonFragmentUBO commonFragmentUbo;
            commonFragmentUbo.lightingSource = m_shadowsCOD->lightingSource();
            m_lightingSourceBuffer->copyRawTo(&commonFragmentUbo, sizeof(commonFragmentUbo));
        }
    };

    class DescriptorSetLayout : public vulkan::DescriptorSetLayout {
    public:
        DescriptorSetLayout(std::shared_ptr<vulkan::Device> inDevice)
                : m_device(inDevice) {
            m_poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            m_poolSizes[0].descriptorCount = m_numberOfDescriptorSetsInPool;
            m_poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            m_poolSizes[1].descriptorCount = m_numberOfDescriptorSetsInPool;
            m_poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            m_poolSizes[2].descriptorCount = m_numberOfDescriptorSetsInPool;
            m_poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            m_poolSizes[3].descriptorCount = m_numberOfDescriptorSetsInPool;
            m_poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            m_poolSizes[4].descriptorCount = m_numberOfDescriptorSetsInPool;
            m_poolInfo = {};
            m_poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            m_poolInfo.poolSizeCount = static_cast<uint32_t>(m_poolSizes.size());
            m_poolInfo.pPoolSizes = m_poolSizes.data();
            m_poolInfo.maxSets = m_numberOfDescriptorSetsInPool;
            m_poolInfo.pNext = nullptr;

            createDescriptorSetLayout();
        }

        virtual std::shared_ptr<VkDescriptorSetLayout_T> const &descriptorSetLayout() {
            return m_descriptorSetLayout;
        }

        virtual uint32_t numberOfDescriptors() { return m_numberOfDescriptorSetsInPool; }

        virtual VkDescriptorPoolCreateInfo const &poolCreateInfo() {
            return m_poolInfo;
        }

        ~DescriptorSetLayout() override = default;

    private:
        static uint32_t constexpr m_numberOfDescriptorSetsInPool = 1024;

        std::shared_ptr<vulkan::Device> m_device;
        std::shared_ptr<VkDescriptorSetLayout_T> m_descriptorSetLayout;
        VkDescriptorPoolCreateInfo m_poolInfo;
        std::array<VkDescriptorPoolSize, 5> m_poolSizes;

        void createDescriptorSetLayout();
    };

    /* for passing data other than the vertex data to the vertex shader */
    class DrawObjectDataVulkan : public renderDetails::DrawObjectDataVulkan {
        std::shared_ptr<vulkan::Buffer> const &bufferModelMatrix() override {
            return m_uniformBuffer;
        }

        std::shared_ptr<vulkan::DescriptorSet> const &descriptorSet(uint32_t id) override {
            return m_descriptorSet;
        }

        DrawObjectDataVulkan(std::shared_ptr<vulkan::Device> const &inDevice,
                std::shared_ptr<CommonObjectDataVulkan> const &cod,
                std::shared_ptr<vulkan::DescriptorSet> inDescriptorSet,
                std::shared_ptr<vulkan::Buffer> inBuffer)
                : m_descriptorSet{std::move(inDescriptorSet)},
                  m_uniformBuffer{std::move(inBuffer)}
        {
            updateDescriptorSet(inDevice, cod);
        }

        ~DrawObjectDataVulkan() override = default;
    private:
        std::shared_ptr<vulkan::DescriptorSet> m_descriptorSet;
        std::shared_ptr<vulkan::Buffer> m_uniformBuffer;

        void updateDescriptorSet(
                std::shared_ptr<vulkan::Device> const &inDevice,
                std::shared_ptr<CommonObjectDataVulkan> const &cod);
    };

    class RenderDetailsVulkan : public renderDetails::RenderDetailsVulkan {
    public:
        static char const *name() { return m_name; }

        static renderDetails::ReferenceVulkan loadNew(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
                std::shared_ptr<vulkan::Device> const &inDevice,
                renderDetails::RenderDetailsParametersVulkan const &parameters,
                Config const &config)
        {
            auto rd = std::make_shared<RenderDetailsVulkan>(
                    gameRequester, inDevice, nullptr, parameters);

            auto cod = rd->createCommonObjectData(parameters.preTransform, config);

            return createReference(std::move(rd), std::move(cod));
        }

        static renderDetails::ReferenceVulkan loadExisting(
                std::shared_ptr<renderDetails::RenderDetailsVulkan> const &rdBase,
                renderDetails::RenderDetailsParametersVulkan const &parameters,
                Config const &config)
        {
            auto rd = dynamic_cast<RenderDetailsVulkan*>(rdBase.get());
            if (rd == nullptr) {
                throw std::runtime_error("Invalid render details type.")
            }

            auto cod = rd->createCommonObjectData(parameters.preTransform, config);

            return createReference(std::move(rdBase), std::move(cod));
        }

        std::shared_ptr<DrawObjectData> createDrawObjectData(
                std::shared_ptr<TextureData> &textureData,
                glm::mat4 const &modelMatrix) {
            PerObjectUBO perObjectUBO{};
            perObjectUBO.model = modelMatrix;
            return std::make_shared<DrawObjectDataVulkan>(m_device, m_descriptorPools,
                                                          m_descriptorPoolsShadows,
                                                          textureData->sampler(), m_samplerShadows,
                                                          m_uniformBufferLighting, m_commonUBO,
                                                          perObjectUBO);
        }


    private:
        static char constexpr const *m_name = "textureWithShadows";
        static char constexpr const *SHADER_VERT_FILE = "shaders/shader.vert.spv";
        static char constexpr const *SHADER_FRAG_FILE = "shaders/shader.frag.spv";

        std::shared_ptr<vulkan::Device> m_device;

        /* main draw resources */
        std::shared_ptr<vulkan::RenderPass> m_renderPass;
        std::shared_ptr<DescriptorSetLayout> m_descriptorSetLayout;
        std::shared_ptr<vulkan::DescriptorPools> m_descriptorPools;
        std::shared_ptr<vulkan::Pipeline> m_graphicsPipeline;

        RenderDetailsVulkan(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<vulkan::Device> const &inDevice,
                std::shared_ptr<vulkan::Pipeline> const &basePipeline,
                renderDetails::RenderDetailsParametersVulkan const &parameters)
                : renderDetails::RenderDetailsVulkan{parameters.width, parameters.height},
                  m_device{inDevice},
                  m_renderPass{vulkan::RenderPass::createDepthTextureRenderPass(
                          m_device, parameters.colorImageInfo, parameters.depthImageInfo)},
                  m_descriptorSetLayout{std::make_shared<DescriptorSetLayout>(m_device)},
                  m_descriptorPools{std::make_shared<vulkan::DescriptorPools>(m_device, m_descriptorSetLayout)},
                  m_graphicsPipeline{std::make_shared<vulkan::Pipeline>(
                          gameRequester, m_device, VkExtent2D{m_surfaceWidth, m_surfaceHeight},
                          m_renderPass, m_descriptorPools,
                          getBindingDescription(),
                          getAttributeDescriptions(),
                          SHADER_VERT_FILE, SHADER_FRAG_FILE, nullptr)}
        {}

        std::shared_ptr<renderDetails::CommonObjectData> createCommonObjectData(
                glm::mat4 const &preTransform,
                Config const &config)
        {
            shadows::Config configShadows{};
            configShadows.viewAngle = config.viewAngle;
            configShadows.nearPlane = config.nearPlane;
            configShadows.farPlane = config.farPlane;
            configShadows.lightingSource = config.lightingSource;
            configShadows.lookAt = config.lookAt;
            configShadows.up = config.up;
            auto shadowsCOD = m_shadowsRenderDetails->createCommonObjectData(preTransform, configShadows);

            auto vertexUbo = createUniformBuffer(m_device, sizeof (CommonObjectDataVulkan::CommonVertexUBO));
            auto fragUbo = createUniformBuffer(m_device, sizeof (CommonObjectDataVulkan::CommonFragmentUBO));

            return std::make_shared<CommonObjectDataVulkan>(shadowsCOD, vertexUbo, fragUbo, preTransform,
                                                            m_surfaceWidth/static_cast<float>(m_surfaceHeight), config);
        }

        static renderDetails::ReferenceVulkan createReference(
                std::shared_ptr<RenderDetailsVulkan> rd,
                std::shared_ptr<CommonObjectDataVulkan> cod)
        {
            renderDetails::ReferenceVulkan ref;
            ref.createDrawObjectData = renderDetails::ReferenceVulkan::CreateDrawObjectData(
                    [rd, cod] (std::shared_ptr<renderDetails::DrawObjectDataVulkan> const &sharingDOD,
                               std::shared_ptr<TextureData> const &textureData,
                               glm::mat4 const &modelMatrix) ->
                            std::shared_ptr<renderDetails::DrawObjectDataVulkan>
                    {
                        std::shared_ptr<vulkan::Buffer> modelMatrixBuffer{};
                        if (sharingDOD) {
                            modelMatrixBuffer = sharingDOD->bufferModelMatrix();
                        } else {
                            modelMatrixBuffer = createUniformBuffer(
                                    rd->device(), sizeof (DrawObjectDataVulkan::PerObjectUBO));
                            DrawObjectDataVulkan::PerObjectUBO perObjectUbo{};
                            perObjectUbo.model = modelMatrix;
                            modelMatrixBuffer->copyRawTo(
                                    &perObjectUbo, sizeof(DrawObjectDataVulkan::PerObjectUBO));
                        }

                        return std::make_shared<DrawObjectDataVulkan>(
                                rd->device(),
                                cod,
                                rd->descriptorPools()->allocateDescriptor(),
                                std::move(modelMatrixBuffer));
                    });

            ref.renderDetails = std::move(rd);
            ref.commonObjectData = std::move(cod);

            return std::move(ref);
        }
    };
}

#endif // AMAZING_LABYRINTH_TEXTUREWITHSHADOWS_RENDER_DETAILS_DATA_VULKAN_HPP
