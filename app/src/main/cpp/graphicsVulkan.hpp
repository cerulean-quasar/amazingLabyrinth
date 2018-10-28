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

namespace vulkan {
    const std::vector<const char *> validationLayers = {
            /* required for checking for errors and getting error messages */
            //"VK_LAYER_LUNARG_standard_validation"
            "VK_LAYER_GOOGLE_threading",
            "VK_LAYER_LUNARG_parameter_validation",
            "VK_LAYER_LUNARG_object_tracker",
            "VK_LAYER_LUNARG_core_validation",
            "VK_LAYER_GOOGLE_unique_objects"
    };

    class VulkanLibrary {
    public:
        VulkanLibrary() {
            if (!loadVulkan()) {
                throw std::runtime_error("Could not find vulkan library.");
            }
        }
    };

    class Instance {
    public:
        Instance(WindowType *inWindow)
                : m_loader{},
                  m_window{},
                  m_instance{},
                  m_callback{},
                  m_surface{} {
            auto deleter = [](WindowType *windowRaw) {
                /* release the java window object */
                if (windowRaw != nullptr) {
                    ANativeWindow_release(windowRaw);
                }
            };

            m_window.reset(inWindow, deleter);

            createInstance();
            setupDebugCallback();
            createSurface();
        }

        inline std::shared_ptr<VkInstance_T> const &instance() { return m_instance; }

        inline std::shared_ptr<VkSurfaceKHR_T> const &surface() { return m_surface; }

        inline std::shared_ptr<VkDebugReportCallbackEXT_T> const &callback() { return m_callback; }

    private:
        VulkanLibrary m_loader;
        std::shared_ptr<WindowType> m_window;
        std::shared_ptr<VkInstance_T> m_instance;
        std::shared_ptr<VkDebugReportCallbackEXT_T> m_callback;
        std::shared_ptr<VkSurfaceKHR_T> m_surface;

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
                VkDebugReportFlagsEXT flags,
                VkDebugReportObjectTypeEXT objType,
                uint64_t obj,
                size_t location,
                int32_t code,
                const char *layerPrefix,
                const char *msg,
                void *userData) {

            std::cerr << "validation layer: " << msg << std::endl;

            return VK_FALSE;
        }

        void createSurface();

        void setupDebugCallback();

        void createInstance();

        bool checkExtensionSupport();

        bool checkValidationLayerSupport();

        std::vector<const char *> getRequiredExtensions();

        VkResult createDebugReportCallbackEXT(
                const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                const VkAllocationCallbacks *pAllocator,
                VkDebugReportCallbackEXT *pCallback);
        static void destroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                  const VkAllocationCallbacks *pAllocator);
    };

    class Device {
    public:
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

        Device(std::shared_ptr<Instance> const &inInstance)
                : m_instance (inInstance),
                  m_physicalDevice{},
                  m_logicalDevice{},
                  m_graphicsQueue{},
                  m_presentQueue{},
                  m_depthFormat{} {
            pickPhysicalDevice();
            createLogicalDevice();
            m_depthFormat = findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                                                 VK_FORMAT_D24_UNORM_S8_UINT},
                                                VK_IMAGE_TILING_OPTIMAL,
                                                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        }

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        inline QueueFamilyIndices findQueueFamilies() {
            return findQueueFamilies(m_physicalDevice);
        }

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
        inline SwapChainSupportDetails querySwapChainSupport() {
            return querySwapChainSupport(m_physicalDevice);
        }

        VkFormat depthFormat() { return m_depthFormat; }

        inline std::shared_ptr<VkDevice_T> const &logicalDevice() { return m_logicalDevice; }

        inline VkPhysicalDevice physicalDevice() { return m_physicalDevice; }

        inline VkQueue graphicsQueue() { return m_graphicsQueue; }

        inline VkQueue presentQueue() { return m_presentQueue; }

        inline std::shared_ptr<Instance> const &instance() { return m_instance; }

    private:
        /* ensure that the instance is not destroyed before the device by holding a shared
         * pointer to the instance here.
         */
        std::shared_ptr<Instance> m_instance;

        // the physical device does not need to be freed.
        VkPhysicalDevice m_physicalDevice;

        std::shared_ptr<VkDevice_T> m_logicalDevice;

        // the graphics and present queues are really part of the logical device and don't need to be freed.
        VkQueue m_graphicsQueue;
        VkQueue m_presentQueue;

        VkFormat m_depthFormat;

        const std::vector<const char *> deviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates,
                                     VkImageTiling tiling, VkFormatFeatureFlags features);

        void pickPhysicalDevice();

        void createLogicalDevice();

        bool isDeviceSuitable(VkPhysicalDevice device);

        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    };

    class SwapChain {
    public:
        SwapChain(std::shared_ptr<Device> inDevice)
                : m_device(inDevice),
                  m_swapChain{VK_NULL_HANDLE},
                  m_imageFormat{},
                  m_extent{} {
            createSwapChain();
        }

        inline std::shared_ptr<Device> const &device() { return m_device; }
        inline std::shared_ptr<VkSwapchainKHR_T> const &swapChain() {return m_swapChain; }
        inline VkFormat imageFormat() { return m_imageFormat; }
        inline VkExtent2D extent() { return m_extent; }
    private:
        std::shared_ptr<Device> m_device;
        std::shared_ptr<VkSwapchainKHR_T> m_swapChain;
        VkFormat m_imageFormat;
        VkExtent2D m_extent;

        void createSwapChain();

        VkSurfaceFormatKHR
        chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

        VkPresentModeKHR
        chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    };

    class RenderPass {
    public:
        RenderPass(std::shared_ptr<Device> const &inDevice, std::shared_ptr<SwapChain> const &swapChain)
                : m_device{inDevice},
                  m_renderPass{} {
            createRenderPass(swapChain);
        }

        inline std::shared_ptr<VkRenderPass_T> const &renderPass() { return m_renderPass; }

    private:
        std::shared_ptr<Device> m_device;
        std::shared_ptr<VkRenderPass_T> m_renderPass;

        void createRenderPass(std::shared_ptr<SwapChain> const &swapChain);
    };
}

#define DEBUG
#ifdef DEBUG
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif

class GraphicsVulkan : public Graphics {
public:
    GraphicsVulkan(WindowType *window, uint32_t level)
        :m_instance{new vulkan::Instance(window)},
        m_device{new vulkan::Device{m_instance}},
        m_swapChain{new vulkan::SwapChain{m_device}},
        m_renderPass{new vulkan::RenderPass{m_device, m_swapChain}},
        descriptorPools{m_device},
        texturesLevel{},
        texturesLevelStarter{},
        texturesLevelFinisher{},
        texturesChanged{false},
        swapChainImages{},
        swapChainImageViews{},
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
        depthImageView{VK_NULL_HANDLE}
    {}
    virtual void init(WindowType *window);

    virtual void initThread() { }

    virtual void cleanupThread() { }

    virtual void cleanup();

    virtual bool updateData();

    virtual void updateAcceleration(float x, float y, float z);

    virtual void drawFrame();

    virtual void destroyWindow() {}

    virtual void recreateSwapChain();

    virtual ~GraphicsVulkan() { }
private:
    std::shared_ptr<vulkan::Instance> m_instance;
    std::shared_ptr<vulkan::Device> m_device;
    std::shared_ptr<vulkan::SwapChain> m_swapChain;

    std::shared_ptr<vulkan::RenderPass> m_renderPass;

    class DescriptorPools;
    class DescriptorPool {
        friend DescriptorPools;
    private:
        std::shared_ptr<vulkan::Device> m_device;
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        uint32_t totalDescriptorsAllocated;
        uint32_t totalDescriptorsInPool;

        DescriptorPool(std::shared_ptr<vulkan::Device> inDevice,
                       VkDescriptorPool pool, uint32_t totalDescriptors)
                : m_device(inDevice),
                  descriptorPool(pool), totalDescriptorsInPool(totalDescriptors),
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
                int rc = vkAllocateDescriptorSets(m_device->logicalDevice().get(), &allocInfo, &descriptorSet);
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
        std::shared_ptr<vulkan::Device> m_device;
        uint32_t const numberOfDescriptorSetsInPool = 1024;
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        std::vector<DescriptorPool> descriptorPools;
        std::vector<VkDescriptorSet> unusedDescriptors;
    public:
        DescriptorPools(std::shared_ptr<vulkan::Device> inDevice)
                : m_device{inDevice}, descriptorSetLayout(VK_NULL_HANDLE) { }
        ~DescriptorPools() { destroyResources(); }

        void setDescriptorSetLayout(VkDescriptorSetLayout layout) {
            descriptorSetLayout = layout;
        }
        VkDescriptorSetLayout getDescriptorSetLayout() { return descriptorSetLayout; }
        void destroyResources() {
            unusedDescriptors.clear();

            for (auto &&descriptorPool: descriptorPools) {
                vkDestroyDescriptorPool(m_device->logicalDevice().get(), descriptorPool.descriptorPool, nullptr);
            }

            descriptorPools.clear();

            if (descriptorSetLayout != VK_NULL_HANDLE) {
                vkDestroyDescriptorSetLayout(m_device->logicalDevice().get(), descriptorSetLayout, nullptr);
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
                if (vkCreateDescriptorPool(m_device->logicalDevice().get(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create descriptor pool!");
                }

                DescriptorPool newDescriptorPool(m_device, descriptorPool, numberOfDescriptorSetsInPool);

                descriptorPools.push_back(newDescriptorPool);
                return newDescriptorPool.allocateDescriptor(descriptorSetLayout);
            }
        }
    };
    DescriptorPools descriptorPools;

    struct TextureDataVulkan : public TextureData {
        TextureDataVulkan(std::shared_ptr<vulkan::Device> inDevice)
                : m_device{inDevice}
        {}
        std::shared_ptr<vulkan::Device> m_device;
        VkImage image;
        VkDeviceMemory memory;
        VkImageView imageView;
        VkSampler sampler;

        virtual ~TextureDataVulkan() {
            vkDestroySampler(m_device->logicalDevice().get(), sampler, nullptr);
            vkDestroyImageView(m_device->logicalDevice().get(), imageView, nullptr);
            vkDestroyImage(m_device->logicalDevice().get(), image, nullptr);
            vkFreeMemory(m_device->logicalDevice().get(), memory, nullptr);
        }
    };
    TextureMap texturesLevel;
    TextureMap texturesLevelStarter;
    TextureMap texturesLevelFinisher;
    bool texturesChanged;

    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    VkBuffer uniformBufferLighting;
    VkDeviceMemory uniformBufferMemoryLighting;

    struct UniformWrapper {
        std::shared_ptr<vulkan::Device> m_device;
        /* for passing data other than the vertex data to the vertex shader */
        VkDescriptorSet descriptorSet;
        VkBuffer uniformBuffer;
        VkDeviceMemory uniformBufferMemory;
        DescriptorPools *pools;

        UniformWrapper(std::shared_ptr<vulkan::Device> inDevice,
                       VkDescriptorSet inDescriptorSet, VkBuffer inUniformBuffer,
                       VkDeviceMemory inUniformBufferMemory, DescriptorPools *inPools)
        : m_device(inDevice) {
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
            vkDestroyBuffer(m_device->logicalDevice().get(), uniformBuffer, nullptr);
            vkFreeMemory(m_device->logicalDevice().get(), uniformBufferMemory, nullptr);
        }
    };

    /* vertex buffer and index buffer. the index buffer indicates which vertices to draw and in
     * the specified order.  Note, vertices can be listed twice if they should be part of more
     * than one triangle.
     */
    struct DrawObjectDataVulkan : public DrawObjectData {
        std::shared_ptr<vulkan::Device> m_device;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
        std::vector<std::shared_ptr<UniformWrapper> > uniforms;

        DrawObjectDataVulkan(std::shared_ptr<vulkan::Device> inDevice)
        : m_device{inDevice} {
        }
        ~DrawObjectDataVulkan() {
            vkDestroyBuffer(m_device->logicalDevice().get(), vertexBuffer, nullptr);
            vkFreeMemory(m_device->logicalDevice().get(), vertexBufferMemory, nullptr);

            vkDestroyBuffer(m_device->logicalDevice().get(), indexBuffer, nullptr);
            vkFreeMemory(m_device->logicalDevice().get(), indexBufferMemory, nullptr);
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
    void createImageViews();
    VkShaderModule createShaderModule(const std::vector<char>& code);
    void createRenderPass();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void createCommandPool();
    void createCommandBuffers();
    void initializeCommandBuffer(VkCommandBuffer &commandBuffer, size_t index);
    void initializeCommandBufferDrawObjects(VkCommandBuffer &commandBuffer, DrawObjectTable const & objs);
    void createSemaphores();
    void createDepthResources();
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

