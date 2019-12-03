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
#include "mazeVulkan.hpp"
#include "../../../../../../Android/Sdk/ndk/20.0.5594570/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/memory"

VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription = {};

    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);

    /* move to the next data entry after each vertex.  VK_VERTEX_INPUT_RATE_INSTANCE
     * moves to the next data entry after each instance, but we are not using instanced
     * rendering
     */
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

/* for accessing data other than the vertices from the shaders */
void AmazingLabyrinthDescriptorSetLayout::createDescriptorSetLayout() {
    /* MVP matrix */
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;

    /* only accessing the MVP matrix from the vertex shader */
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    /* image sampler */
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding lightingSourceBinding = {};
    lightingSourceBinding.binding = 2;
    lightingSourceBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    lightingSourceBinding.descriptorCount = 1;
    lightingSourceBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    lightingSourceBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 3> bindings = {uboLayoutBinding,
                                                            samplerLayoutBinding,
                                                            lightingSourceBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    VkDescriptorSetLayout descriptorSetLayoutRaw;
    if (vkCreateDescriptorSetLayout(m_device->logicalDevice().get(), &layoutInfo, nullptr,
                                    &descriptorSetLayoutRaw) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    auto const &capDevice = m_device;
    auto deleter = [capDevice](VkDescriptorSetLayout descriptorSetLayoutRaw) {
        vkDestroyDescriptorSetLayout(capDevice->logicalDevice().get(), descriptorSetLayoutRaw, nullptr);
    };

    m_descriptorSetLayout.reset(descriptorSetLayoutRaw, deleter);
}

std::shared_ptr<vulkan::Buffer> UniformWrapper::createUniformBuffer(
        std::shared_ptr<vulkan::Device> const &device, size_t bufferSize) {
    return std::shared_ptr<vulkan::Buffer>{new vulkan::Buffer{device, bufferSize,
                                                              VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};
}

/* descriptor set for the MVP matrix and texture samplers */
void UniformWrapper::updateDescriptorSet(std::shared_ptr<vulkan::Device> const &inDevice) {
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = m_uniformBuffer->buffer().get();
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    std::array<VkWriteDescriptorSet, 3> descriptorWrites = {};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = m_descriptorSet->descriptorSet().get();

    /* must be the same as the binding in the vertex shader */
    descriptorWrites[0].dstBinding = 0;

    /* index into the array of descriptors */
    descriptorWrites[0].dstArrayElement = 0;

    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    /* how many array elements you want to update */
    descriptorWrites[0].descriptorCount = 1;

    /* which one of these pointers needs to be used depends on which descriptorType we are
     * using.  pBufferInfo is for buffer based data, pImageInfo is used for image data, and
     * pTexelBufferView is used for decriptors that refer to buffer views.
     */
    descriptorWrites[0].pBufferInfo = &bufferInfo;
    descriptorWrites[0].pImageInfo = nullptr; // Optional
    descriptorWrites[0].pTexelBufferView = nullptr; // Optional

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_sampler->imageView()->imageView().get();
    imageInfo.sampler = m_sampler->sampler().get();

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = m_descriptorSet->descriptorSet().get();
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    VkDescriptorBufferInfo bufferLightingSource = {};
    bufferLightingSource.buffer = m_uniformBufferLighting->buffer().get();
    bufferLightingSource.offset = 0;
    bufferLightingSource.range = sizeof(glm::vec3);

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = m_descriptorSet->descriptorSet().get();
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pBufferInfo = &bufferLightingSource;

    vkUpdateDescriptorSets(inDevice->logicalDevice().get(),
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);
}

void DrawObjectDataVulkan::copyVerticesToBuffer(std::shared_ptr<vulkan::CommandPool> const &cmdpool,
                                                std::shared_ptr<DrawObject> const &drawObj) {
    vulkan::copyVerticesToBuffer<Vertex>(cmdpool, drawObj->vertices, m_vertexBuffer);
}

/* buffer for the indices - used to reference which vertex in the vertex array (by index) to
 * draw.  This way normally duplicated vertices would not need to be specified twice.
 * Only one index buffer per pipeline is allowed.  Put all dice in the same index buffer.
 */
void DrawObjectDataVulkan::copyIndicesToBuffer(std::shared_ptr<vulkan::CommandPool> const &cmdpool,
                                               std::shared_ptr<DrawObject> const &drawObj) {
    vulkan::copyIndicesToBuffer(cmdpool, drawObj->indices, m_indexBuffer);
}

void DrawObjectDataVulkan::addUniforms(std::shared_ptr<DrawObject> const &obj,
                                       glm::mat4 const &proj, glm::mat4 const &view,
                                       TextureMap &textures) {
    if (obj->modelMatrices.size() == m_uniforms.size()) {
        return;
    }

    UniformBufferObject ubo;
    ubo.proj = proj;
    ubo.view = view;

    for (size_t i = m_uniforms.size(); i < obj->modelMatrices.size(); i++) {
        ubo.model = obj->modelMatrices[i];
        TextureMap::iterator it = textures.find(obj->texture);
        if (it == textures.end()) {
            throw std::runtime_error("Could not find texture in texture map.");
        }
        TextureDataVulkan *textureData = static_cast<TextureDataVulkan *> (it->second.get());
        std::shared_ptr<UniformWrapper> uniform(new UniformWrapper(m_device, m_descriptorPools,
                                                                   textureData->sampler(), m_uniformBufferLighting, ubo));
        m_uniforms.push_back(uniform);
    }
}

void DrawObjectDataVulkan::update(std::shared_ptr<DrawObject> const &obj,
                                  glm::mat4 const &proj, glm::mat4 const &view, TextureMap &textures) {
    if (m_uniforms.size() < obj->modelMatrices.size()) {
        // a new model matrix (new object but same texture, vertices and indices).
        addUniforms(obj, proj, view, textures);
    } else if (m_uniforms.size() > obj->modelMatrices.size()) {
        // a model matrix got removed...
        m_uniforms.pop_back();
    }

    UniformBufferObject ubo;
    ubo.proj = proj;
    ubo.view = view;
    // copy over the model matrices into the graphics card memory... they might
    // have changed.
    for (size_t j = 0; j < obj->modelMatrices.size(); j++) {
        ubo.model = obj->modelMatrices[j];
        m_uniforms[j]->uniformBuffer()->copyRawTo(&ubo, sizeof (ubo));
    }
}

void LevelSequenceVulkan::updateLevelData(DrawObjectTable &objsData, TextureMap &textures) {
    if (m_texturesChanged) {
        addTextures(textures);
        for (auto &&obj : objsData) {
            DrawObjectDataVulkan *objData =
                    dynamic_cast<DrawObjectDataVulkan *> (obj.second.get());
            if (objData != nullptr) {
                objData->clearUniforms();
            }
        }
    }

    for (auto &&objData : objsData) {
        DrawObjectDataVulkan *data = dynamic_cast<DrawObjectDataVulkan *> (objData.second.get());
        if (data == nullptr) {
            // a completely new entry
            objData.second = createObject(objData.first, textures);
        } else {
            data->update(objData.first, m_proj, m_view, textures);
        }
    }
}

std::shared_ptr<TextureData> LevelSequenceVulkan::createTexture(std::shared_ptr<TextureDescription> const &textureDescription) {
    return std::make_shared<TextureDataVulkan>(m_device, m_commandPool, textureDescription);
}

std::shared_ptr<DrawObjectData> LevelSequenceVulkan::createObject(std::shared_ptr<DrawObject> const &obj, TextureMap &textures) {
    std::shared_ptr<DrawObjectDataVulkan> objData =
            std::make_shared<DrawObjectDataVulkan>(m_device, m_commandPool,
                                                   m_descriptorPools, obj, m_uniformBufferLighting);

    objData->addUniforms(obj, m_proj, m_view, textures);
    return objData;
}

void GraphicsVulkan::recreateSwapChain(uint32_t width, uint32_t height) {
    vkDeviceWaitIdle(m_device->logicalDevice().get());

    cleanupSwapChain();

    m_swapChain.reset(new vulkan::SwapChain(m_device));
    m_depthImageView.reset(new vulkan::ImageView{vulkan::ImageFactory::createDepthImage(m_swapChain),
                                                 m_device->depthFormat(), VK_IMAGE_ASPECT_DEPTH_BIT});
    prepareDepthResources();
    m_renderPass = vulkan::RenderPass::createRenderPass(m_device, m_swapChain);
    m_graphicsPipeline.reset(new vulkan::Pipeline{m_gameRequester, m_device, m_swapChain->extent(),
                                                  m_renderPass, m_descriptorPools,
                                                  getBindingDescription(), getAttributeDescriptions()});
    m_swapChainCommands.reset(new vulkan::SwapChainCommands{m_swapChain, m_commandPool, m_renderPass,
                                                            m_depthImageView});
}

void GraphicsVulkan::cleanupSwapChain() {
    if (m_device->logicalDevice().get() == VK_NULL_HANDLE) {
        return;
    }

    m_swapChainCommands.reset();

    m_graphicsPipeline.reset();
    m_renderPass.reset();

    m_depthImageView.reset();

    m_swapChain.reset();
}

void GraphicsVulkan::initializeCommandBuffers() {
    /* begin recording commands into each comand buffer */
    for (size_t i = 0; i < m_swapChainCommands->size(); i++) {
        initializeCommandBuffer(i);
    }
}

void GraphicsVulkan::initializeCommandBuffer(size_t index) {
    VkCommandBuffer commandBuffer = m_swapChainCommands->commandBuffer(index);
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    /* pInheritanceInfo is only used if flags includes:
     * VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
     * because that would mean that there is a secondary command buffer that will be used
     * in a single render pass
     */
    beginInfo.pInheritanceInfo = nullptr; // Optional

    /* this call will reset the command buffer.  its not possible to append commands at
     * a later time.
     */
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    /* begin the render pass: drawing starts here*/
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass->renderPass().get();
    renderPassInfo.framebuffer = m_swapChainCommands->frameBuffer(index);
    /* size of the render area */
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapChain->extent();

    /* the color value to use when clearing the image with VK_ATTACHMENT_LOAD_OP_CLEAR,
     * using black with 0% opacity
     */
    std::array<VkClearValue, 2> clearValues = {};
    glm::vec4 bgColor = m_levelSequence->backgroundColor();
    clearValues[0].color = {bgColor.r, bgColor.g, bgColor.b, bgColor.a};
    clearValues[1].depthStencil = {1.0, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    /* begin recording commands - start by beginning the render pass.
     * none of these functions returns an error (they return void).  There will be no error
     * handling until recording is done.
     */
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    /* bind the graphics pipeline to the command buffer, the second parameter tells Vulkan
     * that we are binding to a graphics pipeline.
     */
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline->pipeline().get());

    // the objects that stay static.
    initializeCommandBufferDrawObjects(commandBuffer, m_levelSequence->levelStaticObjsData());

    // the objects that move.
    initializeCommandBufferDrawObjects(commandBuffer, m_levelSequence->levelDynObjsData());

    // the level starter
    initializeCommandBufferDrawObjects(commandBuffer, m_levelSequence->starterStaticObjsData());
    initializeCommandBufferDrawObjects(commandBuffer, m_levelSequence->starterDynObjsData());

    // the level finisher objects.
    if (m_levelSequence->needFinisherObjs()) {
        initializeCommandBufferDrawObjects(commandBuffer, m_levelSequence->finisherObjsData());
    }

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void GraphicsVulkan::initializeCommandBufferDrawObjects(VkCommandBuffer commandBuffer, DrawObjectTable const &objs) {
    VkDeviceSize offsets[1] = {0};

    for (auto &&obj : objs) {
        DrawObjectDataVulkan *objData = dynamic_cast<DrawObjectDataVulkan*> (obj.second.get());

        VkBuffer vertexBuffer = objData->vertexBuffer().buffer().get();
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, objData->indexBuffer().buffer().get(), 0, VK_INDEX_TYPE_UINT32);
        for (auto &&uniform : objData->uniforms()) {
            /* The MVP matrix and texture samplers */
            VkDescriptorSet descriptorSet = uniform->m_descriptorSet->descriptorSet().get();
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    m_graphicsPipeline->layout().get(), 0, 1, &descriptorSet, 0, nullptr);

            /* indexed draw command:
             * parameter 1 - Command buffer for the draw command
             * parameter 2 - the number of indices (the vertex count)
             * parameter 3 - the instance count, use 1 because we are not using instanced rendering
             * parameter 4 - offset into the index buffer
             * parameter 5 - offset to add to the indices in the index buffer
             * parameter 6 - offset for instance rendering
             */
            vkCmdDrawIndexed(commandBuffer, obj.first->indices.size(), 1, 0, 0, 0);
        }
    }
}

void GraphicsVulkan::drawFrame() {
    /* update the app state here */

    /* wait for presentation to finish before drawing the next frame.  Avoids a memory leak */
    vkQueueWaitIdle(m_device->presentQueue());

    uint32_t imageIndex;
    /* the third parameter is a timeout indicating how much time in nanoseconds we want to
     * wait for the image to become available (std::numeric_limits<uint64_t>::max() disables it.
     * an error from this function does not necessarily mean that we need to terminate
     * the program
     */
    VkResult result = vkAcquireNextImageKHR(m_device->logicalDevice().get(), m_swapChain->swapChain().get(),
                                            std::numeric_limits<uint64_t>::max(), m_imageAvailableSemaphore.semaphore().get(),
                                            VK_NULL_HANDLE, &imageIndex);

    //if (maze->isFinished() || levelFinisher->isUnveiling() || texturesChanged) {
    // The user completed the maze or the textures changed.  If the maze is completed, we need
    // to display the level finished animation.  Since there are additional objects, we need to
    // rewrite the command buffer to display the new objects.  If the textures changed, then we
    // need to update the descriptor sets, so the command buffers need to be rewritten.
    initializeCommandBuffer(imageIndex);
    //}

    /* If the window surface is no longer compatible with the swap chain, then we need to
     * recreate the swap chain and let the next call draw the image.
     * VK_SUBOPTIMAL_KHR means that the swap chain can still be used to present to the surface
     * but it no longer matches the window surface exactly.
     */
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain(0, 0);
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    /* wait for the semaphore before writing to the color attachment.  This means that we
     * could start executing the vertex shader before the image is available.
     */
    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphore.semaphore().get()};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    /* use the command buffer that corresponds to the image we just acquired */
    submitInfo.commandBufferCount = 1;
    VkCommandBuffer commandBuffer = m_swapChainCommands->commandBuffer(imageIndex);
    submitInfo.pCommandBuffers = &commandBuffer;

    /* indicate which semaphore to signal when execution is done */
    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphore.semaphore().get()};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    /* the last parameter is a fence to indicate when execution is done, but we are using
     * semaphores instead so pass VK_NULL_HANDLE
     */
    if (vkQueueSubmit(m_device->graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    /* submit the image back to the swap chain to have it eventually show up on the screen */
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    /* wait for the renderFinishedSemaphore to be signaled before presentation */
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    /* which swap chains to present the image to and the index of the image for
     * each swap chain
     */
    VkSwapchainKHR swapChains[] = {m_swapChain->swapChain().get()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    /* get an array of return codes for each swap chain.  Since there is only one, the
     * function return code can just be checked.
     */
    presentInfo.pResults = nullptr; // Optional

    result = vkQueuePresentKHR(m_device->presentQueue(), &presentInfo);

    /* If the window surface is no longer compatible with the swap chain
     * (VK_ERROR_OUT_OF_DATE_KHR), then we need to recreate the swap chain and let the next
     * call draw the image. VK_SUBOPTIMAL_KHR means that the swap chain can still be used
     * to present to the surface but it no longer matches the window surface exactly.
     * We recreate the swap chain in this case too.
     */
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreateSwapChain(0, 0);
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    result = vkQueueWaitIdle(m_device->presentQueue());
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to wait on present queue.");
    }
}


void GraphicsVulkan::prepareDepthResources() {
    VkFormat depthFormat = m_device->depthFormat();

    m_depthImageView->image()->transitionImageLayout(depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
                                                     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, m_commandPool);
}

class DepthTextureDescriptorSetLayout : public vulkan::DescriptorSetLayout {
public:
    DepthTextureDescriptorSetLayout(std::shared_ptr<vulkan::Device> inDevice)
            : m_device(inDevice)
    {
        m_poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        m_poolSizes[0].descriptorCount = m_numberOfDescriptorSetsInPool;
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
    virtual ~DepthTextureDescriptorSetLayout() {}

private:
    static uint32_t constexpr m_numberOfDescriptorSetsInPool = 1;

    std::shared_ptr<vulkan::Device> m_device;
    std::shared_ptr<VkDescriptorSetLayout_T> m_descriptorSetLayout;
    VkDescriptorPoolCreateInfo m_poolInfo;
    std::array<VkDescriptorPoolSize, 1> m_poolSizes;

    void createDescriptorSetLayout();
};

/* for accessing data other than the vertices from the shaders */
void DepthTextureDescriptorSetLayout::createDescriptorSetLayout() {
    /* MVP matrix */
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;

    /* only accessing the MVP matrix from the vertex shader */
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    VkDescriptorSetLayout descriptorSetLayoutRaw;
    if (vkCreateDescriptorSetLayout(m_device->logicalDevice().get(), &layoutInfo, nullptr,
                                    &descriptorSetLayoutRaw) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout for depth texture!");
    }

    auto const &capDevice = m_device;
    auto deleter = [capDevice](VkDescriptorSetLayout descriptorSetLayoutRaw) {
        vkDestroyDescriptorSetLayout(capDevice->logicalDevice().get(), descriptorSetLayoutRaw, nullptr);
    };

    m_descriptorSetLayout.reset(descriptorSetLayoutRaw, deleter);
}

std::vector<std::vector<float>> GraphicsVulkan::getDepthTexture(
        std::vector<Vertex> const &vertices,
        std::vector<uint32_t> indices,
        float width,
        float height,
        float zPos)
{
    DepthTextureDescriptorSetLayout dscLayout{m_device};
    vulkan::DescriptorPools dscPools{m_device, dscLayout};
    auto dsc = dscPools.allocateDescriptor();

    glm::mat4 proj = glm::ortho(-width/2.0f, width/2.0f, -height/2.0f, height/2.0f, 0.1f, 100.0f);
    glm::mat4 model = glm::translate(glm::vec3{0.0f, 0.0f, zPos});
    glm::mat4 ubo = proj * m_levelSequence->viewMatrix() * model;

    auto uboBuffer = std::make_shared<vulkan::Buffer>(m_device, sizeof (ubo),
                             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    uboBuffer.copyRawTo(&ubo, sizeof (ubo));

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = uboBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(uboBuffer);

    vulkan::Buffer vertexBuffer{m_device, vertices[0]) * vertices.size(),
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};
    vulkan::copyVerticesToBuffer<Vertex>(m_commandPool, vertices, vertexBuffer);

    vulkan::Buffer indexBuffer{m_device, sizeof(indices[0]) * indices.size(),
                           VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};
    vulkan::copyIndicesToBuffer(m_commandPool, indices, indexBuffer);

    VkWriteDescriptorSet descriptorWrite;
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = dsc->descriptorSet().get();
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.pImageInfo = nullptr; // Optional
    descriptorWrite.pTexelBufferView = nullptr; // Optional

    vkUpdateDescriptorSets(m_device->logicalDevice().get(), 1, &descriptorWrite, 0, nullptr);

    auto depthView = std::make_shared<vulkan::ImageView>(vulkan::ImageFactory::createDepthImage(m_swapChain),
        m_device->depthFormat(), VK_IMAGE_ASPECT_DEPTH_BIT);
    depthView->image()->transitionImageLayout(m_device->depthFormat(), VK_IMAGE_LAYOUT_UNDEFINED,
                                              VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, m_commandPool);

    auto renderPass = vulkan::RenderPass::createDepthTextureRenderPass(m_device);

    std::vector<std::shared_ptr<vulkan::ImageView>> attachments = {depthView};
    auto frameBuffer = std::make_shared<vulkan::Framebuffer>(m_device, renderPass, attachments,
            m_swapChain->extent().width, m_swapChain->extent().height);

    auto pipeline = std::make_shared<vulkan::Pipeline>(m_device, m_swapChain->extent(), renderPass,
            dscPools, getBindingDescription(), getAttributeDescriptions(), m_graphicsPipeline, false);

    // start recording commands
    vulkan::SingleTimeCommands cmds{m_device, m_commandPool};
    cmds.begin();

    /* begin the render pass: drawing starts here*/
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass->renderPass().get();
    renderPassInfo.framebuffer = frameBuffer->framebuffer().get();
    /* size of the render area */
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapChain->extent();

    /* the color value to use when clearing the image with VK_ATTACHMENT_LOAD_OP_CLEAR,
     * using black with 0% opacity
     */
    VkClearValue clearValue = {};
    clearValue.depthStencil = {1.0, 0};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearValue;

    /* begin recording commands - start by beginning the render pass.
     * none of these functions returns an error (they return void).  There will be no error
     * handling until recording is done.
     */
    vkCmdBeginRenderPass(cmds.commandBuffer().get(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmds.commandBuffer().get(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(cmds.commandBuffer().get(), VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline->pipelineLayout().get(), 0, 1, dsc, 0, nullptr);

    VkDeviceSize offsets[1] = {0};
    VkBuffer vertexBufferRaw = vertexBuffer.buffer().get();
    vkCmdBindVertexBuffers(cmds.commandBuffer().get(), 0, 1, &vertexBufferRaw, offsets);

    vkCmdBindIndexBuffer(cmds.commandBuffer().get(), indexBuffer->buffer().get(), 0, VK_INDEX_TYPE_UINT32);
}