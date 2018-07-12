/**
 * Copyright 2018 Cerulean Quasar. All Rights Reserved.
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
#ifndef AMAZING_LABYRINTH_GRAPHICS_VULKAN_HPP
#define AMAZING_LABYRINTH_GRAPHICS_VULKAN_HPP
#include "vulkanWrapper.hpp"

#include <stdexcept>
#include <functional>
#include <vector>
#include <string>
#include <string.h>
#include <set>
#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <map>

#include "graphics.hpp"
#include "maze.hpp"
#include "levelFinish.hpp"

const std::vector<const char*> validationLayers = {
    /* required for checking for errors and getting error messages */
    //"VK_LAYER_LUNARG_standard_validation"
    "VK_LAYER_GOOGLE_threading",
    "VK_LAYER_LUNARG_parameter_validation",
    "VK_LAYER_LUNARG_object_tracker",
    "VK_LAYER_LUNARG_core_validation",
    "VK_LAYER_GOOGLE_unique_objects"
};

#define DEBUG
#ifdef DEBUG
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif


class GraphicsVulkan : public Graphics {
public:
    GraphicsVulkan(WindowType *window) {}
    virtual void init(WindowType *window);

    virtual void initThread() { }

    virtual void cleanupThread() { }

    virtual void cleanup();

    virtual bool updateData();

    virtual void updateAcceleration(float x, float y, float z);

    virtual void drawFrame();

    virtual void destroyWindow();

    virtual void recreateSwapChain();

    virtual ~GraphicsVulkan() { }
private:
    struct QueueFamilyIndices {
        int graphicsFamily = -1;
        int presentFamily = -1;

        bool isComplete() {
            return graphicsFamily >= 0 && presentFamily >= 0;
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    static VkDevice logicalDevice;
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT callback = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;

    struct TextureObject {
        VkImage image;
        VkDeviceMemory memory;
        VkImageView imageView;
        VkSampler sampler;

        ~TextureObject() {
            vkDestroySampler(logicalDevice, sampler, nullptr);
            vkDestroyImageView(logicalDevice, imageView, nullptr);
            vkDestroyImage(logicalDevice, image, nullptr);
            vkFreeMemory(logicalDevice, memory, nullptr);
        }
    };
    std::map<std::string, std::shared_ptr<TextureObject> > textures;
    typedef std::map<std::string, std::shared_ptr<TextureObject> >::iterator TextureIterator;

    /* for passing data other than the vertex data to the vertex shader */
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    VkRenderPass renderPass = VK_NULL_HANDLE;

    VkBuffer uniformBufferLighting;
    VkDeviceMemory uniformBufferMemoryLighting;

    struct UniformWrapper {
        /* for passing data other than the vertex data to the vertex shader */
        VkDescriptorSet descriptorSet;
        VkBuffer uniformBuffer;
        VkDeviceMemory uniformBufferMemory;

        UniformWrapper(VkDescriptorSet inDescriptorSet, VkBuffer inUniformBuffer, VkDeviceMemory inUniformBufferMemory) {
            uniformBuffer = inUniformBuffer;
            uniformBufferMemory = inUniformBufferMemory;
            descriptorSet = inDescriptorSet;
        }

        ~UniformWrapper() {
            /* free the memory after the buffer has been destroyed because the buffer is bound to 
             * the memory, so the buffer is still using the memory until the buffer is destroyed.
             */
            vkDestroyBuffer(logicalDevice, uniformBuffer, nullptr);
            vkFreeMemory(logicalDevice, uniformBufferMemory, nullptr);
        }
    };

    /* vertex buffer and index buffer. the index buffer indicates which vertices to draw and in
     * the specified order.  Note, vertices can be listed twice if they should be part of more
     * than one triangle.
     */
    struct DrawObjectData {
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
        uint32_t numberIndices;
        std::vector<std::shared_ptr<UniformWrapper> > uniforms;

        ~DrawObjectData() {
            vkDestroyBuffer(logicalDevice, vertexBuffer, nullptr);
            vkFreeMemory(logicalDevice, vertexBufferMemory, nullptr);

            vkDestroyBuffer(logicalDevice, indexBuffer, nullptr);
            vkFreeMemory(logicalDevice, indexBufferMemory, nullptr);
        }
    };

    std::vector<std::shared_ptr<DrawObjectData> > staticObjsData;
    std::vector<std::shared_ptr<DrawObjectData> > dynObjsData;
    std::vector<std::shared_ptr<DrawObjectData> > levelFinisherObjsData;

    std::shared_ptr<Maze> maze;
    std::shared_ptr<LevelFinish> levelFinisher;

    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    /* use semaphores to coordinate the rendering and presentation. Could also use fences
     * but fences are more for coordinating in our program itself and not for internal
     * Vulkan coordination of resource usage.
     */
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;

    /* depth buffer image */
    VkImage depthImage = VK_NULL_HANDLE;
    VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
    VkImageView depthImageView = VK_NULL_HANDLE;
    WindowType *window = nullptr;

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugReportFlagsEXT flags,
            VkDebugReportObjectTypeEXT objType,
            uint64_t obj,
            size_t location,
            int32_t code,
            const char* layerPrefix,
            const char* msg,
            void* userData) {

        std::cerr << "validation layer: " << msg << std::endl;

        return VK_FALSE;
    }

    void createSurface();

    void addTextures(std::vector<DrawObject> const &objs);
    void addTexture(DrawObject const &obj);
    void addObjects(std::vector<DrawObject> const &objs, std::vector<std::shared_ptr<DrawObjectData> > &objsData);
    void addObject(DrawObject const &obj, std::vector<std::shared_ptr<DrawObjectData> > &objsData);
    void addUniforms(DrawObject const &obj, DrawObjectData &objData);
    void createUniformBuffer(VkBuffer &uniformBuffer, VkDeviceMemory &uniformBufferMemory);
    void createVertexBuffer(std::vector<Vertex> const &vertices, VkBuffer &vertexBuffer, VkDeviceMemory &vertexBufferMemory);
    void createIndexBuffer(std::vector<uint32_t> const &indices, VkBuffer &indexBuffer, VkDeviceMemory &indexBufferMemory);
    void updatePerspectiveMatrix();

    void cleanupSwapChain();
    std::vector<const char *> getRequiredExtensions();
    void createInstance();
    void setupDebugCallback();
    bool checkExtensionSupport();
    bool checkValidationLayerSupport();
    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    void createLogicalDevice();
    void createSwapChain();
    void createImageViews();
    VkShaderModule createShaderModule(const std::vector<char>& code);
    void createRenderPass();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createCommandPool();
    void createCommandBuffers();
    void initializeCommandBuffer(VkCommandBuffer &commandBuffer, size_t index);
    void initializeCommandBufferDrawObjects(VkCommandBuffer &commandBuffer, std::vector<std::shared_ptr<DrawObjectData> > objs);
    void createSemaphores();
    void createDepthResources();
    VkFormat findDepthFormat();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    bool hasStencilComponent(VkFormat format);
    void createDescriptorSet(VkBuffer uniformBuffer, VkImageView imageView, VkSampler textureSampler, VkBuffer lightingSource, VkDescriptorSet &descriptorSet);
    void createDescriptorPool(uint32_t nbrWalls);
    void createDescriptorSetLayout();
    void createTextureImage(std::string const &path, VkImage &textureImage, VkDeviceMemory &textureImageMemory);
    void createTextureSampler(VkSampler &textureSampler);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
};
#endif
