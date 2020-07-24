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
#include <vector>
#include <memory>
#include <glm/glm.hpp>

#include "mazeVulkan.hpp"
#include "mathGraphics.hpp"
#include "renderDetails/renderDetailsVulkan.hpp"

glm::mat4 GraphicsVulkan::preTransform() {
    glm::mat4 preTransformRet{1.0f};

    switch (m_swapChain->preTransform()) {
        case VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR:
            preTransformRet = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            break;
        case VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR:
            preTransformRet = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            break;
        case VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR:
            preTransformRet = glm::rotate(glm::mat4(1.0f), glm::radians(270.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            break;
        case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR:
            preTransformRet[0][0] = -1.0f;
            break;
        case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR:
            preTransformRet[0][0] = -1.0f;
            preTransformRet = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * preTransformRet;
            break;
        case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR:
            preTransformRet[0][0] = -1.0f;
            preTransformRet = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * preTransformRet;
            break;
        case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR:
            preTransformRet[0][0] = -1.0f;
            preTransformRet = glm::rotate(glm::mat4(1.0f), glm::radians(270.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * preTransformRet;
            break;
        case VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR:
        default:
            break;
    }

    return preTransformRet;
}

void GraphicsVulkan::recreateSwapChain(uint32_t width, uint32_t height) {
    VkExtent2D extent = m_swapChain->extent();

    // android sends a surface change message right after starting with the same surface width
    // and height.  Avoid recreating everything in that case.
    if (width == extent.width && height == extent.height) {
        return;
    }

    vkDeviceWaitIdle(m_device->logicalDevice().get());

    cleanupSwapChain();

    m_swapChain = std::make_shared<vulkan::SwapChain>(m_device, width, height);
    m_depthImageView = std::make_shared<vulkan::ImageView>(vulkan::ImageFactory::createDepthImage(m_swapChain),
                                                 VK_IMAGE_ASPECT_DEPTH_BIT);
    prepareDepthResources();
    m_renderPass = vulkan::RenderPass::createRenderPass(m_device, m_swapChain);
    m_swapChainCommands = std::make_shared<vulkan::SwapChainCommands>(m_swapChain, m_commandPool, m_renderPass, m_depthImageView);

    extent = m_swapChain->extent();
    m_levelSequence->notifySurfaceChanged(extent.width, extent.height);
}

void GraphicsVulkan::cleanupSwapChain() {
    if (m_device->logicalDevice().get() == VK_NULL_HANDLE) {
        return;
    }

    m_levelSequence->cleanupLevelData();

    m_swapChainCommands.reset();

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

void GraphicsVulkan::initializeCommandBuffer(uint32_t cmdBufferIndex) {
    VkCommandBuffer commandBuffer = m_swapChainCommands->commandBuffer(cmdBufferIndex);
    VkFramebuffer framebuffer = m_swapChainCommands->frameBuffer(cmdBufferIndex);

    levelDrawer::DrawArgumentVulkan info;
    info.cmdBuffer = commandBuffer;
    info.framebuffer = framebuffer;
    info.renderPass = m_renderPass;
    info.extent = m_swapChain->extent();

    m_levelDrawer->draw(info);
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

std::shared_ptr<renderDetails::Parameters> GraphicsVulkan::getParametersForRenderDetailsName(
        char const */* unused renderDetailsName - reserved for future use */)
{
    auto extent = m_swapChain->extent();
    renderDetails::ParametersVulkan parameters;
    parameters.width = extent.width;
    parameters.height = extent.height;
    parameters.preTransform = preTransform();
    parameters.renderPass = m_renderPass;

    return std::make_shared<renderDetails::ParametersVulkan>(parameters);
}