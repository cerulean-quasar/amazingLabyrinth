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
#ifndef AMAZING_LABYRINTH_RENDER_DETAILS_DATA_VULKAN_HPP
#define AMAZING_LABYRINTH_RENDER_DETAILS_DATA_VULKAN_HPP

#include <memory>

#include "../../levelDrawer/commonVulkan.hpp"
#include "../../graphicsVulkan.hpp"
#include "../../levelDrawer/drawObjectTable/drawObject.hpp"
#include "../../levelDrawer/drawObjectTable/drawObjectDataVulkan.hpp"
#include "../../levelDrawer/renderDetailsTable/renderDetailsData.hpp"

namespace shadows {
    class CommonObjectDataVulkan : public CommonObjectData {
    public:
        CommonObjectDataVulkan(
            std::shared_ptr<vulkan::Buffer> buffer,
            float viewAngle,
            float aspectRatio,
            float nearPlane,
            float farPlane,
            glm::vec3 const &lightingSource,
            glm::vec3 const &lookAt,
            glm::vec3 const &up)
            : m_camera{std::move(buffer)},
            m_viewAngle{viewAngle},
            m_aspectRatio{aspectRatio},
            m_nearPlane{nearPlane},
            m_farPlane{farPlane},
            m_viewPoint{lightingSource},
            m_lookAt{lookAt},
            m_up{up}
        {}

        void setView(
            glm::vec3 const &lightingSource,
            glm::vec3 const &lookAt,
            glm::vec3 const &up)
        {
            m_viewPoint = lightingSource;
            m_lookAt = lookAt;
            m_up = up;
            update();
        }

        void setProjection(
                float viewAngle,
                float aspectRatio,
                float nearPlane,
                float farPlane)
        {
            m_viewAngle = viewAngle;
            m_aspectRatio = aspectRatio;
            m_nearPlane = nearPlane;
            m_farPlane = farPlane;
            update();
        }

        glm::mat4 getPerspectiveMatrixForLevel() override {
            /* perspective matrix: takes the perspective projection, the aspect ratio, near and far
             * view planes.
             */
            return getPerspectiveMatrix(m_viewAngle, m_aspectRatio, m_nearPlane, m_farPlane,
                                        false, true);
        }

    private:
        struct CommonUBO {
            glm::mat4 proj;
            glm::mat4 view;
        };

        std::shared_ptr<vulkan::Buffer> m_camera;
        glm::mat4 m_preTransform;

        float m_viewAngle;
        float m_aspectRatio;
        float m_nearPlane;
        float m_farPlane;

        glm::vec3 m_viewPoint;
        glm::vec3 m_lookAt;
        glm::vec3 m_up;

        void update() {
            CommonUBO commonUbo;
            commonUbo.proj = m_preTransform *
                getPerspectiveMatrix(m_viewAngle, m_aspectRatio, m_nearPlane, m_farPlane, true, true);

            // eye at the m_viewPoint, looking at the m_lookAt position, pointing up is m_up.
            commonUbo.view = glm::lookAt(m_viewPoint, m_lookAt, m_up);;

            m_camera->copyRawTo(&commonUbo, sizeof(commonUbo));
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

    class RenderDetailsDataVulkan : public RenderDetailsData {
    public:
        std::shared_ptr<DrawObjectData> createDrawObjectData(
                std::shared_ptr<TextureData> const&,
                std::shared_ptr<vulkan::Buffer> const &modelMatrixBuffer) override
        {
            return std::make_shared<DrawObjectDataVulkan>(m_device, m_descriptorPools,
                                                          m_commonUBO, modelMatrixBuffer);
        }

        std::shared_ptr<DrawObjectData> createDrawObjectData(
                std::shared_ptr<TextureData> const&,
                glm::mat4 const &modelMatrix) override
        {
            auto modelMatrixBuffer = createUniformBuffer(m_device, sizeof (PerObjectUBO));
            PerObjectUBO perObjectUbo{};
            perObjectUbo.model = modelMatrix;
            modelMatrixBuffer->copyRawTo(&perObjectUbo, sizeof(PerObjectUBO));
            return std::make_shared<DrawObjectDataVulkan>(m_device, m_descriptorPools,
                                                          m_commonUBO, modelMatrixBuffer);
        }

        RenderDetailsDataVulkan(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<vulkan::Device> const &inDevice,
            std::shared_ptr<vulkan::CommandPool> const &inCommandPool,
            std::shared_ptr<vulkan::Pipeline> const &basePipeline,
            std::vector<vulkan::RenderPass::ImageAttachmentInfo> const &colorAttachmentInfos,
            vulkan::RenderPass::ImageAttachmentInfo depthAttachmentInfos,
            glm::mat4 inPreTransform,
            uint32_t width,
            uint32_t height)
            : RenderDetailsData{width, height},
            m_device{inDevice},
            m_commandPool{inCommandPool},
            m_commonUBO{createUniformBuffer(inDevice, sizeof(CommonUBO))},
            m_preTransform{std::move(inPreTransform)},
            m_descriptorSetLayout{
                std::make_shared<DescriptorSetLayout>(m_device)},
            m_descriptorPools{
                std::make_shared<vulkan::DescriptorPools>(m_device, m_descriptorSetLayout)},
            m_renderPass{vulkan::RenderPass::createDepthTextureRenderPass(
                m_device, colorAttachmentInfos, depthAttachmentInfos)},
            m_pipeline{std::make_shared<vulkan::Pipeline>(
                gameRequester, m_device,
                VkExtent2D{width, height},
                m_renderPass, m_descriptorPools, getBindingDescription(),
                getAttributeDescriptions(),
                SHADOW_VERT_FILE, SHADER_SIMPLE_FRAG_FILE, mainPipeline,
                VK_CULL_MODE_FRONT_BIT)}
        {
        }

    private:
        static char constexpr const *SHADER_VERT_FILE = "shaders/shader.vert.spv";
        static char constexpr const *SHADER_FRAG_FILE = "shaders/shader.frag.spv";
        static char constexpr const *SHADER_SIMPLE_FRAG_FILE = "shaders/simple.frag.spv";
        static char constexpr const *SHADOW_VERT_FILE = "shaders/depthShader.vert.spv";

        std::shared_ptr<vulkan::Device> m_device;
        std::shared_ptr<vulkan::CommandPool> m_commandPool;
        std::shared_ptr<vulkan::Buffer> m_uniformBufferLighting;
        std::shared_ptr<vulkan::Buffer> m_commonUBO;
        glm::mat4 m_preTransform;

        std::shared_ptr<vulkan::RenderPass> m_renderPass;
        std::shared_ptr<vulkan::DescriptorPools> m_descriptorPools;
        std::shared_ptr<DescriptorSetLayout> m_descriptorSetLayout;
        std::shared_ptr<vulkan::Pipeline> m_pipeline;

        // use less precision for the shadow buffer
        static float constexpr shadowsSizeMultiplier = 0.5f;

        uint32_t getShadowsFramebufferHeigth(VkExtent2D const &extent) {
            return static_cast<uint32_t>(std::floor(extent.height * shadowsSizeMultiplier));
            //return m_swapChain->extent().height;
        }

        uint32_t getShadowsFramebufferWidth(VkExtent2D const &extent) {
            return static_cast<uint32_t>(std::floor(extent.width * shadowsSizeMultiplier));
            //return m_swapChain->extent().width+1;
            //return m_swapChain->extent().width;
        }
    };
}
#endif
