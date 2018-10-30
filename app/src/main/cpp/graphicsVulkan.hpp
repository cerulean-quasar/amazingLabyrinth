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

    class DescriptorPools;

    class DescriptorSet {
        friend DescriptorPools;
    private:
        std::shared_ptr<DescriptorPools> m_descriptorPools;
        std::shared_ptr<VkDescriptorSet_T> m_descriptorSet;

        DescriptorSet(std::shared_ptr<DescriptorPools> inDescriptorPools,
                      std::shared_ptr<VkDescriptorSet_T> const &inDsc)
                : m_descriptorPools{inDescriptorPools},
                  m_descriptorSet{inDsc} {
        }

    public:
        inline std::shared_ptr<VkDescriptorSet_T> const &descriptorSet() { return m_descriptorSet; }
    };

    class DescriptorPool {
        friend DescriptorPools;
    private:
        std::shared_ptr<Device> m_device;
        std::shared_ptr<VkDescriptorPool_T> m_descriptorPool;
        uint32_t m_totalDescriptorsInPool;
        uint32_t m_totalDescriptorsAllocated;

        DescriptorPool(std::shared_ptr<Device> const &inDevice, uint32_t totalDescriptors)
                : m_device{inDevice},
                  m_descriptorPool{},
                  m_totalDescriptorsInPool{totalDescriptors},
                  m_totalDescriptorsAllocated{0} {
            // no more descriptors in all the descriptor pools.  create another descriptor pool...
            std::array<VkDescriptorPoolSize, 3> poolSizes = {};

            // one for each wall and +3 for the floor, ball, and hole.
            poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes[0].descriptorCount = m_totalDescriptorsInPool;
            poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes[1].descriptorCount = m_totalDescriptorsInPool;
            poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes[2].descriptorCount = m_totalDescriptorsInPool;
            VkDescriptorPoolCreateInfo poolInfo = {};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = m_totalDescriptorsInPool;

            VkDescriptorPool descriptorPoolRaw;
            if (vkCreateDescriptorPool(m_device->logicalDevice().get(), &poolInfo, nullptr,
                                       &descriptorPoolRaw) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor pool!");
            }

            auto deleter = [inDevice](VkDescriptorPool descriptorPoolRaw) {
                vkDestroyDescriptorPool(inDevice->logicalDevice().get(), descriptorPoolRaw, nullptr);
            };

            m_descriptorPool.reset(descriptorPoolRaw, deleter);
        }

        VkDescriptorSet allocateDescriptor(std::shared_ptr<VkDescriptorSetLayout_T> const &layout) {
            if (m_totalDescriptorsAllocated == m_totalDescriptorsInPool) {
                return VK_NULL_HANDLE;
            } else {
                VkDescriptorSet descriptorSet;
                VkDescriptorSetLayout layouts[] = {layout.get()};
                VkDescriptorSetAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = m_descriptorPool.get();
                allocInfo.descriptorSetCount = 1;
                allocInfo.pSetLayouts = layouts;

                /* the descriptor sets don't need to be freed because they are freed when the
                 * descriptor pool is freed
                 */
                int rc = vkAllocateDescriptorSets(m_device->logicalDevice().get(), &allocInfo,
                                                  &descriptorSet);
                if (rc != VK_SUCCESS) {
                    throw std::runtime_error("failed to allocate descriptor set!");
                }
                m_totalDescriptorsAllocated++;
                return descriptorSet;
            }
        }

        bool hasAvailableDescriptorSets() {
            return m_totalDescriptorsAllocated < m_totalDescriptorsInPool;
        }
    };

/* for passing data other than the vertex data to the vertex shader */
    class DescriptorPools : public std::enable_shared_from_this<DescriptorPools> {
        friend DescriptorSet;
    private:
        std::shared_ptr<Device> m_device;
        static uint32_t constexpr m_numberOfDescriptorSetsInPool = 1024;
        std::shared_ptr<VkDescriptorSetLayout_T> m_descriptorSetLayout;
        std::vector<DescriptorPool> m_descriptorPools;
        std::vector<VkDescriptorSet> m_unusedDescriptors;

        void createDescriptorSetLayout();

    public:
        DescriptorPools(std::shared_ptr<Device> const &inDevice)
                : m_device{inDevice},
                  m_descriptorSetLayout{},
                  m_descriptorPools{},
                  m_unusedDescriptors{} {
            createDescriptorSetLayout();
        }

        std::shared_ptr<VkDescriptorSetLayout_T> const &descriptorSetLayout() { return m_descriptorSetLayout; }

        std::shared_ptr<DescriptorSet> allocateDescriptor() {
            if (m_descriptorSetLayout == VK_NULL_HANDLE) {
                throw (std::runtime_error(
                        "DescriptorPool::allocateDescriptor - no descriptor set layout"));
            }

            auto deleter = [this](VkDescriptorSet descSet) {
                m_unusedDescriptors.push_back(descSet);
            };

            if (m_unusedDescriptors.size() > 0) {
                VkDescriptorSet descriptorSet = m_unusedDescriptors.back();
                m_unusedDescriptors.pop_back();

                return std::shared_ptr<DescriptorSet>{new DescriptorSet{shared_from_this(),
                                                                        std::shared_ptr<VkDescriptorSet_T>{descriptorSet, deleter}}};
            } else {
                for (auto &&descriptorPool : m_descriptorPools) {
                    if (descriptorPool.hasAvailableDescriptorSets()) {
                        VkDescriptorSet descriptorSet = descriptorPool.allocateDescriptor(
                                m_descriptorSetLayout);
                        return std::shared_ptr<DescriptorSet>{new DescriptorSet{shared_from_this(),
                                                                                std::shared_ptr<VkDescriptorSet_T>{descriptorSet, deleter}}};
                    }
                }


                DescriptorPool newDescriptorPool(m_device, m_numberOfDescriptorSetsInPool);
                m_descriptorPools.push_back(newDescriptorPool);
                VkDescriptorSet descriptorSet = newDescriptorPool.allocateDescriptor(
                        m_descriptorSetLayout);
                return std::shared_ptr<DescriptorSet>(new DescriptorSet(shared_from_this(),
                                                                        std::shared_ptr<VkDescriptorSet_T>(descriptorSet, deleter)));
            }
        }
    };

    class Shader {
        std::shared_ptr<Device> m_device;

        std::shared_ptr<VkShaderModule_T> m_shaderModule;

        void createShaderModule(std::string const &codeFile);

    public:
        Shader(std::shared_ptr<Device> const &inDevice, std::string const &codeFile)
                : m_device(inDevice),
                  m_shaderModule{} {
            createShaderModule(codeFile);
        }

        inline std::shared_ptr<VkShaderModule_T> const &shader() { return m_shaderModule; }
    };

    class Pipeline {
    public:
        Pipeline(std::shared_ptr<SwapChain> &inSwapChain, std::shared_ptr<RenderPass> &inRenderPass,
                 std::shared_ptr<DescriptorPools> &inDescriptorPools,
                 VkVertexInputBindingDescription const &bindingDescription,
                 std::vector<VkVertexInputAttributeDescription> const &attributeDescription)
                : m_device{inSwapChain->device()},
                  m_swapChain{inSwapChain},
                  m_renderPass{inRenderPass},
                  m_descriptorPools{inDescriptorPools},
                  m_pipelineLayout{},
                  m_pipeline{} {
            createGraphicsPipeline(bindingDescription, attributeDescription);
        }

        inline std::shared_ptr<VkPipeline_T> const &pipeline() { return m_pipeline; }
        inline std::shared_ptr<VkPipelineLayout_T> const &layout() { return m_pipelineLayout; }

        ~Pipeline() {
            m_pipeline.reset();
            m_pipelineLayout.reset();
        }

    private:
        std::shared_ptr<Device> m_device;
        std::shared_ptr<SwapChain> m_swapChain;
        std::shared_ptr<RenderPass> m_renderPass;
        std::shared_ptr<DescriptorPools> m_descriptorPools;

        std::shared_ptr<VkPipelineLayout_T> m_pipelineLayout;
        std::shared_ptr<VkPipeline_T> m_pipeline;

        void createGraphicsPipeline(VkVertexInputBindingDescription const &bindingDescription,
                                    std::vector<VkVertexInputAttributeDescription> const &attributeDescriptions);
    };

    class CommandPool {
    public:
        CommandPool(std::shared_ptr<Device> inDevice)
                : m_device{inDevice},
                  m_commandPool{} {
            createCommandPool();
        }

        inline std::shared_ptr<VkCommandPool_T> const &commandPool() { return m_commandPool; }
        inline std::shared_ptr<Device> const &device() { return m_device; }

    private:
        std::shared_ptr<Device> m_device;
        std::shared_ptr<VkCommandPool_T> m_commandPool;

        void createCommandPool();
    };

    class SingleTimeCommands {
    public:
        SingleTimeCommands(std::shared_ptr<Device> inDevice,
                           std::shared_ptr<CommandPool> commandPool)
                : m_device{inDevice},
                  m_pool{commandPool},
                  m_commandBuffer{} {
            create();
        }

        void create();
        void begin();
        void end();

        inline std::shared_ptr<VkCommandBuffer_T> commandBuffer() { return m_commandBuffer; }
    private:
        std::shared_ptr<Device> m_device;
        std::shared_ptr<CommandPool> m_pool;
        std::shared_ptr<VkCommandBuffer_T> m_commandBuffer;
    };

    class Buffer {
    public:
        Buffer(std::shared_ptr<Device> inDevice, VkDeviceSize size, VkBufferUsageFlags usage,
               VkMemoryPropertyFlags properties)
                : m_device{inDevice},
                  m_buffer{},
                  m_bufferMemory{} {
            createBuffer(size, usage, properties);
        }

        void copyTo(std::shared_ptr<CommandPool> cmds, std::shared_ptr<Buffer> const &srcBuffer,
                    VkDeviceSize size) {
            copyTo(cmds, *srcBuffer, size);
        }

        void copyTo(std::shared_ptr<CommandPool> cmds, Buffer const &srcBuffer, VkDeviceSize size);

        void copyRawTo(void const *dataRaw, size_t size);

        inline std::shared_ptr<VkBuffer_T> const &buffer() const { return m_buffer; }
        inline std::shared_ptr<VkDeviceMemory_T> const &memory() const { return m_bufferMemory; }

        ~Buffer() {
            /* ensure order of destruction.
             * free the memory after the buffer has been destroyed because the buffer is bound to
             * the memory, so the buffer is still using the memory until the buffer is destroyed.
             */
            m_buffer.reset();
            m_bufferMemory.reset();
        }

    private:
        std::shared_ptr<Device> m_device;

        std::shared_ptr<VkBuffer_T> m_buffer;
        std::shared_ptr<VkDeviceMemory_T> m_bufferMemory;

        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    };

    class Semaphore {
        std::shared_ptr<Device> m_device;

        std::shared_ptr<VkSemaphore_T> m_semaphore;

        void createSemaphore();

    public:
        Semaphore(std::shared_ptr<Device> const &inDevice)
                : m_device{inDevice},
                  m_semaphore{} {
            createSemaphore();
        }

        inline std::shared_ptr<VkSemaphore_T> const &semaphore() { return m_semaphore; }
    };

    class Image {
    public:
        Image(std::shared_ptr<Device> const &inDevice, uint32_t inWidth,
              uint32_t inHeight, VkFormat format,
              VkImageTiling tiling, VkImageUsageFlags usage,
              VkMemoryPropertyFlags properties)
                : m_device{inDevice},
                  m_image{},
                  m_imageMemory{},
                  m_width{inWidth},
                  m_height{inHeight} {
            createImage(format, tiling, usage, properties);
        }

        // Image was created by another object, but we are to manage it.
        Image(std::shared_ptr<Device> const &inDevice, std::shared_ptr<VkImage_T> const &inImage,
              uint32_t inWidth, uint32_t inHeight)
                : m_device{inDevice},
                  m_image{inImage},
                  m_imageMemory{},
                  m_width{inWidth},
                  m_height{inHeight} {
        }

        void copyBufferToImage(Buffer &buffer, std::shared_ptr<CommandPool> const &pool);

        void transitionImageLayout(VkFormat format, VkImageLayout oldLayout,
                                   VkImageLayout newLayout, std::shared_ptr<CommandPool> const &pool);

        virtual ~Image() {
            m_image.reset();
            m_imageMemory.reset();
        }

        inline std::shared_ptr<Device> const &device() { return m_device; }
        inline std::shared_ptr<VkImage_T> const &image() { return m_image; }
    protected:
        std::shared_ptr<Device> m_device;

        std::shared_ptr<VkImage_T> m_image;
        std::shared_ptr<VkDeviceMemory_T> m_imageMemory;
        uint32_t m_width;
        uint32_t m_height;

        void createImage(VkFormat format, VkImageTiling tiling,
                         VkImageUsageFlags usage, VkMemoryPropertyFlags properties);

        bool hasStencilComponent(VkFormat format) {
            return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
        }
    };

    class ImageView {
    public:
        ImageView(std::shared_ptr<Image> const &inImage, VkFormat format,
                  VkImageAspectFlags aspectFlags)
                : m_image{inImage},
                  m_imageView{} {
            createImageView(format, aspectFlags);
        }

        inline std::shared_ptr<VkImageView_T> const &imageView() { return m_imageView; }
        inline std::shared_ptr<Image> const &image() { return m_image; }

    protected:
        std::shared_ptr<Image> m_image;
        std::shared_ptr<VkImageView_T> m_imageView;

    private:
        inline VkDevice logicalDevice() { return m_image->device()->logicalDevice().get(); }

        void createImageView(VkFormat format, VkImageAspectFlags aspectFlags);
    };

    class ImageFactory {
    public:
        static std::shared_ptr<Image> createTextureImage(
                std::shared_ptr<Device> const &inDevice,
                std::shared_ptr<CommandPool> const &cmdPool,
                std::shared_ptr<TextureDescription> const &texture);

        static std::shared_ptr<Image> createDepthImage(std::shared_ptr<SwapChain> inSwapChain) {
            VkExtent2D extent = inSwapChain->extent();
            std::shared_ptr<Image> img{new Image{inSwapChain->device(), extent.width,
                                                 extent.height,
                                                 inSwapChain->device()->depthFormat(),
                                                 VK_IMAGE_TILING_OPTIMAL,
                                                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT}};

            return img;
        }
    };

    class ImageSampler {
    public:
        ImageSampler(std::shared_ptr<Device> const &inDevice,
                     std::shared_ptr<CommandPool> const &pool,
                     std::shared_ptr<TextureDescription> const &textureDescription)
                : m_device{inDevice},
                  m_image{},
                  m_imageView{},
                  m_sampler{} {
            m_image = ImageFactory::createTextureImage(m_device, pool, textureDescription);

            m_imageView.reset(new ImageView(m_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT));

            createTextureSampler();
        }

        inline std::shared_ptr<VkSampler_T> const &sampler() { return m_sampler; }
        inline std::shared_ptr<Image> const &image() { return m_image; }
        inline std::shared_ptr<ImageView> const &imageView() { return m_imageView; }

    private:
        std::shared_ptr<Device> m_device;
        std::shared_ptr<Image> m_image;
        std::shared_ptr<ImageView> m_imageView;
        std::shared_ptr<VkSampler_T> m_sampler;

        void createTextureSampler();
    };
} /* namespace vulkan */

#define DEBUG
#ifdef DEBUG
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif

std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
VkVertexInputBindingDescription getBindingDescription();

class GraphicsVulkan : public Graphics {
public:
    GraphicsVulkan(WindowType *window, uint32_t level)
        :m_instance{new vulkan::Instance(window)},
        m_device{new vulkan::Device{m_instance}},
        m_swapChain{new vulkan::SwapChain{m_device}},
        m_renderPass{new vulkan::RenderPass{m_device, m_swapChain}},
        m_descriptorPools{new vulkan::DescriptorPools{m_device}},
        m_graphicsPipeline{new vulkan::Pipeline{m_swapChain, m_renderPass, m_descriptorPools,
            getBindingDescription(), getAttributeDescriptions()}},
        m_commandPool{new vulkan::CommandPool{m_device}},
        texturesLevel{},
        texturesLevelStarter{},
        texturesLevelFinisher{},
        texturesChanged{false},
        swapChainImages{},
        swapChainImageViews{},
        m_uniformBufferLighting{new vulkan::Buffer{m_device, sizeof (glm::vec3), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}},
        staticObjsData{},
        dynObjsData{},
        levelFinisherObjsData{},
        levelStarterStaticObjsData{},
        levelStarterDynObjsData{},
        maze{},
        levelFinisher{},
        levelStarter{},
        levelTracker{level},
        swapChainFramebuffers{},
        commandBuffers{},
        m_imageAvailableSemaphore{m_device},
        m_renderFinishedSemaphore{m_device},
        m_depthImageView{new vulkan::ImageView{vulkan::ImageFactory::createDepthImage(m_swapChain),
                                               m_device->depthFormat(), VK_IMAGE_ASPECT_DEPTH_BIT}}
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

    std::shared_ptr<vulkan::DescriptorPools> m_descriptorPools;
    std::shared_ptr<vulkan::Pipeline> m_graphicsPipeline;
    std::shared_ptr<vulkan::CommandPool> m_commandPool;

    struct TextureDataVulkan : public TextureData {
        TextureDataVulkan(std::shared_ptr<vulkan::Device> inDevice)
                : m_device{inDevice}
        {}
        std::shared_ptr<vulkan::Device> m_device;
        std::shared_ptr<vulkan::ImageSampler> m_sampler;
    };
    TextureMap texturesLevel;
    TextureMap texturesLevelStarter;
    TextureMap texturesLevelFinisher;
    bool texturesChanged;

    std::vector<VkImage> swapChainImages;
    std::vector<std::shared_ptr<vulkan::ImageView>> swapChainImageViews;

    std::shared_ptr<vulkan::Buffer> m_uniformBufferLighting;

    struct UniformWrapper {
        std::shared_ptr<vulkan::Device> m_device;
        /* for passing data other than the vertex data to the vertex shader */
        std::shared_ptr<vulkan::DescriptorSet> m_descriptorSet;
        std::shared_ptr<vulkan::Buffer> m_uniformBuffer;
        std::shared_ptr<vulkan::DescriptorPools> m_pools;

        UniformWrapper(std::shared_ptr<vulkan::Device> const &inDevice,
                       std::shared_ptr<vulkan::DescriptorSet> const &inDescriptorSet,
                       std::shared_ptr<vulkan::Buffer> const &inUniformBuffer,
                       std::shared_ptr<vulkan::DescriptorPools> const &inPools)
        : m_device(inDevice),
          m_descriptorSet{inDescriptorSet},
          m_uniformBuffer{inUniformBuffer},
          m_pools{inPools}
        {}
    };

    /* vertex buffer and index buffer. the index buffer indicates which vertices to draw and in
     * the specified order.  Note, vertices can be listed twice if they should be part of more
     * than one triangle.
     */
    struct DrawObjectDataVulkan : public DrawObjectData {
        std::shared_ptr<vulkan::Device> m_device;
        std::shared_ptr<vulkan::Buffer> m_vertexBuffer;
        std::shared_ptr<vulkan::Buffer> m_indexBuffer;
        std::vector<std::shared_ptr<UniformWrapper> > uniforms;

        DrawObjectDataVulkan(std::shared_ptr<vulkan::Device> inDevice)
        : m_device{inDevice} {
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

    std::vector<VkFramebuffer> swapChainFramebuffers;

    std::vector<VkCommandBuffer> commandBuffers;

    /* use semaphores to coordinate the rendering and presentation. Could also use fences
     * but fences are more for coordinating in our program itself and not for internal
     * Vulkan coordination of resource usage.
     */
    vulkan::Semaphore m_imageAvailableSemaphore;
    vulkan::Semaphore m_renderFinishedSemaphore;

    /* depth buffer image */
    std::shared_ptr<vulkan::ImageView> m_depthImageView;

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
    std::shared_ptr<vulkan::Buffer> createVertexBuffer(std::vector<Vertex> const &vertices);
    std::shared_ptr<vulkan::Buffer> createIndexBuffer(std::vector<uint32_t> const &indices);
    void updatePerspectiveMatrix();

    void cleanupSwapChain();
    void createImageViews();
    void createFramebuffers();
    void createCommandBuffers();
    void initializeCommandBuffer(VkCommandBuffer &commandBuffer, size_t index);
    void initializeCommandBufferDrawObjects(VkCommandBuffer &commandBuffer, DrawObjectTable const & objs);
    void createDepthResources();
    void updateDescriptorSet(std::shared_ptr<vulkan::Buffer> const &uniformBuffer, VkImageView imageView, VkSampler textureSampler,
                             std::shared_ptr<vulkan::Buffer> const &lightingSource, std::shared_ptr<vulkan::DescriptorSet> const &descriptorSet);
    std::shared_ptr<vulkan::Image> createTextureImage(TextureDescription *texture);
    void createTextureSampler(VkSampler &textureSampler);
};
#endif

