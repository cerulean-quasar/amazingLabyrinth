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
#include <unistd.h>
#include <glm/glm.hpp>

#include "mazeVulkan.hpp"
#include "mathGraphics.hpp"

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

std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {};

    attributeDescriptions.resize(4);

    /* position */
    attributeDescriptions[0].binding = 0; /* binding description to use */
    attributeDescriptions[0].location = 0; /* matches the location in the vertex shader */
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    /* color */
    attributeDescriptions[1].binding = 0; /* binding description to use */
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    /* texture coordinate */
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    /* normal vector */
    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex, normal);
    return attributeDescriptions;
}

/* for accessing data other than the vertices from the shaders */
void AmazingLabyrinthDescriptorSetLayout::createDescriptorSetLayout() {
    /* model matrix - different for each object */
    VkDescriptorSetLayoutBinding modelMatrixBinding = {};
    modelMatrixBinding.binding = 0;
    modelMatrixBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    modelMatrixBinding.descriptorCount = 1;

    /* only accessing the MVP matrix from the vertex shader */
    modelMatrixBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    modelMatrixBinding.pImmutableSamplers = nullptr; // Optional

    /* view and projection matrix - the same for all objects */
    VkDescriptorSetLayoutBinding commonDataBinding = {};
    commonDataBinding.binding = 1;
    commonDataBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    commonDataBinding.descriptorCount = 1;
    commonDataBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    commonDataBinding.pImmutableSamplers = nullptr; // Optional

    /* image sampler */
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 2;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding lightingSourceBinding = {};
    lightingSourceBinding.binding = 3;
    lightingSourceBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    lightingSourceBinding.descriptorCount = 1;
    lightingSourceBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    lightingSourceBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerShadows = {};
    samplerShadows.binding = 4;
    samplerShadows.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerShadows.descriptorCount = 1;
    samplerShadows.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerShadows.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 5> bindings = {modelMatrixBinding,
                                                            commonDataBinding,
                                                            samplerLayoutBinding,
                                                            lightingSourceBinding,
                                                            samplerShadows};

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

/* for accessing data other than the vertices from the shaders */
void AmazingLabyrinthShadowsDescriptorSetLayout::createDescriptorSetLayout() {
    /* model matrix */
    VkDescriptorSetLayoutBinding perObject = {};
    perObject.binding = 0;
    perObject.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    perObject.descriptorCount = 1;
    perObject.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    perObject.pImmutableSamplers = nullptr; // Optional

    /* the projection, view, and view light source matrix */
    VkDescriptorSetLayoutBinding commonData = {};
    commonData.binding = 1;
    commonData.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    commonData.descriptorCount = 1;
    commonData.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    commonData.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {perObject, commonData};

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

std::shared_ptr<vulkan::Buffer> createUniformBuffer(
        std::shared_ptr<vulkan::Device> const &device, size_t bufferSize) {
    return std::shared_ptr<vulkan::Buffer>{new vulkan::Buffer{device, bufferSize,
                                                              VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};
}

/* descriptor set for the MVP matrix and texture samplers */
void UniformWrapper::updateDescriptorSet(std::shared_ptr<vulkan::Device> const &inDevice) {
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = m_uniformBuffer->buffer();
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(PerObjectUBO);

    std::array<VkWriteDescriptorSet, 5> descriptorWrites = {};
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

    VkDescriptorBufferInfo commonInfo = {};
    commonInfo.buffer = m_commonUBO->buffer();
    commonInfo.offset = 0;
    commonInfo.range = sizeof(CommonUBO);

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = m_descriptorSet->descriptorSet().get();
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &commonInfo;

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_sampler->imageView()->imageView().get();
    imageInfo.sampler = m_sampler->sampler().get();

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = m_descriptorSet->descriptorSet().get();
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pImageInfo = &imageInfo;

    VkDescriptorBufferInfo bufferLightingSource = {};
    bufferLightingSource.buffer = m_uniformBufferLighting->buffer();
    bufferLightingSource.offset = 0;
    bufferLightingSource.range = sizeof(glm::vec3);

    descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[3].dstSet = m_descriptorSet->descriptorSet().get();
    descriptorWrites[3].dstBinding = 3;
    descriptorWrites[3].dstArrayElement = 0;
    descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[3].descriptorCount = 1;
    descriptorWrites[3].pBufferInfo = &bufferLightingSource;

    VkDescriptorImageInfo shadowInfo = {};
    shadowInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    shadowInfo.imageView = m_shadows->imageView()->imageView().get();
    shadowInfo.sampler = m_shadows->sampler().get();

    descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[4].dstSet = m_descriptorSet->descriptorSet().get();
    descriptorWrites[4].dstBinding = 4;
    descriptorWrites[4].dstArrayElement = 0;
    descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[4].descriptorCount = 1;
    descriptorWrites[4].pImageInfo = &shadowInfo;

    vkUpdateDescriptorSets(inDevice->logicalDevice().get(),
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);

    descriptorWrites[0].dstSet = m_descriptorSetShadows->descriptorSet().get();
    descriptorWrites[1].dstSet = m_descriptorSetShadows->descriptorSet().get();
    vkUpdateDescriptorSets(inDevice->logicalDevice().get(), 2,
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
                                       TextureMap &textures) {
    if (obj->modelMatrices.size() == m_uniforms.size()) {
        return;
    }

    PerObjectUBO ubo;

    for (size_t i = m_uniforms.size(); i < obj->modelMatrices.size(); i++) {
        ubo.model = obj->modelMatrices[i];
        auto it = textures.find(obj->texture);
        if (it == textures.end()) {
            throw std::runtime_error("Could not find texture in texture map.");
        }
        TextureDataVulkan *textureData = static_cast<TextureDataVulkan *> (it->second.get());
        auto uniform = std::make_shared<UniformWrapper>(
                m_device,
                m_descriptorPools,
                m_descriptorPoolsShadows,
                textureData->sampler(),
                m_shadows,
                m_uniformBufferLighting,
                m_commonUBO,
                ubo);
        m_uniforms.push_back(uniform);
    }
}

void DrawObjectDataVulkan::update(std::shared_ptr<DrawObject> const &obj, TextureMap &textures) {
    if (m_uniforms.size() < obj->modelMatrices.size()) {
        // a new model matrix (new object but same texture, vertices and indices).
        addUniforms(obj, textures);
    } else if (m_uniforms.size() > obj->modelMatrices.size()) {
        // a model matrix got removed...
        m_uniforms.pop_back();
    }

    // copy over the model matrices into the graphics card memory... they might
    // have changed.
    for (size_t j = 0; j < obj->modelMatrices.size(); j++) {
        PerObjectUBO ubo;
        ubo.model = obj->modelMatrices[j];
        m_uniforms[j]->uniformBuffer()->copyRawTo(&ubo, sizeof (ubo));
    }
}

glm::mat4 LevelSequenceVulkan::getPerspectiveMatrixForLevel(uint32_t surfaceWidth, uint32_t surfaceHeight) {
    /* perspective matrix: takes the perspective projection, the aspect ratio, near and far
     * view planes.
     */
    return getPerspectiveMatrix(m_perspectiveViewAngle,
                                surfaceWidth / static_cast<float>(surfaceHeight),
                                m_perspectiveNearPlane, m_perspectiveFarPlane,
                                false, true);
}

void LevelSequenceVulkan::updatePerspectiveMatrix(uint32_t surfaceWidth, uint32_t surfaceHeight) {
    m_proj = getPerspectiveMatrix(m_perspectiveViewAngle,
                                surfaceWidth / static_cast<float>(surfaceHeight),
                                m_perspectiveNearPlane, m_perspectiveFarPlane,
                                true, true);
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
            data->update(objData.first, textures);
        }
    }
}

std::shared_ptr<TextureData> LevelSequenceVulkan::createTexture(std::shared_ptr<TextureDescription> const &textureDescription) {
    return std::make_shared<TextureDataVulkan>(m_device, m_commandPool, textureDescription);
}

std::shared_ptr<DrawObjectData> LevelSequenceVulkan::createObject(std::shared_ptr<DrawObject> const &obj, TextureMap &textures) {
    auto objData =
            std::make_shared<DrawObjectDataVulkan>(m_device, m_commandPool,
                                                   m_descriptorPools, m_descriptorPoolsShadows, obj,
                                                   m_shadows, m_uniformBufferLighting, m_commonUBO);

    objData->addUniforms(obj, textures);
    return objData;
}

void GraphicsVulkan::recreateSwapChain(uint32_t, uint32_t) {
    vkDeviceWaitIdle(m_device->logicalDevice().get());

    cleanupSwapChain();

    m_swapChain.reset(new vulkan::SwapChain(m_device));
    m_depthImageView.reset(new vulkan::ImageView{vulkan::ImageFactory::createDepthImage(m_swapChain),
                                                 VK_IMAGE_ASPECT_DEPTH_BIT});
    prepareDepthResources();
    m_renderPass = vulkan::RenderPass::createRenderPass(m_device, m_swapChain);
    m_graphicsPipeline.reset(new vulkan::Pipeline{m_gameRequester, m_device, m_swapChain->extent(),
                                                  m_renderPass, m_descriptorPools,
                                                  getBindingDescription(), getAttributeDescriptions(),
                                                  SHADER_VERT_FILE, SHADER_FRAG_FILE,
                                                  nullptr});
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

void GraphicsVulkan::initializeCommandBuffer(
    VkCommandBuffer const &commandBuffer,
    VkFramebuffer const &framebuffer,
    std::shared_ptr<vulkan::Pipeline> const &pipeline,
    std::vector<VkClearValue> const &clearValues,
    bool doShadows)
{
    /* begin the render pass: drawing starts here*/
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = pipeline->renderPass()->renderPass().get();
    renderPassInfo.framebuffer = framebuffer;
    /* size of the render area */
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = pipeline->extent();

    /* the color value to use when clearing the image with VK_ATTACHMENT_LOAD_OP_CLEAR,
     * using black with 0% opacity
     */
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
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline().get());

    // the objects that stay static.
    initializeCommandBufferDrawObjects(commandBuffer, m_levelSequence->levelStaticObjsData(), pipeline, doShadows);

    // the objects that move.
    initializeCommandBufferDrawObjects(commandBuffer, m_levelSequence->levelDynObjsData(), pipeline, doShadows);

    // only do shadows for the level itself

    // the level starter
    if (!doShadows) {
        initializeCommandBufferDrawObjects(commandBuffer, m_levelSequence->starterStaticObjsData(), pipeline, doShadows);
        initializeCommandBufferDrawObjects(commandBuffer, m_levelSequence->starterDynObjsData(), pipeline, doShadows);
    }

    // the level finisher objects.
    if (m_levelSequence->needFinisherObjs() && !doShadows) {
        initializeCommandBufferDrawObjects(commandBuffer, m_levelSequence->finisherObjsData(), pipeline, doShadows);
    }

    vkCmdEndRenderPass(commandBuffer);
}

void GraphicsVulkan::initializeCommandBuffer(uint32_t cmdBufferIndex) {
    VkCommandBuffer commandBuffer = m_swapChainCommands->commandBuffer(cmdBufferIndex);
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

    std::vector<VkClearValue> clearValues;
    clearValues.resize(2);
    glm::vec4 bgColor = m_levelSequence->backgroundColor();
    clearValues[0].color = {1.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0, 0};

    initializeCommandBuffer(commandBuffer,
                            m_framebufferShadows->framebuffer().get(),
                            m_pipelineShadows, clearValues, true);
    clearValues[0].color = {bgColor.r, bgColor.g, bgColor.b, bgColor.a};
    initializeCommandBuffer(commandBuffer,
            m_swapChainCommands->frameBuffer(cmdBufferIndex), m_graphicsPipeline, clearValues, false);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}


void GraphicsVulkan::initializeCommandBufferDrawObjects(
    VkCommandBuffer commandBuffer,
    DrawObjectTable const &objs,
    std::shared_ptr<vulkan::Pipeline> const &pipeline,
    bool doShadows)
{
    VkDeviceSize offsets[1] = {0};

    for (auto const &obj : objs) {
        DrawObjectDataVulkan *objData = dynamic_cast<DrawObjectDataVulkan*> (obj.second.get());

        VkBuffer vertexBuffer = objData->vertexBuffer().cbuffer();
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, objData->indexBuffer().cbuffer(), 0, VK_INDEX_TYPE_UINT32);
        for (auto const &uniform : objData->uniforms()) {
            /* The MVP matrix and texture samplers */
            VkDescriptorSet descriptorSet;
            if (doShadows) {
                descriptorSet = uniform->m_descriptorSetShadows->descriptorSet().get();
            } else {
                descriptorSet = uniform->m_descriptorSet->descriptorSet().get();
            }
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipeline->layout().get(), 0, 1, &descriptorSet, 0, nullptr);

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
    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphore.semaphore().get()/*, m_shadowsAvailableForRead.semaphore().get() */};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT /*, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT */};
    submitInfo.waitSemaphoreCount = 1;//2;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    /* use the command buffer that corresponds to the image we just acquired */
    submitInfo.commandBufferCount = 1;
    VkCommandBuffer commandBuffer = m_swapChainCommands->commandBuffer(imageIndex);
    submitInfo.pCommandBuffers = &commandBuffer;

    /* indicate which semaphore to signal when execution is done */
    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphore.semaphore().get()/*, m_shadowsAvailableForWrite.semaphore().get()*/};
    submitInfo.signalSemaphoreCount = 1;//2;
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

    m_depthImageView->image()->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED,
                                                     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, m_commandPool);
}

class DepthTextureDescriptorSetLayout : public vulkan::DescriptorSetLayout {
public:
    DepthTextureDescriptorSetLayout(std::shared_ptr<vulkan::Device> inDevice)
            : m_device(inDevice)
    {
        m_poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        m_poolSizes[0].descriptorCount = 1;
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
    static uint32_t constexpr m_numberOfDescriptorSetsInPool = 1024;

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

std::shared_ptr<vulkan::ImageView> GraphicsVulkan::runTextureComputation(
        std::vector<std::shared_ptr<DrawObjectDataVulkanDepthTexture>> const &drawObjsData,
        std::vector<uint32_t> const &indexArraySizes,
        std::shared_ptr<vulkan::ImageView> &depthView,
        uint32_t imageWidth,
        uint32_t imageHeight,
        VkClearColorValue const &clearColorValue,
        VkFormat colorImageFormat,
        std::string const &vertexShader,
        std::string const &fragmentShader,
        std::shared_ptr<vulkan::DescriptorPools> &dscPools)
{
    auto colorImage = vulkan::ImageView::createImageViewAndImage(
            m_device,
            imageWidth,
            imageHeight,
            colorImageFormat,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, // todo: remove sampled bit usage
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);

    std::vector<vulkan::RenderPass::ImageAttachmentInfo> infos;
    infos.emplace_back(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
            colorImage->image()->format(),
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    auto depthInfo = std::make_shared<vulkan::RenderPass::ImageAttachmentInfo>(
            VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
            depthView->image()->format(),
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    auto renderPass = vulkan::RenderPass::createDepthTextureRenderPass(m_device, infos, depthInfo);

    std::vector<std::shared_ptr<vulkan::ImageView>> attachments = {colorImage, depthView};
    auto frameBuffer = std::make_shared<vulkan::Framebuffer>(m_device, renderPass, attachments,
                                                             imageWidth, imageHeight);

    VkExtent2D extentFB{imageWidth, imageHeight};
    auto pipeline = std::make_shared<vulkan::Pipeline>(m_gameRequester, m_device, extentFB,
                                                       renderPass, dscPools, getBindingDescription(), getAttributeDescriptions(),
                                                       vertexShader, fragmentShader, m_graphicsPipeline);

    // start recording commands
    vulkan::CommandBuffer cmds{m_device, m_commandPool};
    cmds.begin();

    /* begin the render pass: drawing starts here*/
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass->renderPass().get();
    renderPassInfo.framebuffer = frameBuffer->framebuffer().get();
    /* size of the render area */
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {imageWidth, imageHeight};

    /* the color value to use when clearing the image with VK_ATTACHMENT_LOAD_OP_CLEAR,
     * using black with 0% opacity
     */
    std::array<VkClearValue, 2> clearValues = {};
    // matching what OpenGL is doing with the clear buffers.
    clearValues[0].color = clearColorValue;
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    /* begin recording commands - start by beginning the render pass.
     * none of these functions returns an error (they return void).  There will be no error
     * handling until recording is done.
     */
    vkCmdBeginRenderPass(cmds.commandBuffer().get(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmds.commandBuffer().get(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline().get());

    size_t i = 0;
    VkDeviceSize offsets[1] = {0};
    for (auto const &drawObjData : drawObjsData) {
        VkBuffer vertexBufferRaw = drawObjData->vertexBuffer().cbuffer();
        vkCmdBindVertexBuffers(cmds.commandBuffer().get(), 0, 1, &vertexBufferRaw, offsets);

        vkCmdBindIndexBuffer(cmds.commandBuffer().get(), drawObjData->indexBuffer().cbuffer(), 0,
                             VK_INDEX_TYPE_UINT32);

        for (auto const &uniform : drawObjData->uniforms()) {
            VkDescriptorSet dscSet = uniform->descriptorSet()->descriptorSet().get();
            vkCmdBindDescriptorSets(cmds.commandBuffer().get(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipeline->layout().get(), 0, 1, &dscSet, 0, nullptr);
            vkCmdDrawIndexed(cmds.commandBuffer().get(), indexArraySizes[i], 1, 0, 0, 0);
        }
        i++;
    }
    vkCmdEndRenderPass(cmds.commandBuffer().get());

    cmds.end();

    return colorImage;
}

std::shared_ptr<TextureData> GraphicsVulkan::getDepthTexture(
        DrawObjectTable const &objsData,
        float width,
        float height,
        uint32_t nbrSamplesForWidth,
        float farthestDepth,
        float nearestDepth,
        std::vector<float> &depthMap,
        std::vector<glm::vec3> &normalMap)
{
    auto dscLayout = std::make_shared<DepthTextureDescriptorSetLayout>(m_device);
    auto dscPools = std::make_shared<vulkan::DescriptorPools>(m_device, dscLayout);

    glm::mat4 proj = getOrthoMatrix(-width/2.0f, width/2.0f, -height/2.0f, height/2.0f,
                                    m_depthTextureNearPlane, m_depthTextureFarPlane, true, true);
    glm::mat4 view = m_levelSequence->viewMatrix();
    glm::mat4 vp = proj * view;

    std::vector<std::shared_ptr<DrawObjectDataVulkanDepthTexture>> drawObjsData;
    for (auto const &objdata : objsData) {
        auto drawObjData = std::make_shared<DrawObjectDataVulkanDepthTexture>(
                m_device, m_commandPool, dscPools, objdata.first);
        for (auto const &modelMatrix : objdata.first->modelMatrices) {
            drawObjData->addUniforms(objdata.first, vp, farthestDepth, nearestDepth);
        }
        drawObjsData.push_back(drawObjData);
    }

    uint32_t imageWidth = nbrSamplesForWidth;
    uint32_t imageHeight = static_cast<uint32_t>(std::floor((imageWidth * height)/width));
    auto depthView = std::make_shared<vulkan::ImageView>(
            vulkan::ImageFactory::createDepthImage(m_device, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    imageWidth, imageHeight),
            VK_IMAGE_ASPECT_DEPTH_BIT);
    depthView->image()->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED,
                                              VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, m_commandPool);

    std::vector<uint32_t> indexArraySizes;
    for (auto const &objData : objsData) {
        indexArraySizes.push_back(objData.first->indices.size());
    }

    std::shared_ptr<vulkan::ImageView> colorDepthImage = runTextureComputation(
            drawObjsData,
            indexArraySizes,
            depthView,
            imageWidth,
            imageHeight,
            VkClearColorValue{0.0f, 0.0f, 0.0f, 1.0f},
            VK_FORMAT_R32_SFLOAT,
            SHADER_LINEAR_DEPTH_VERT_FILE,
            SHADER_SIMPLE_FRAG_FILE,
            dscPools);


    std::vector<float> colorDepthMap;
    colorDepthMap.resize(imageWidth * imageHeight);

    // use buffer for both the color depth image (R32) and the color normal image (R32G32B32)
    vulkan::Buffer buffer{m_device,
                          imageWidth * imageHeight * sizeof (float) * 4,
                          VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
    colorDepthImage->image()->copyImageToBuffer(buffer, m_commandPool);
    buffer.copyRawFrom(colorDepthMap.data(), colorDepthMap.size() * sizeof (float));
    bitmapToDepthMap(colorDepthMap, farthestDepth, nearestDepth, imageWidth, imageHeight, 1, true, depthMap);

    std::shared_ptr<vulkan::ImageView> colorNormalImage = runTextureComputation(
            drawObjsData,
            indexArraySizes,
            depthView,
            imageWidth,
            imageHeight,
            VkClearColorValue{0.5f, 0.5f, 1.0f, 1.0f},
            VK_FORMAT_R32G32B32A32_SFLOAT,
            SHADER_NORMAL_VERT_FILE,
            SHADER_SIMPLE_FRAG_FILE,
            dscPools);

    colorDepthMap.resize(imageWidth * imageHeight * 4);
    colorNormalImage->image()->copyImageToBuffer(buffer, m_commandPool);
    buffer.copyRawFrom(colorDepthMap.data(), colorDepthMap.size() * sizeof (float));
    bitmapToNormals(colorDepthMap, imageWidth, imageHeight, 4, true, normalMap);

    colorNormalImage->image()->transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                     m_commandPool);

    auto imgSampler = std::make_shared<vulkan::ImageSampler>(m_device, colorNormalImage);

    return std::make_shared<TextureDataVulkan>(imgSampler);
}
