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

#include "../../graphicsVulkan.hpp"
#include "../drawObjectTable/drawObject.hpp"
#include "../drawObjectTable/drawObjectDataVulkan.hpp"
#include "renderDetailsData.hpp"

VkVertexInputBindingDescription getBindingDescription();
std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

class AmazingLabyrinthDescriptorSetLayout : public vulkan::DescriptorSetLayout {
public:
    AmazingLabyrinthDescriptorSetLayout(std::shared_ptr<vulkan::Device> inDevice)
            : m_device(inDevice)
    {
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
    virtual ~AmazingLabyrinthDescriptorSetLayout() {}

private:
    static uint32_t constexpr m_numberOfDescriptorSetsInPool = 1024;

    std::shared_ptr<vulkan::Device> m_device;
    std::shared_ptr<VkDescriptorSetLayout_T> m_descriptorSetLayout;
    VkDescriptorPoolCreateInfo m_poolInfo;
    std::array<VkDescriptorPoolSize, 5> m_poolSizes;

    void createDescriptorSetLayout();
};

class AmazingLabyrinthShadowsDescriptorSetLayout : public vulkan::DescriptorSetLayout {
public:
    AmazingLabyrinthShadowsDescriptorSetLayout(std::shared_ptr<vulkan::Device> inDevice)
            : m_device(inDevice)
    {
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
    virtual std::shared_ptr<VkDescriptorSetLayout_T> const &descriptorSetLayout() {
        return m_descriptorSetLayout;
    }
    virtual uint32_t numberOfDescriptors() { return m_numberOfDescriptorSetsInPool; }
    virtual VkDescriptorPoolCreateInfo const &poolCreateInfo() {
        return m_poolInfo;
    }
    virtual ~AmazingLabyrinthShadowsDescriptorSetLayout() {}

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
            std::shared_ptr<TextureData> &textureData,
            glm::mat4 const &modelMatrix) override
    {
        PerObjectUBO perObjectUBO{};
        perObjectUBO.model = modelMatrix;
        return std::make_shared<DrawObjectDataVulkan>(m_device, m_descriptorPools,
                m_descriptorPoolsShadows, textureData->sampler(), m_samplerShadows,
                m_uniformBufferLighting, m_commonUBO, perObjectUBO);
    }

    glm::mat4 getPerspectiveMatrixForLevel() {
        /* perspective matrix: takes the perspective projection, the aspect ratio, near and far
         * view planes.
         */
        return getPerspectiveMatrix(m_perspectiveViewAngle,
                                    m_surfaceWidth / static_cast<float>(m_surfaceHeight),
                                    m_perspectiveNearPlane, m_perspectiveFarPlane,
                                    false, true);
    }

    RenderDetailsDataVulkan(std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<vulkan::Device> const &inDevice,
            std::shared_ptr<vulkan::CommandPool> const &inCommandPool,
            std::shared_ptr<vulkan::SwapChain> const &inSwapChain,
            glm::mat4 inPreTransform)
        : RenderDetailsData{inSwapChain->extent().width, inSwapChain->extent().height},
          m_device{inDevice},
          m_commandPool{inCommandPool},
          m_uniformBufferLighting{createUniformBuffer(inDevice, sizeof (glm::vec3))},
          m_commonUBO{createUniformBuffer(inDevice, sizeof (CommonUBO))},
          m_preTransform{std::move(inPreTransform)},
          m_renderPass{vulkan::RenderPass::createRenderPass(m_device, inSwapChain)},
          m_descriptorSetLayout{new AmazingLabyrinthDescriptorSetLayout{m_device}},
          m_descriptorPools{new vulkan::DescriptorPools{m_device, m_descriptorSetLayout}},
          m_graphicsPipeline{new vulkan::Pipeline{
              gameRequester, m_device, inSwapChain->extent(),
              m_renderPass, m_descriptorPools,
              getBindingDescription(), getAttributeDescriptions(),
              SHADER_VERT_FILE, SHADER_FRAG_FILE, nullptr}},
          m_descriptorSetLayoutShadows{std::make_shared<AmazingLabyrinthShadowsDescriptorSetLayout>(m_device)},
          m_descriptorPoolsShadows{std::make_shared<vulkan::DescriptorPools>(m_device, m_descriptorSetLayoutShadows)},
          m_depthImageViewShadows{std::make_shared<vulkan::ImageView>(
                  vulkan::ImageFactory::createDepthImage(m_device,
                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, getShadowsFramebufferWidth(inSwapChain->extent()),
                          getShadowsFramebufferHeigth(inSwapChain->extent())),
                  VK_IMAGE_ASPECT_DEPTH_BIT)},
          m_shadowsColorAttachment{vulkan::ImageView::createImageViewAndImage(
                  m_device,
                  getShadowsFramebufferWidth(inSwapChain->extent()),
                  getShadowsFramebufferHeigth(inSwapChain->extent()),
                  VK_FORMAT_R32G32B32A32_SFLOAT,
                  VK_IMAGE_TILING_OPTIMAL,
                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                  VK_IMAGE_ASPECT_COLOR_BIT)},
          m_samplerShadows{std::make_shared<vulkan::ImageSampler>(
                  m_device, m_shadowsColorAttachment, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                  VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)},
          m_commandBufferShadows{std::make_shared<vulkan::CommandBuffer>(
                  m_device, m_commandPool, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)},
          m_renderPassShadows{vulkan::RenderPass::createDepthTextureRenderPass(
                  m_device,
                  std::vector<vulkan::RenderPass::ImageAttachmentInfo>{vulkan::RenderPass::ImageAttachmentInfo{
                        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
                        m_shadowsColorAttachment->image()->format(),
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}},
                  std::make_shared<vulkan::RenderPass::ImageAttachmentInfo>(
                        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
                        m_depthImageViewShadows->image()->format(),
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL))},
          m_framebufferShadows{std::make_shared<vulkan::Framebuffer>(
                   m_device, m_renderPassShadows,
                   std::vector<std::shared_ptr<vulkan::ImageView>>{m_shadowsColorAttachment, m_depthImageViewShadows},
                   getShadowsFramebufferWidth(inSwapChain->extent()),
                   getShadowsFramebufferHeigth(inSwapChain->extent()))},
          m_pipelineShadows{std::make_shared<vulkan::Pipeline>(
                  gameRequester, m_device,
                  VkExtent2D{getShadowsFramebufferWidth(inSwapChain->extent()),
                             getShadowsFramebufferHeigth(inSwapChain->extent())},
                  m_renderPassShadows, m_descriptorPoolsShadows, getBindingDescription(), getAttributeDescriptions(),
                  SHADOW_VERT_FILE, SHADER_SIMPLE_FRAG_FILE, m_graphicsPipeline, VK_CULL_MODE_FRONT_BIT)}
    {
        Extent2D extent = inSwapChain->extent();
        m_uniformBufferLighting->copyRawTo(&m_lightingSource, sizeof (m_lightingSource));
        CommonUBO commonUbo;
        commonUbo.proj = m_preTransform *
                getPerspectiveMatrix(m_perspectiveViewAngle,
                                     m_surfaceWidth / static_cast<float>(m_surfaceHeight),
                                     m_perspectiveNearPlane, m_perspectiveFarPlane,
                                     true, true);
        commonUbo.view = getViewMatrix();
        commonUbo.viewLightMatrix = glm::lookAt(m_lightingSource,
                                                glm::vec3(0.0f, 0.0f, levelTracker::Loader::m_maxZLevel),
                                                glm::vec3(0.0f, 1.0f, 0.0f));

        m_commonUBO->copyRawTo(&commonUbo, sizeof (commonUbo));

    }
private:
    static char constexpr const *SHADER_VERT_FILE ="shaders/shader.vert.spv";
    static char constexpr const *SHADER_FRAG_FILE ="shaders/shader.frag.spv";
    static char constexpr const *SHADER_SIMPLE_FRAG_FILE ="shaders/simple.frag.spv";
    static char constexpr const *SHADOW_VERT_FILE = "shaders/depthShader.vert.spv";

    std::shared_ptr<vulkan::Device> m_device;
    std::shared_ptr<vulkan::CommandPool> m_commandPool;
    std::shared_ptr<vulkan::Buffer> m_uniformBufferLighting;
    std::shared_ptr<vulkan::Buffer> m_commonUBO;
    glm::mat4 m_preTransform;

    /* main draw resources */
    std::shared_ptr<vulkan::RenderPass> m_renderPass;
    std::shared_ptr<AmazingLabyrinthDescriptorSetLayout> m_descriptorSetLayout;
    std::shared_ptr<vulkan::DescriptorPools> m_descriptorPools;
    std::shared_ptr<vulkan::Pipeline> m_graphicsPipeline;

    /* shadow resources */
    std::shared_ptr<AmazingLabyrinthShadowsDescriptorSetLayout> m_descriptorSetLayoutShadows;
    std::shared_ptr<vulkan::DescriptorPools> m_descriptorPoolsShadows;
    std::shared_ptr<vulkan::ImageView> m_depthImageViewShadows;
    std::shared_ptr<vulkan::ImageView> m_shadowsColorAttachment;
    std::shared_ptr<vulkan::ImageSampler> m_samplerShadows;
    std::shared_ptr<vulkan::CommandBuffer> m_commandBufferShadows;
    std::shared_ptr<vulkan::RenderPass> m_renderPassShadows;
    std::shared_ptr<vulkan::Framebuffer> m_framebufferShadows;
    std::shared_ptr<vulkan::Pipeline> m_pipelineShadows;

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

#endif
