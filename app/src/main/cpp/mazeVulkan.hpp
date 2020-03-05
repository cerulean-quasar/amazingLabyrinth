/**
 * Copyright 2019 Cerulean Quasar. All Rights Reserved.
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
#ifndef AMAZING_LABYRINTH_MAZE_VULKAN_HPP
#define AMAZING_LABYRINTH_MAZE_VULKAN_HPP
#include "graphicsVulkan.hpp"
#include "mazeGraphics.hpp"
#include "common.hpp"

char constexpr const *SHADER_VERT_FILE ="shaders/shader.vert.spv";
char constexpr const *SHADER_FRAG_FILE ="shaders/shader.frag.spv";
char constexpr const *SHADER_DEPTH_VERT_FILE ="shaders/depthShader.vert.spv";
char constexpr const *SHADER_DEPTH_FRAG_FILE ="shaders/depthShader.frag.spv";

std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
VkVertexInputBindingDescription getBindingDescription();

class AmazingLabyrinthDescriptorSetLayout : public vulkan::DescriptorSetLayout {
public:
    AmazingLabyrinthDescriptorSetLayout(std::shared_ptr<vulkan::Device> inDevice)
            : m_device(inDevice)
    {
        m_poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        m_poolSizes[0].descriptorCount = m_numberOfDescriptorSetsInPool;
        m_poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        m_poolSizes[1].descriptorCount = m_numberOfDescriptorSetsInPool;
        m_poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        m_poolSizes[2].descriptorCount = m_numberOfDescriptorSetsInPool;
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
    std::array<VkDescriptorPoolSize, 3> m_poolSizes;

    void createDescriptorSetLayout();
};

class TextureDataVulkan : public TextureData {
public:
    TextureDataVulkan(std::shared_ptr<vulkan::Device> const &inDevice,
                      std::shared_ptr<vulkan::CommandPool> const &inCommandPool,
                      std::shared_ptr<TextureDescription> const &inTextureDescription)
            : m_sampler{new vulkan::ImageSampler{inDevice, inCommandPool, inTextureDescription}}
    {}

    // TODO: testing, can remove
    TextureDataVulkan(std::shared_ptr<vulkan::ImageSampler> const &inSampler)
            : m_sampler{inSampler}
    {}

    inline std::shared_ptr<vulkan::ImageSampler> const &sampler() { return m_sampler; }
private:
    std::shared_ptr<vulkan::ImageSampler> m_sampler;
};

std::shared_ptr<vulkan::Buffer> createUniformBuffer(
        std::shared_ptr<vulkan::Device> const &device, size_t bufferSize);

class UniformWrapper {
public:
    /* for passing data other than the vertex data to the vertex shader */
    std::shared_ptr<vulkan::DescriptorSet> m_descriptorSet;
    std::shared_ptr<vulkan::ImageSampler> m_sampler;
    std::shared_ptr<vulkan::Buffer> m_uniformBufferLighting;
    std::shared_ptr<vulkan::Buffer> m_uniformBuffer;

    UniformWrapper(std::shared_ptr<vulkan::Device> const &inDevice,
                   std::shared_ptr<vulkan::DescriptorPools> const &descriptorPools,
                   std::shared_ptr<vulkan::ImageSampler> const &inSampler,
                   std::shared_ptr<vulkan::Buffer> const &inUniformBufferLighting,
                   UniformBufferObject const &ubo)
            : m_descriptorSet{},
              m_sampler{inSampler},
              m_uniformBufferLighting{inUniformBufferLighting},
              m_uniformBuffer{}
    {
        VkBuffer uniformBuffer;
        VkDeviceMemory uniformBufferMemory;

        m_uniformBuffer = createUniformBuffer(inDevice, sizeof (UniformBufferObject));
        m_uniformBuffer->copyRawTo(&ubo, sizeof (ubo));

        m_descriptorSet = descriptorPools->allocateDescriptor();
        updateDescriptorSet(inDevice);
    }

    inline std::shared_ptr<vulkan::DescriptorSet> const &descriptorSet() { return m_descriptorSet; }
    inline std::shared_ptr<vulkan::Buffer> const &uniformBuffer() { return m_uniformBuffer; }
    inline std::shared_ptr<vulkan::Buffer> const &uniformBufferLighting() { return m_uniformBufferLighting; }
    inline std::shared_ptr<vulkan::ImageSampler> const &imageSampler() { return m_sampler; }

private:
    void updateDescriptorSet(std::shared_ptr<vulkan::Device> const &inDevice);
};

class DrawObjectDataVulkan : public DrawObjectData {
public:
    DrawObjectDataVulkan(std::shared_ptr<vulkan::Device> const &inDevice,
                         std::shared_ptr<vulkan::CommandPool> const &inPool,
                         std::shared_ptr<vulkan::DescriptorPools> const &inDescriptorPools,
                         std::shared_ptr<DrawObject> const &drawObj,
                         std::shared_ptr<vulkan::Buffer> const &inLightingPosition)
            : m_device{inDevice},
              m_commandPool{inPool},
              m_descriptorPools{inDescriptorPools},
              m_vertexBuffer{m_device, sizeof(drawObj->vertices[0]) * drawObj->vertices.size(),
                             VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
              m_indexBuffer{m_device, sizeof(drawObj->indices[0]) * drawObj->indices.size(),
                            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
              m_uniformBufferLighting{inLightingPosition},
              m_uniforms{} {
        copyVerticesToBuffer(inPool, drawObj);
        copyIndicesToBuffer(inPool, drawObj);
    }

    void addUniforms(std::shared_ptr<DrawObject> const &obj,
                     glm::mat4 const &proj, glm::mat4 const &view, TextureMap &textures);
    void update(std::shared_ptr<DrawObject> const &obj,
                glm::mat4 const &proj, glm::mat4 const &view, TextureMap &textures);
    inline void clearUniforms() { m_uniforms.clear(); }

    inline vulkan::Buffer const &vertexBuffer() { return m_vertexBuffer; }
    inline vulkan::Buffer const &indexBuffer() { return m_indexBuffer; }
    inline std::vector<std::shared_ptr<UniformWrapper>> const &uniforms() { return m_uniforms; }

    virtual ~DrawObjectDataVulkan() = default;
private:
    std::shared_ptr<vulkan::Device> m_device;
    std::shared_ptr<vulkan::CommandPool> m_commandPool;
    std::shared_ptr<vulkan::DescriptorPools> m_descriptorPools;

    /* vertex buffer and index buffer. the index buffer indicates which vertices to draw and in
     * the specified order.  Note, vertices can be listed twice if they should be part of more
     * than one triangle.
     */
    vulkan::Buffer m_vertexBuffer;
    vulkan::Buffer m_indexBuffer;

    std::shared_ptr<vulkan::Buffer> m_uniformBufferLighting;
    std::vector<std::shared_ptr<UniformWrapper>> m_uniforms;

    void copyVerticesToBuffer(std::shared_ptr<vulkan::CommandPool> const &cmdpool,
                              std::shared_ptr<DrawObject> const &drawObj);

    void copyIndicesToBuffer(std::shared_ptr<vulkan::CommandPool> const &cmdpool,
                             std::shared_ptr<DrawObject> const &drawObj);
};

class UniformWrapperMVPOnly {
public:
    UniformWrapperMVPOnly(std::shared_ptr<vulkan::Device> const &inDevice,
                   std::shared_ptr<vulkan::DescriptorPools> const &descriptorPools,
                   glm::mat4 const &mvp)
            : m_descriptorSet{},
              m_uniformBuffer{}
    {
        VkBuffer uniformBuffer;
        VkDeviceMemory uniformBufferMemory;

        m_uniformBuffer = createUniformBuffer(inDevice, sizeof (mvp));
        m_uniformBuffer->copyRawTo(&mvp, sizeof (mvp));

        m_descriptorSet = descriptorPools->allocateDescriptor();
        updateDescriptorSet(inDevice);
    }

    inline std::shared_ptr<vulkan::DescriptorSet> const &descriptorSet() { return m_descriptorSet; }
    inline std::shared_ptr<vulkan::Buffer> const &uniformBuffer() { return m_uniformBuffer; }

private:
    void updateDescriptorSet(std::shared_ptr<vulkan::Device> const &inDevice) {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = m_uniformBuffer->buffer().get();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof (glm::mat4);

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_descriptorSet->descriptorSet().get();
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pNext = nullptr;
        descriptorWrite.pImageInfo = nullptr; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(inDevice->logicalDevice().get(), 1, &descriptorWrite, 0, nullptr);
    }

    /* for passing data other than the vertex data to the vertex shader */
    std::shared_ptr<vulkan::DescriptorSet> m_descriptorSet;
    std::shared_ptr<vulkan::Buffer> m_uniformBuffer;
};

class DrawObjectDataVulkanDepthTexture : public DrawObjectData {
public:
    DrawObjectDataVulkanDepthTexture(std::shared_ptr<vulkan::Device> const &inDevice,
                         std::shared_ptr<vulkan::CommandPool> const &inPool,
                         std::shared_ptr<vulkan::DescriptorPools> const &inDescriptorPools,
                         std::shared_ptr<DrawObject> const &drawObj)
            : m_device{inDevice},
              m_commandPool{inPool},
              m_descriptorPools{inDescriptorPools},
              m_vertexBuffer{m_device, sizeof(drawObj->vertices[0]) * drawObj->vertices.size(),
                             VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
              m_indexBuffer{m_device, sizeof(drawObj->indices[0]) * drawObj->indices.size(),
                            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
              m_uniforms{} {
        vulkan::copyVerticesToBuffer<Vertex>(inPool, drawObj->vertices, m_vertexBuffer);
        vulkan::copyIndicesToBuffer(inPool, drawObj->indices, m_indexBuffer);
    }

    void addUniforms(std::shared_ptr<DrawObject> const &obj, glm::mat4 const &vp) {
        if (obj->modelMatrices.size() == m_uniforms.size()) {
            return;
        }

        for (size_t i = m_uniforms.size(); i < obj->modelMatrices.size(); i++) {
            glm::mat4 mvp = vp * obj->modelMatrices[i];
            auto uniform = std::make_shared<UniformWrapperMVPOnly>(m_device, m_descriptorPools, mvp);
            m_uniforms.push_back(uniform);
        }
    }

    inline vulkan::Buffer const &vertexBuffer() { return m_vertexBuffer; }
    inline vulkan::Buffer const &indexBuffer() { return m_indexBuffer; }
    inline std::vector<std::shared_ptr<UniformWrapperMVPOnly>> const &uniforms() { return m_uniforms; }
private:
    std::shared_ptr<vulkan::Device> m_device;
    std::shared_ptr<vulkan::CommandPool> m_commandPool;
    std::shared_ptr<vulkan::DescriptorPools> m_descriptorPools;

    /* vertex buffer and index buffer. the index buffer indicates which vertices to draw and in
     * the specified order.  Note, vertices can be listed twice if they should be part of more
     * than one triangle.
     */
    vulkan::Buffer m_vertexBuffer;
    vulkan::Buffer m_indexBuffer;

    std::vector<std::shared_ptr<UniformWrapperMVPOnly>> m_uniforms;
};

class LevelSequenceVulkan : public LevelSequence {
public:
    LevelSequenceVulkan(std::shared_ptr<GameRequester> const &inRequester,
                        std::shared_ptr<vulkan::Device> const &inDevice,
                        std::shared_ptr<vulkan::CommandPool> const &inPool,
                        std::shared_ptr<vulkan::DescriptorPools> const &inDescriptorPools,
                        uint32_t width,
                        uint32_t height)
            :LevelSequence{inRequester, width, height, false},
             m_device{inDevice},
             m_commandPool{inPool},
             m_descriptorPools{inDescriptorPools},
             m_uniformBufferLighting{createUniformBuffer(inDevice, sizeof (glm::vec3))}
    {
    }

    inline bool needsInitializeCommandBuffers() { return m_level->isFinished() ||
                                                         m_levelFinisher->isUnveiling() || m_texturesChanged; }
    inline void doneInitializingCommandBuffers() { m_texturesChanged = false; }

protected:
    glm::mat4 getPerspectiveMatrixForLevel(uint32_t surfaceWidth, uint32_t surfaceHeight) override;
    void updatePerspectiveMatrix(uint32_t surfaceWidth, uint32_t surfaceHeight) override;
    std::shared_ptr<TextureData> createTexture(std::shared_ptr<TextureDescription> const &textureDescription) override;
    std::shared_ptr<DrawObjectData> createObject(std::shared_ptr<DrawObject> const &obj, TextureMap &textures) override;
    void updateLevelData(DrawObjectTable &objsData, TextureMap &textures) override;
    void setupLightingSourceBuffer() override {
        m_uniformBufferLighting->copyRawTo(&m_lightingSource, sizeof (m_lightingSource));
    }

private:
    std::shared_ptr<vulkan::Device> m_device;
    std::shared_ptr<vulkan::CommandPool> m_commandPool;
    std::shared_ptr<vulkan::DescriptorPools> m_descriptorPools;
    std::shared_ptr<vulkan::Buffer> m_uniformBufferLighting;
};

class GraphicsVulkan : public Graphics {
public:
    GraphicsVulkan(std::shared_ptr<WindowType> window,
            GameRequesterCreator inRequesterCreator)
            : Graphics{inRequesterCreator},
              m_instance{new vulkan::Instance(std::move(window))},
              m_device{new vulkan::Device{m_instance}},
              m_swapChain{new vulkan::SwapChain{m_device}},
              m_renderPass{vulkan::RenderPass::createRenderPass(m_device, m_swapChain)},
              m_descriptorSetLayout{new AmazingLabyrinthDescriptorSetLayout{m_device}},
              m_descriptorPools{new vulkan::DescriptorPools{m_device, m_descriptorSetLayout}},
              m_graphicsPipeline{new vulkan::Pipeline{m_gameRequester, m_device, m_swapChain->extent(),
                                                      m_renderPass, m_descriptorPools,
                                                      getBindingDescription(), getAttributeDescriptions(),
                                                      SHADER_VERT_FILE, SHADER_FRAG_FILE, nullptr,
                                                      true}},
              m_commandPool{new vulkan::CommandPool{m_device}},
              m_depthImageView{new vulkan::ImageView{vulkan::ImageFactory::createDepthImage(m_swapChain),
                                                     m_device->depthFormat(), VK_IMAGE_ASPECT_DEPTH_BIT}},
              m_swapChainCommands{new vulkan::SwapChainCommands{m_swapChain, m_commandPool, m_renderPass, m_depthImageView}},
              m_imageAvailableSemaphore{m_device},
              m_renderFinishedSemaphore{m_device}
    {
        m_levelSequence = std::make_shared<LevelSequenceVulkan>(m_gameRequester, m_device, m_commandPool, m_descriptorPools,
                        m_swapChain->extent().width, m_swapChain->extent().height);

        prepareDepthResources();

        //initializeCommandBuffers();
    }

    virtual void initThread() { }

    virtual void cleanupThread() { }

    virtual bool updateData() { return m_levelSequence->updateData(); }

    virtual void drawFrame();

    virtual void recreateSwapChain(uint32_t width, uint32_t height);

    virtual GraphicsDescription graphicsDescription() {
        auto devGraphicsDescription = m_device->properties();
        return GraphicsDescription{std::string{"Vulkan"},
                std::move(devGraphicsDescription.m_vulkanAPIVersion),
                std::move(devGraphicsDescription.m_name)};
    }

    virtual std::shared_ptr<TextureData> getDepthTexture(
            DrawObjectTable const &objsData,
            float width,
            float height);

    virtual ~GraphicsVulkan() { }
private:
    std::shared_ptr<vulkan::Instance> m_instance;
    std::shared_ptr<vulkan::Device> m_device;
    std::shared_ptr<vulkan::SwapChain> m_swapChain;

    std::shared_ptr<vulkan::RenderPass> m_renderPass;

    std::shared_ptr<AmazingLabyrinthDescriptorSetLayout> m_descriptorSetLayout;
    std::shared_ptr<vulkan::DescriptorPools> m_descriptorPools;
    std::shared_ptr<vulkan::Pipeline> m_graphicsPipeline;
    std::shared_ptr<vulkan::CommandPool> m_commandPool;

    /* depth buffer image */
    std::shared_ptr<vulkan::ImageView> m_depthImageView;

    std::shared_ptr<vulkan::SwapChainCommands> m_swapChainCommands;

    /* use semaphores to coordinate the rendering and presentation. Could also use fences
     * but fences are more for coordinating in our program itself and not for internal
     * Vulkan coordination of resource usage.
     */
    vulkan::Semaphore m_imageAvailableSemaphore;
    vulkan::Semaphore m_renderFinishedSemaphore;

    void cleanupSwapChain();
    void initializeCommandBuffers();
    void initializeCommandBuffer(size_t index);
    void initializeCommandBufferDrawObjects(VkCommandBuffer commandBuffer, DrawObjectTable const & objs);
    void prepareDepthResources();
};

#endif // AMAZING_LABYRINTH_MAZE_VULKAN_HPP