/**
 * Copyright 2024 Cerulean Quasar. All Rights Reserved.
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
#include "levelDrawer/levelDrawerVulkan.hpp"
#include "renderLoader/renderLoaderVulkan.hpp"

class GraphicsVulkan : public Graphics {
public:
    GraphicsVulkan(std::shared_ptr<WindowType> window,
            bool shadowsEnabled,
            std::shared_ptr<GameRequester> inGameRequester,
            float rotationAngle)
            : Graphics{std::move(inGameRequester), rotationAngle},
              m_instance{new vulkan::Instance(std::move(window))},
              m_device{new vulkan::Device{m_instance}},
              m_swapChain{new vulkan::SwapChain{m_device}},
              m_surfaceDetails{std::make_shared<vulkan::SurfaceDetails>(
                      vulkan::SurfaceDetails{vulkan::RenderPass::createRenderPass(m_device, m_swapChain),
                      preTransform(), m_swapChain->extent().width, m_swapChain->extent().height})},
              m_commandPool{new vulkan::CommandPool{m_device}},
              m_depthImageView{new vulkan::ImageView{vulkan::ImageFactory::createDepthImage(m_swapChain),
                                                     VK_IMAGE_ASPECT_DEPTH_BIT}},
              m_swapChainCommands{new vulkan::SwapChainCommands{m_swapChain, m_commandPool, m_surfaceDetails->renderPass, m_depthImageView}},
              m_imageAvailableSemaphore{m_device},
              m_renderFinishedSemaphore{m_device},
              m_renderLoader{std::make_shared<RenderLoaderVulkan>(m_device)},
              m_levelDrawer{std::make_shared<levelDrawer::LevelDrawerVulkan>(levelDrawer::NeededForDrawingVulkan{m_device, m_commandPool},
                      m_surfaceDetails, m_renderLoader,
                      m_gameRequester)}
    {
        prepareDepthResources();

        if (!testDepthTexture(levelDrawer::Adaptor(levelDrawer::LEVEL, m_levelDrawer))) {
            throw std::runtime_error(
                    "This version of Vulkan has bugs making it impossible to get the depth texture and normal map.");
        }

        m_levelSequence = std::make_shared<LevelSequence>(
                m_gameRequester, m_levelDrawer,
                m_swapChain->extent().width, m_swapChain->extent().height,
                shadowsEnabled);
    }

    void initThread() override { }

    void cleanupThread() override { }

    bool updateData(bool alwaysUpdateDynObjs) override { return m_levelSequence->updateData(alwaysUpdateDynObjs); }

    void drawFrame() override;

    bool isVulkanImplementation() override { return true; }

    void recreateSwapChain(uint32_t width, uint32_t height) override ;

    GraphicsDescription graphicsDescription() override {
        auto devGraphicsDescription = m_device->properties();
        return GraphicsDescription{std::string{"Vulkan"},
                std::move(devGraphicsDescription.m_vulkanAPIVersion),
                std::move(devGraphicsDescription.m_name),
                std::vector<std::string>{}};
    }

    ~GraphicsVulkan() override {
        if (m_device) {
            vkDeviceWaitIdle(m_device->logicalDevice().get());
        }
    }
protected:
    glm::mat4 preTransform();

private:
    std::shared_ptr<vulkan::Instance> m_instance;
    std::shared_ptr<vulkan::Device> m_device;
    std::shared_ptr<vulkan::SwapChain> m_swapChain;
    std::shared_ptr<vulkan::SurfaceDetails> m_surfaceDetails;
    std::shared_ptr<vulkan::CommandPool> m_commandPool;
    std::shared_ptr<vulkan::ImageView> m_depthImageView;
    std::shared_ptr<vulkan::SwapChainCommands> m_swapChainCommands;

    /* use semaphores to coordinate the rendering and presentation. Could also use fences
     * but fences are more for coordinating in our program itself and not for internal
     * Vulkan coordination of resource usage.
     */
    vulkan::Semaphore m_imageAvailableSemaphore;
    vulkan::Semaphore m_renderFinishedSemaphore;

    std::shared_ptr<RenderLoaderVulkan> m_renderLoader;
    std::shared_ptr<levelDrawer::LevelDrawerVulkan> m_levelDrawer;

    void cleanupSwapChain();
    void initializeCommandBuffers();
    void initializeCommandBuffer(uint32_t cmdBufferIndex);
    void prepareDepthResources();
};

#endif // AMAZING_LABYRINTH_MAZE_VULKAN_HPP