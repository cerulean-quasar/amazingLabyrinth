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
#include "levelTracker.hpp"
#include "levelStarter.hpp"

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
    GraphicsVulkan(WindowType *window, uint32_t level)
        :instance{VK_NULL_HANDLE},
        callback{VK_NULL_HANDLE},
        physicalDevice{VK_NULL_HANDLE},
        graphicsQueue{VK_NULL_HANDLE},
        presentQueue{VK_NULL_HANDLE},
        surface{VK_NULL_HANDLE},
        swapChain{VK_NULL_HANDLE},
        descriptorPools{},
        texturesLevel{},
        texturesLevelStarter{},
        texturesLevelFinisher{},
        texturesChanged{false},
        swapChainImages{},
        swapChainImageViews{},
        swapChainImageFormat{},
        swapChainExtent{},
        renderPass{VK_NULL_HANDLE},
        uniformBufferLighting{VK_NULL_HANDLE},
        uniformBufferMemoryLighting{VK_NULL_HANDLE},
        staticObjsData{},
        dynObjsData{},
        levelFinisherObjsData{},
        levelStarterStaticObjsData{},
        levelStarterDynObjsData{},
        maze{},
        levelFinisher{},
        levelStarter{},
        levelTracker{level},
        pipelineLayout{},
        graphicsPipeline{},
        swapChainFramebuffers{},
        commandPool{},
        commandBuffers{},
        imageAvailableSemaphore{},
        renderFinishedSemaphore{},
        depthImage{VK_NULL_HANDLE},
        depthImageMemory{VK_NULL_HANDLE},
        depthImageView{VK_NULL_HANDLE},
        window{nullptr}
    {}
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
    VkInstance instance;
    VkDebugReportCallbackEXT callback;
    VkPhysicalDevice physicalDevice;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;

    class DescriptorPools;
    class DescriptorPool {
        friend DescriptorPools;
    private:
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        uint32_t totalDescriptorsAllocated;
        uint32_t totalDescriptorsInPool;

        DescriptorPool(VkDescriptorPool pool, uint32_t totalDescriptors)
                : descriptorPool(pool), totalDescriptorsInPool(totalDescriptors),
                  totalDescriptorsAllocated(0)
        { }
        VkDescriptorSet allocateDescriptor(VkDescriptorSetLayout layout) {
            if (totalDescriptorsAllocated == totalDescriptorsInPool) {
                return VK_NULL_HANDLE;
            } else {
                VkDescriptorSet descriptorSet;
                VkDescriptorSetLayout layouts[] = {layout};
                VkDescriptorSetAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = descriptorPool;
                allocInfo.descriptorSetCount = 1;
                allocInfo.pSetLayouts = layouts;

                /* the descriptor sets don't need to be freed because they are freed when the
                 * descriptor pool is freed
                 */
                int rc = vkAllocateDescriptorSets(logicalDevice, &allocInfo, &descriptorSet);
                if (rc != VK_SUCCESS) {
                    throw std::runtime_error("failed to allocate descriptor set!");
                }
                totalDescriptorsAllocated++;
                return descriptorSet;
            }
        }

        bool hasAvailableDescriptorSets() { return totalDescriptorsAllocated < totalDescriptorsInPool; }

    };

    /* for passing data other than the vertex data to the vertex shader */
    class DescriptorPools {
    private:
        uint32_t const numberOfDescriptorSetsInPool = 1024;
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        std::vector<DescriptorPool> descriptorPools;
        std::vector<VkDescriptorSet> unusedDescriptors;
    public:
        DescriptorPools() : descriptorSetLayout(VK_NULL_HANDLE) { }
        ~DescriptorPools() { destroyResources(); }

        void setDescriptorSetLayout(VkDescriptorSetLayout layout) {
            descriptorSetLayout = layout;
        }
        VkDescriptorSetLayout getDescriptorSetLayout() { return descriptorSetLayout; }
        void destroyResources() {
            unusedDescriptors.clear();

            for (auto &&descriptorPool: descriptorPools) {
                vkDestroyDescriptorPool(logicalDevice, descriptorPool.descriptorPool, nullptr);
            }

            descriptorPools.clear();

            if (descriptorSetLayout != VK_NULL_HANDLE) {
                vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, nullptr);
                descriptorSetLayout = VK_NULL_HANDLE;
            }

        }
        void freeDescriptor(VkDescriptorSet descriptorSet) {
            unusedDescriptors.push_back(descriptorSet);
        }

        VkDescriptorSet allocateDescriptor() {
            if (descriptorSetLayout == VK_NULL_HANDLE) {
                throw (std::runtime_error("DescriptorPool::allocateDescriptor - no descriptor set layout"));
            }

            if (unusedDescriptors.size() > 0) {
                VkDescriptorSet descriptorSet = unusedDescriptors.back();
                unusedDescriptors.pop_back();
                return descriptorSet;
            } else {
                for (auto &&descriptorPool : descriptorPools) {
                    if (descriptorPool.hasAvailableDescriptorSets()) {
                        return descriptorPool.allocateDescriptor(descriptorSetLayout);
                    }
                }

                // no more descriptors in all the descriptor pools.  create another descriptor pool...
                std::array<VkDescriptorPoolSize, 3> poolSizes = {};

                // one for each wall and +3 for the floor, ball, and hole.
                poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                poolSizes[0].descriptorCount = numberOfDescriptorSetsInPool;
                poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                poolSizes[1].descriptorCount = numberOfDescriptorSetsInPool;
                poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                poolSizes[2].descriptorCount = numberOfDescriptorSetsInPool;
                VkDescriptorPoolCreateInfo poolInfo = {};
                poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                poolInfo.pPoolSizes = poolSizes.data();
                poolInfo.maxSets = numberOfDescriptorSetsInPool;

                VkDescriptorPool descriptorPool;
                if (vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create descriptor pool!");
                }

                DescriptorPool newDescriptorPool(descriptorPool, numberOfDescriptorSetsInPool);

                descriptorPools.push_back(newDescriptorPool);
                return newDescriptorPool.allocateDescriptor(descriptorSetLayout);
            }
        }
    };
    DescriptorPools descriptorPools;

    struct TextureDataVulkan : public TextureData {
        VkImage image;
        VkDeviceMemory memory;
        VkImageView imageView;
        VkSampler sampler;

        virtual ~TextureDataVulkan() {
            vkDestroySampler(logicalDevice, sampler, nullptr);
            vkDestroyImageView(logicalDevice, imageView, nullptr);
            vkDestroyImage(logicalDevice, image, nullptr);
            vkFreeMemory(logicalDevice, memory, nullptr);
        }
    };
    TextureMap texturesLevel;
    TextureMap texturesLevelStarter;
    TextureMap texturesLevelFinisher;
    bool texturesChanged;

    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    VkRenderPass renderPass;

    VkBuffer uniformBufferLighting;
    VkDeviceMemory uniformBufferMemoryLighting;

    struct UniformWrapper {
        /* for passing data other than the vertex data to the vertex shader */
        VkDescriptorSet descriptorSet;
        VkBuffer uniformBuffer;
        VkDeviceMemory uniformBufferMemory;
        DescriptorPools *pools;

        UniformWrapper(VkDescriptorSet inDescriptorSet, VkBuffer inUniformBuffer,
                       VkDeviceMemory inUniformBufferMemory, DescriptorPools *inPools) {
            uniformBuffer = inUniformBuffer;
            uniformBufferMemory = inUniformBufferMemory;
            descriptorSet = inDescriptorSet;
            pools = inPools;
        }

        ~UniformWrapper() {
            pools->freeDescriptor(descriptorSet);
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
    struct DrawObjectDataVulkan : public DrawObjectData {
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
        std::vector<std::shared_ptr<UniformWrapper> > uniforms;

        ~DrawObjectDataVulkan() {
            vkDestroyBuffer(logicalDevice, vertexBuffer, nullptr);
            vkFreeMemory(logicalDevice, vertexBufferMemory, nullptr);

            vkDestroyBuffer(logicalDevice, indexBuffer, nullptr);
            vkFreeMemory(logicalDevice, indexBufferMemory, nullptr);
        }
    };

    DrawObjectTable staticObjsData;
    DrawObjectTable dynObjsData;
    DrawObjectTable levelFinisherObjsData;
    DrawObjectTable levelStarterStaticObjsData;
    DrawObjectTable levelStarterDynObjsData;

    std::shared_ptr<Level> maze;
    std::shared_ptr<LevelFinish> levelFinisher;
    std::shared_ptr<LevelStarter> levelStarter;
    LevelTracker levelTracker;

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
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
    WindowType *window;

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

    void addTextures(TextureMap &texture);
    UniformBufferObject getViewPerspectiveMatrix();
    void addObjects(DrawObjectTable &objs, TextureMap &texture);
    void addObject(DrawObjectEntry &obj, TextureMap &texture);
    void addUniforms(DrawObjectEntry &obj, TextureMap &texture);
    bool updateLevelData(Level *level, DrawObjectTable &objsData, TextureMap &textures);
    void initializeLevelData(Level *level, DrawObjectTable &staticObjsData,
                             DrawObjectTable &dynObjsData, TextureMap &textures);
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
    void initializeCommandBufferDrawObjects(VkCommandBuffer &commandBuffer, DrawObjectTable const & objs);
    void createSemaphores();
    void createDepthResources();
    VkFormat findDepthFormat();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    bool hasStencilComponent(VkFormat format);
    void updateDescriptorSet(VkBuffer uniformBuffer, VkImageView imageView, VkSampler textureSampler, VkBuffer lightingSource, VkDescriptorSet &descriptorSet);
    void createDescriptorSetLayout(VkDescriptorSetLayout &descriptorSetLayout);
    void createTextureImage(TextureDescription *texture, VkImage &textureImage, VkDeviceMemory &textureImageMemory);
    void createTextureSampler(VkSampler &textureSampler);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
};
#endif
