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
#ifndef AMAZING_LABYRINTH_SHADOWS_RENDER_DETAILS_VULKAN_HPP
#define AMAZING_LABYRINTH_SHADOWS_RENDER_DETAILS_VULKAN_HPP

#include <memory>
#include <glm/glm.h>
#include <glm/gtc/matrix_transform.hpp>

#include "../../graphicsVulkan.hpp"
#include "../../levelDrawer/drawObjectTable/drawObjectTable.hpp"
#include "../renderDetails.hpp"
#include "../../renderLoader/renderLoaderVulkan.hpp"
#include "../renderDetailsVulkan.hpp"

#include "config.hpp"

namespace shadows {
    class RenderDetailsVulkan;

    class CommonObjectDataVulkan : public renderDetails::CommonObjectDataPerspective {
        friend RenderDetailsVulkan;
    public:
        std::pair<glm::mat4, glm::mat4> getProjViewForLevel() override {
            /* perspective matrix: takes the perspective projection, the aspect ratio, near and far
             * view planes.
             */
            return std::make_pair<glm::mat4, glm::mat4>(
                    getPerspectiveMatrix(m_viewAngle, m_aspectRatio, m_nearPlane, m_farPlane,
                                         false, true),
                    view());
        }

        glm::mat4 getViewLightSource() override {
            return view();
        }

        glm::vec3 lightSource() { return viewPoint(); }

        std::shared_ptr<vulkan::Buffer> const &cameraBuffer() { return m_camera; }

        uint32_t cameraBufferSize() { return sizeof(CommonUBO); }

        ~CommonObjectDataVulkan() override = default;

    protected:
        void update() override {
            CommonUBO commonUbo;
            commonUbo.proj = m_preTransform *
                             getPerspectiveMatrix(m_viewAngle, m_aspectRatio, m_nearPlane, m_farPlane, true, true);

            // eye at the m_viewPoint, looking at the m_lookAt position, pointing up is m_up.
            commonUbo.view = view();

            m_camera->copyRawTo(&commonUbo, sizeof(commonUbo));
        }

    private:
        struct CommonUBO {
            glm::mat4 proj;
            glm::mat4 view;
        };

        std::shared_ptr<vulkan::Buffer> m_camera;
        glm::mat4 m_preTransform;

        CommonObjectDataVulkan(
                std::shared_ptr<vulkan::Buffer> buffer,
                glm::mat4 preTransform,
                float aspectRatio,
                Config const &config)
                : renderDetails::CommonObjectDataPerspective(
                config.viewAngle, aspectRatio, config.nearPlane, config.farPlane,
                config.lightingSource, config.lookAt, config.up),
                m_camera{std::move(buffer)},
                m_preTransform{preTransform}
        {}
    };

    /* for passing data other than the vertex data to the vertex shader */
    class DrawObjectDataVulkan : public renderDetails::DrawObjectDataVulkan {
        friend RenderDetailsVulkan;
    public:
        void update(glm::mat4 const &modelMatrix) override {
            PerObjectUBO ubo{};
            ubo.modelMatrix = modelMatrix;
            m_uniformBuffer->copyRawTo(&ubo, sizeof(ubo));
        }

        std::shared_ptr<vulkan::Buffer> const &bufferModelMatrix() override { return m_uniformBuffer; }
        std::shared_ptr<vulkan::DescriptorSet> const &descriptorSet(uint32_t id) override { return m_descriptorSet; }

        ~DrawObjectDataVulkan() override = default;

    private:
        struct PerObjectUBO {
            glm::mat4 modelMatrix;
        };

        std::shared_ptr<vulkan::DescriptorSet> m_descriptorSet;
        std::shared_ptr<vulkan::Buffer> m_uniformBuffer;

        DrawObjectDataVulkan(std::shared_ptr<vulkan::Device> const &inDevice,
                             std::shared_ptr<CommonObjectDataVulkan> const &inCommonObjectData,
                             std::shared_ptr<vulkan::DescriptorSet> inDescriptorSet,
                             std::shared_ptr<vulkan::Buffer> inUniformBuffer)
                : m_descriptorSet{std::move(inDescriptorSet)},
                  m_uniformBuffer{std::move(inUniformBuffer)}
        {
            updateDescriptorSet(inDevice, inCommonObjectData);
        }

        void updateDescriptorSet(std::shared_ptr<vulkan::Device> const &inDevice,
                std::shared_ptr<CommonObjectDataVulkan> const &inCommonObjectData);
    };

    class DescriptorSetLayout : public vulkan::DescriptorSetLayout {
    public:
        DescriptorSetLayout(std::shared_ptr<vulkan::Device> inDevice)
                : m_device(inDevice) {
            m_poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            m_poolSizes[0].descriptorCount = m_numberOfDescriptorSetsInPool;
            m_poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            m_poolSizes[1].descriptorCount = m_numberOfDescriptorSetsInPool;
            m_poolInfo = {};
            m_poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            m_poolInfo.poolSizeCount = static_cast<uint32_t>(m_poolSizes.size());
            m_poolInfo.pPoolSizes = m_poolSizes.data();
            m_poolInfo.maxSets = m_numberOfDescriptorSetsInPool;
            m_poolInfo.pNext = nullptr;

            createDescriptorSetLayout();
        }

        std::shared_ptr<VkDescriptorSetLayout_T> const &descriptorSetLayout() override {
            return m_descriptorSetLayout;
        }

        uint32_t numberOfDescriptors() override { return m_numberOfDescriptorSetsInPool; }

        VkDescriptorPoolCreateInfo const &poolCreateInfo() override {
            return m_poolInfo;
        }

        ~DescriptorSetLayout() override = default;

    private:
        static uint32_t constexpr m_numberOfDescriptorSetsInPool = 1024;

        std::shared_ptr<vulkan::Device> m_device;
        std::shared_ptr<VkDescriptorSetLayout_T> m_descriptorSetLayout;
        VkDescriptorPoolCreateInfo m_poolInfo;
        std::array<VkDescriptorPoolSize, 2> m_poolSizes;

        void createDescriptorSetLayout();
    };

    class RenderDetailsVulkan : public renderDetails::RenderDetails {
    public:
        static char const *name() { return shadowsRenderDetailsName; }

        static renderDetails::ReferenceVulkan loadNew(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
                std::shared_ptr<vulkan::Device> const &inDevice,
                renderDetails::ParametersVulkan const &parameters,
                Config const &config)
        {
            auto rd = std::make_shared<RenderDetailsVulkan>(
                    gameRequester, inDevice, nullptr, parameters);

            auto cod = rd->createCommonObjectData(parameters.preTransform, config);

            return createReference(std::move(rd), std::move(cod));
        }

        static renderDetails::ReferenceVulkan loadExisting(
                std::shared_ptr<GameRequester> const &,
                std::shared_ptr<RenderLoaderVulkan> const &,
                std::shared_ptr<renderDetails::RenderDetailsVulkan> rdBase,
                renderDetails::ParametersVulkan const &parameters,
                Config const &config)
        {
            auto rd = dynamic_cast<RenderDetailsVulkan*>(rdBase.get());
            if (rd == nullptr) {
                throw std::runtime_error("Invalid render details type.");
            }

            auto cod = rd->createCommonObjectData(parameters.preTransform, config);

            return createReference(std::move(rdBase), std::move(cod));
        }

        void addDrawCmdsToCommandBuffer(
                VkCommandBuffer const &commandBuffer,
                size_t descriptorSetID,
                std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
                std::shared_ptr<levelDrawer::DrawObjectTableVulkan> const &drawObjTable,
                std::vector<size_t> const &drawObjectsIndices) override;

        void reload(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
                ParametersVulkan const &parameters) override;

        std::shared_ptr<vulkan::Device> const &device() override { return m_device; }
        std::shared_ptr<vulkan::DescriptorPools> const &descriptorPools() override {
            return m_descriptorPools;
        }

        ~RenderDetailsVulkan() override = default;
    private:
        static char constexpr const *SHADER_SIMPLE_FRAG_FILE = "shaders/simple.frag.spv";
        static char constexpr const *SHADOW_VERT_FILE = "shaders/depthShader.vert.spv";

        std::shared_ptr<vulkan::Device> m_device;

        std::shared_ptr<vulkan::DescriptorPools> m_descriptorPools;
        std::shared_ptr<DescriptorSetLayout> m_descriptorSetLayout;
        std::shared_ptr<vulkan::Pipeline> m_pipeline;

        RenderDetailsVulkan(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<vulkan::Device> const &inDevice,
            std::shared_ptr<vulkan::Pipeline> const &basePipeline,
            renderDetails::ParametersVulkan const &parameters)
            : RenderDetails{parameters.width, parameters.height},
            m_device{inDevice},
            m_descriptorSetLayout{std::make_shared<DescriptorSetLayout>(m_device)},
            m_descriptorPools{std::make_shared<vulkan::DescriptorPools>(m_device, m_descriptorSetLayout)},
            m_pipeline{std::make_shared<vulkan::Pipeline>(
                        gameRequester, m_device,
                        VkExtent2D{m_surfaceWidth, m_surfaceHeight},
                        parameters.renderPass, m_descriptorPools, getBindingDescription(),
                        getAttributeDescriptions(),
                        SHADOW_VERT_FILE, SHADER_SIMPLE_FRAG_FILE, basePipeline,
                        VK_CULL_MODE_FRONT_BIT)}
        {}

        static renderDetails::ReferenceVulkan createReference(
                std::shared_ptr<renderDetails::RenderDetailsVulkan> rd,
                std::shared_ptr<CommonObjectDataVulkan> cod);

        std::shared_ptr<CommonObjectDataVulkan> createCommonObjectData(
                glm::mat4 const &preTransform,
                Config const &config)
        {
            auto buffer = createUniformBuffer(m_device, sizeof (CommonObjectDataVulkan::CommonUBO));
            return std::make_shared<CommonObjectDataVulkan>(buffer, preTransform,
                                                            m_surfaceWidth/ static_cast<float>(m_surfaceHeight), config);
        }
    };
}
#endif // AMAZING_LABYRINTH_SHADOWS_RENDER_DETAILS_VULKAN_HPP
