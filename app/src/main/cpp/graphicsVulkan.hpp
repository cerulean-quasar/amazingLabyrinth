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

namespace vk {
    const std::vector<const char *> validationLayers = {
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

    uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter,
                            VkMemoryPropertyFlags properties);

    class Instance {
    public:
        Instance(WindowType *inWindow)
                : m_window{},
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

        VkResult CreateDebugReportCallbackEXT(
                const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                const VkAllocationCallbacks *pAllocator,
                VkDebugReportCallbackEXT *pCallback);
        void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
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
                m_physicalDevice{VK_NULL_HANDLE},
                m_logicalDevice{},
                m_graphicsQueue{VK_NULL_HANDLE},
                m_presentQueue{VK_NULL_HANDLE},
                m_depthFormat{} {
            pickPhysicalDevice();
            createLogicalDevice();
            m_depthFormat = findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                                                 VK_FORMAT_D24_UNORM_S8_UINT},
                                                VK_IMAGE_TILING_OPTIMAL,
                                                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        }

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device = m_physicalDevice);

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device = m_physicalDevice);

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
        bool m_depthFormatInitialized;

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

    class CommandPool;

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

        inline std::shared_ptr<VkBuffer_T> const &buffer() { return m_buffer; }
        inline std::shared_ptr<VkDeviceMemory_T> const &memory() { return m_bufferMemory; }

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
    };

    class ImageView {
    public:
        ImageView(std::shared_ptr<Image> inImage, VkFormat format,
                  VkImageAspectFlags aspectFlags)
                : m_image{inImage},
                  m_imageView{} {
            createImageView(format, aspectFlags);
        }

        inline std::shared_ptr<VkImageView_T> const &imageView() { return m_imageView; }

    protected:
        std::shared_ptr<Image> m_image;
        std::shared_ptr<VkImageView_T> m_imageView;

    private:
        inline VkDevice logicalDevice() { return m_image->device()->logicalDevice().get(); }

        void createImageView(VkFormat format, VkImageAspectFlags aspectFlags);
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

    struct RenderPass {
        std::shared_ptr<Device> m_device;

        std::shared_ptr<VkRenderPass_T> m_renderPass;

        RenderPass(std::shared_ptr<Device> const &inDevice, std::shared_ptr<SwapChain> const &swapChain)
                : m_device{inDevice},
                  m_renderPass{} {
            createRenderPass(swapChain);
        }

        inline std::shared_ptr<VkRenderPass_T> const &renderPass() { return m_renderPass; }

    private:
        void createRenderPass(std::shared_ptr<SwapChain> const &swapChain);
    };

    class DescriptorPools;

    class DescriptorSet {
        friend DescriptorPools;
    private:
        std::shared_ptr<VkDescriptorSet_T> m_descriptorSet;

        DescriptorSet(std::shared_ptr<VkDescriptorSet_T> inDsc)
                : m_descriptorSet{inDsc} {
        }

    public:
        std::shared_ptr<VkDescriptorSet_T> const &descriptorSet() { return m_descriptorSet; }
    };

    class DescriptorPool {
        friend DescriptorPools;
    private:
        std::shared_ptr<Device> m_device;
        std::shared_ptr<VkDescriptorPool_T> m_descriptorPool;
        uint32_t m_totalDescriptorsInPool;
        uint32_t m_totalDescriptorsAllocated;

        DescriptorPool(std::shared_ptr<Device> inDevice, uint32_t totalDescriptors)
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

            auto deleter = [m_device](VkDescriptorPool descriptorPoolRaw) {
                vkDestroyDescriptorPool(m_device->logicalDevice().get(), descriptorPoolRaw, nullptr);
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
    class DescriptorPools {
    private:
        std::shared_ptr<Device> m_device;
        uint32_t constexpr m_numberOfDescriptorSetsInPool = 1024;
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

        void setDescriptorSetLayout(VkDescriptorSetLayout layout) {
            m_descriptorSetLayout = layout;
        }

        std::shared_ptr<VkDescriptorSetLayout_T> const &descriptorSetLayout() { return m_descriptorSetLayout; }

        std::shared_ptr<DescriptorSet> allocateDescriptor() {
            if (m_descriptorSetLayout == VK_NULL_HANDLE) {
                throw (std::runtime_error(
                        "DescriptorPool::allocateDescriptor - no descriptor set layout"));
            }

            auto deleter = [&m_unusedDescriptors](VkDescriptorSet descSet) {
                m_unusedDescriptors.push_back(descSet);
            };

            if (m_unusedDescriptors.size() > 0) {
                VkDescriptorSet descriptorSet = m_unusedDescriptors.back();
                m_unusedDescriptors.pop_back();

                return std::shared_ptr<DescriptorSet>{new DescriptorSet{std::shared_ptr{descriptorSet, deleter}}};
            } else {
                for (auto &&descriptorPool : m_descriptorPools) {
                    if (descriptorPool.hasAvailableDescriptorSets()) {
                        VkDescriptorSet descriptorSet = descriptorPool.allocateDescriptor(
                                m_descriptorSetLayout);
                        return std::shared_ptr<DescriptorSet>{new DescriptorSet{std::shared_ptr{descriptorSet, deleter}}};
                    }
                }


                DescriptorPool newDescriptorPool(m_device, m_numberOfDescriptorSetsInPool);
                m_descriptorPools.push_back(newDescriptorPool);
                VkDescriptorSet descriptorSet = newDescriptorPool.allocateDescriptor(
                        m_descriptorSetLayout);
                return std::shared_ptr(new DescriptorSet(std::shared_ptr(descriptorSet, deleter)));
            }
        }
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
                 std::shared_ptr<DescriptorPools> &inDescriptorPools)
                : m_pipelineLayout{},
                  m_pipeline{},
                  m_device{inSwapChain->device()},
                  m_swapChain{inSwapChain},
                  m_renderPass{inRenderPass},
                  m_descriptorPools{inDescriptorPools} {
            createGraphicsPipeline();
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

        std::shared_ptr<VkPipeline_T> m_pipeline;
        std::shared_ptr<VkPipelineLayout_T> m_pipelineLayout;

        void createGraphicsPipeline();
    };

    class SwapChainCommands {
    public:
        SwapChainCommands(std::shared_ptr<SwapChain> const &inSwapChain,
                          std::shared_ptr<CommandPool> const &inPool,
                          std::shared_ptr<RenderPass> const &inRenderPass,
                          std::shared_ptr<ImageView> const &inDepthImage)
                : m_swapChain{inSwapChain},
                  m_pool{inPool},
                  m_renderPass{inRenderPass},
                  m_depthImage{inDepthImage},
                  m_images{},
                  m_imageViews{},
                  m_framebuffers{},
                  m_commandBuffers{} {
            /* retrieve the image handles from the swap chain. - handles cleaned up by Vulkan
             * Note: Vulkan may have chosen to create more images than specified in minImageCount,
             * so we have to requery the image count here.
             */
            uint32_t imageCount;
            vkGetSwapchainImagesKHR(m_swapChain->device()->logicalDevice().get(), m_swapChain->swapChain().get(),
                                    &imageCount, nullptr);
            m_images.resize(imageCount);
            vkGetSwapchainImagesKHR(m_swapChain->device()->logicalDevice().get(), m_swapChain->swapChain().get(),
                                    &imageCount, m_images.data());

            // The swap chain images are owned by the swap chain.  They go away when the swap chain
            // is destroyed.  So make sure the swap chain hangs around until all the images are
            // deleted.  So add it as a capture in the deleter.
            auto deleter = [m_swapChain](VkImage imageRaw) {
                // do nothing
            };

            for (auto &img : m_images) {
                std::shared_ptr<VkImage_T> imgptr(img, deleter);
                VkExtent2D ext = m_swapChain->extent();
                ImageView imageView{
                    std::shared_ptr<Image>(new Image(m_swapChain->device(), imgptr, ext.width, ext.height)),
                            m_swapChain->imageFormat(), VK_IMAGE_ASPECT_COLOR_BIT};
                m_imageViews.push_back(imageView);
            }
            createFramebuffers(m_renderPass, m_depthImage);
            createCommandBuffers();
        }

        void initializeCommandBuffers(std::shared_ptr<Pipeline> const &graphicsPipeline,
                                      std::shared_ptr<Level> const &maze,
                                      DrawObjectTable const &staticObjsData,
                                      DrawObjectTable const &dynObjsData,
                                      std::shared_ptr<LevelStarter> const &levelStarter,
                                      DrawObjectTable const &levelStarterStaticObjsData,
                                      DrawObjectTable const &levelStarterDynObjsData,
                                      std::shared_ptr<LevelFinish> const &levelFinisher,
                                      DrawObjectTable const &levelFinisherObjsData);

        ~SwapChainCommands() {
            vkFreeCommandBuffers(m_swapChain->device()->logicalDevice().get(), m_pool->commandPool().get(),
                                 m_commandBuffers.size(), m_commandBuffers.data());
            m_framebuffers.clear();
            m_imageViews.clear();
        }

    private:
        std::shared_ptr<SwapChain> m_swapChain;
        std::shared_ptr<CommandPool> m_pool;
        std::shared_ptr<RenderPass> m_renderPass;
        std::shared_ptr<ImageView> m_depthImage;

        std::vector<VkImage> m_images;
        std::vector<ImageView> m_imageViews;
        std::vector<std::shared_ptr<VkFramebuffer_T>> m_framebuffers;
        std::vector<VkCommandBuffer> m_commandBuffers;

        void initializeCommandBufferDrawObjects(std::shared_ptr<Pipeline> const &pipeline,
                                                size_t index, DrawObjectTable const &objs);

        void createFramebuffers(std::shared_ptr<RenderPass> &renderPass,
                                std::shared_ptr<ImageView> &depthImage);

        void createCommandBuffers();

        void initializeCommandBuffer(size_t index, std::shared_ptr<Pipeline> const &graphicsPipeline,
                                     std::shared_ptr<Level> const &maze, DrawObjectTable const &staticObjsData,
                                     DrawObjectTable const &dynObjsData,
                                     std::shared_ptr<LevelStarter> const &levelStarter,
                                     DrawObjectTable const &levelStarterStaticObjsData,
                                     DrawObjectTable const &levelStarterDynObjsData,
                                     std::shared_ptr<LevelFinish> const &levelFinisher,
                                     DrawObjectTable const &levelFinisherObjsData);
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

    struct ImageViewDepth : public ImageView {
        ImageViewDepth(std::shared_ptr<SwapChain> inSwapChain, std::shared_ptr<CommandPool> cmds)
                : ImageView(std::shared_ptr(
                new Image{inSwapChain->device(), inSwapChain->extent().width,
                          inSwapChain->extent().height,
                          inSwapChain->device()->depthFormat(),
                          VK_IMAGE_TILING_OPTIMAL,
                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT}),
                            inSwapChain->device()->depthFormat(),
                            VK_IMAGE_ASPECT_DEPTH_BIT) {
            m_image->transitionImageLayout(
                    inSwapChain->device()->depthFormat(), VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, cmds);
        }
    };

    class ImageFactory {
    public:
        static std::shared_ptr<Image> createTextureImage(
                std::shared_ptr<Device> const &inDevice,
                std::shared_ptr<TextureDescription> const &texture,
                std::shared_ptr<CommandPool> const &cmdPool);

        static std::shared_ptr<Image> &createDeptImage(std::shared_ptr<SwapChain> inSwapChain,
                                                       std::shared_ptr<CommandPool> cmdPool) {
            VkExtent2D extent = inSwapChain->extent();
            std::shared_ptr<Image> img{new Image{inSwapChain->device(), extent.width,
                      extent.height,
                      inSwapChain->device()->depthFormat(),
                      VK_IMAGE_TILING_OPTIMAL,
                      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT}};

            img->transitionImageLayout(
                    inSwapChain->device()->depthFormat(), VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, cmdPool);

            return img;
        }
    };

    class ImageSampler {
    public:
        ImageSampler(std::shared_ptr<Device> inDevice,
                          std::shared_ptr<TextureDescription> textureDescription,
                          std::shared_ptr<CommandPool> &pool)
                : m_device{inDevice},
                  m_image{},
                  m_imageView{},
                  m_sampler{} {
            m_image = ImageFactory::createTextureImage(m_device, textureDescription, pool);

            m_imageView.reset(new ImageView(m_image, VK_FORMAT_R8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT));

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

} // namespace vk

class TextureDataVulkan : public TextureData {
public:
    inline std::shared_ptr<vk::ImageSampler> const &sampler() { return m_sampler; }
private:
    std::shared_ptr<vk::ImageSampler> m_sampler;
};

class UniformWrapper {
public:
    /* for passing data other than the vertex data to the vertex shader */
    std::shared_ptr<vk::DescriptorSet> m_descriptorSet;
    std::shared_ptr<vk::Buffer> m_uniformBuffer;
    std::shared_ptr<vk::Buffer> m_uniformBufferLighting;
    std::shared_ptr<vk::ImageSampler> m_sampler;

    UniformWrapper::UniformWrapper(std::shared_ptr<vk::Device> const &inDevice,
                                   std::shared_ptr<vk::DescriptorPools> const &descriptorPools,
                                   std::shared_ptr<vk::ImageSampler> const &inSampler,
                                   std::shared_ptr<vk::Buffer> const &inUniformBufferLighting,
                                   UniformBufferObject const &ubo)
            : m_descriptorSet{},
              m_uniformBuffer{},
              m_uniformBufferLighting{},
              m_sampler{}
    {
        VkBuffer uniformBuffer;
        VkDeviceMemory uniformBufferMemory;

        m_uniformBuffer = createUniformBuffer(inDevice, sizeof (UniformBufferObject));
        m_uniformBuffer->copyRawTo(&ubo, sizeof (ubo));

        m_descriptorSet = descriptorPools->allocateDescriptor();
        updateDescriptorSet(inDevice);
    }

    inline std::shared_ptr<vk::DescriptorSet> const &descriptorSet() { return m_descriptorSet; }
    inline std::shared_ptr<vk::Buffer> const &uniformBuffer() { return m_uniformBuffer; }
    inline std::shared_ptr<vk::Buffer> const &uniformBufferLighting() { return m_uniformBufferLighting; }
    inline std::shared_ptr<vk::ImageSampler> const &imageSampler() { return m_sampler; }
    static std::shared_ptr<vk::Buffer> createUniformBuffer(
            std::shared_ptr<vk::Device> const &device, size_t bufferSize);

private:
    void updateDescriptorSet(std::shared_ptr<vk::Device> const &inDevice);
};

class DrawObjectDataVulkan : public DrawObjectData {
    std::shared_ptr<vk::Device> m_device;
    std::shared_ptr<vk::DescriptorPools> m_descriptorPools;

    /* vertex buffer and index buffer. the index buffer indicates which vertices to draw and in
     * the specified order.  Note, vertices can be listed twice if they should be part of more
     * than one triangle.
     */
    vk::Buffer m_vertexBuffer;
    vk::Buffer m_indexBuffer;

    std::shared_ptr<vk::Buffer> m_uniformBufferLighting;
    std::vector<std::shared_ptr<UniformWrapper> > m_uniforms;

    void copyVerticesToBuffer(std::shared_ptr<vk::CommandPool> const &cmdpool,
                              std::shared_ptr<DrawObject> const &drawObj);

    void copyIndicesToBuffer(std::shared_ptr<vk::CommandPool> const &cmdpool,
                             std::shared_ptr<DrawObject> const &drawObj);

public:
    DrawObjectDataVulkan(std::shared_ptr<vk::Device> const &inDevice,
                         std::shared_ptr<vk::CommandPool> const &inPool,
                         std::shared_ptr<vk::DescriptorPools> const &inDescriptorPools,
                         std::shared_ptr<DrawObject> const &drawObj,
                         std::shared_ptr<vk::Buffer> const &inLightingPosition)
            : m_device{inDevice},
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
                     glm::mat4 const &perspective,
                     glm::mat4 const &view,
                     TextureMap &textures);
    void update(std::shared_ptr<DrawObject> const &obj, glm::mat4 const &perspective,
                glm::mat4 const &view, TextureMap &textures);

    inline void clearUniforms() { m_uniforms.clear(); }
};

class LevelSequence {
public:
    LevelSequence(std::shared_ptr<vk::Device> const &inDevice,
                  std::shared_ptr<vk::CommandPool> const &inPool,
                  std::shared_ptr<vk::DescriptorPools> const &inDescriptorPools,
                  uint32_t level)
        :m_levelTracker{level},
         m_texturesLevel{},
         m_texturesLevelStarter{},
         m_texturesLevelFinisher{},
         m_texturesChanged{false},
         m_uniformBufferLighting{UniformWrapper::createUniformBuffer(inDevice, sizeof (glm::vec3))},
         m_staticObjsData{},
         m_dynObjsData{},
         m_levelFinisherObjsData{},
         m_levelStarterStaticObjsData{},
         m_levelStarterDynObjsData{},
         m_level{m_levelTracker.getLevel()},
         m_levelFinisher{}, /* needs to be empty until the player finishes the level */
         m_levelStarter{m_levelTracker.getLevelStarter()}
    {
        glm::vec3 lightingVector = glm::vec3(0.0f, 0.0f, 1.28f/*0.01-1.28*/);
        m_uniformBufferLighting->copyRawTo(&lightingVector, sizeof (glm::vec3));

        initializeLevelData(m_levelStarter, m_levelStarterStaticObjsData,
                            m_levelStarterDynObjsData, m_texturesLevelStarter);
        initializeLevelData(m_level, m_staticObjsData, m_dynObjsData, m_texturesLevel);
    }

    bool updateData();
    void updateAcceleration(float x, float y, float z);
private:
    LevelTracker m_levelTracker;
    TextureMap m_texturesLevel;
    TextureMap m_texturesLevelStarter;
    TextureMap m_texturesLevelFinisher;
    bool m_texturesChanged;

    std::shared_ptr<vk::Buffer> m_uniformBufferLighting;

    DrawObjectTable m_staticObjsData;
    DrawObjectTable m_dynObjsData;
    DrawObjectTable m_levelFinisherObjsData;
    DrawObjectTable m_levelStarterStaticObjsData;
    DrawObjectTable m_levelStarterDynObjsData;

    std::shared_ptr<Level> m_level;
    std::shared_ptr<LevelFinish> m_levelFinisher;
    std::shared_ptr<LevelStarter> m_levelStarter;

    void initializeLevelData(std::shared_ptr<Level> const &level, DrawObjectTable &staticObjsData,
                             DrawObjectTable &dynObjsData, TextureMap &textures);
    void updateLevelData(DrawObjectTable &objsData, TextureMap &textures);
    UniformBufferObject getViewPerspectiveMatrix();
};

class GraphicsVulkan : public Graphics {
public:
    GraphicsVulkan(WindowType *window, uint32_t level)
            : instance{new vk::Instance{window}},
              device{new vk::Device{instance}},
              descriptorPools{device},
              renderPass{VK_NULL_HANDLE},
              uniformBufferLighting{VK_NULL_HANDLE},
              uniformBufferMemoryLighting{VK_NULL_HANDLE},
              levelTracker{level},
              pipelineLayout{},
              graphicsPipeline{},
              commandPool{},
              commandBuffers{},
              imageAvailableSemaphore{},
              renderFinishedSemaphore{},
              depthImage{VK_NULL_HANDLE},
              depthImageMemory{VK_NULL_HANDLE},
              depthImageView{VK_NULL_HANDLE} {}

    virtual void init(WindowType *window);

    virtual void initThread() {}

    virtual void cleanupThread() {}

    virtual void cleanup();

    virtual bool updateData();

    virtual void updateAcceleration(float x, float y, float z);

    virtual void drawFrame();

    virtual void destroyWindow();

    virtual void recreateSwapChain();

    virtual ~GraphicsVulkan() {}

private:

    std::shared_ptr<vk::Instance> instance;
    std::shared_ptr<vk::Device> device;
    std::shared_ptr<vk::SwapChain> swapchain;
    std::shared_ptr<vk::RenderPass> renderPass;
    std::shared_ptr<vk::DescriptorPools> descriptorPools;
    std::shared_ptr<vk::Pipeline> graphicsPipeline;
    std::shared_ptr<vk::CommandPool> commandPool;

    LevelSequence level;

    /* depth buffer image */
    std::shared_ptr<vk::ImageView> depthImageView;

    std::shared_ptr<vk::SwapChainCommands> swapChainCommands;

    /* use semaphores to coordinate the rendering and presentation. Could also use fences
     * but fences are more for coordinating in our program itself and not for internal
     * Vulkan coordination of resource usage.
     */
    vk::Semaphore imageAvailableSemaphore;
    vk::Semaphore renderFinishedSemaphore;


    void addTextures(TextureMap &texture);

    UniformBufferObject getViewPerspectiveMatrix();

    void addObjects(DrawObjectTable &objs, TextureMap &texture);

    void addObject(DrawObjectEntry &obj, TextureMap &texture);

    void addUniforms(DrawObjectEntry &obj, TextureMap &texture);

    bool updateLevelData(Level *level, DrawObjectTable &objsData, TextureMap &textures);

    void initializeLevelData(Level *level, DrawObjectTable &staticObjsData,
                             DrawObjectTable &dynObjsData, TextureMap &textures);

    void createUniformBuffer(VkBuffer &uniformBuffer, VkDeviceMemory &uniformBufferMemory);

    void createVertexBuffer(std::vector<Vertex> const &vertices, VkBuffer &vertexBuffer,
                            VkDeviceMemory &vertexBufferMemory);

    void createIndexBuffer(std::vector<uint32_t> const &indices, VkBuffer &indexBuffer,
                           VkDeviceMemory &indexBufferMemory);

    void updatePerspectiveMatrix();

    void cleanupSwapChain();

    VkShaderModule createShaderModule(const std::vector<char> &code);

    void createRenderPass();

    void createGraphicsPipeline();

    void createFramebuffers();

    void
    createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                 VkBuffer &buffer, VkDeviceMemory &bufferMemory);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    VkCommandBuffer beginSingleTimeCommands();

    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void createCommandPool();

    void createCommandBuffers();

    void initializeCommandBuffer(VkCommandBuffer &commandBuffer, size_t index);

    void initializeCommandBufferDrawObjects(VkCommandBuffer &commandBuffer,
                                            DrawObjectTable const &objs);

    void createSemaphores();

    void createDepthResources();

    VkFormat findDepthFormat();

    VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                 VkFormatFeatureFlags features);

    bool hasStencilComponent(VkFormat format);

    void
    updateDescriptorSet(VkBuffer uniformBuffer, VkImageView imageView, VkSampler textureSampler,
                        VkBuffer lightingSource, VkDescriptorSet &descriptorSet);

    void createDescriptorSetLayout(VkDescriptorSetLayout &descriptorSetLayout);

    void createTextureImage(TextureDescription *texture, VkImage &textureImage,
                            VkDeviceMemory &textureImageMemory);

    void createTextureSampler(VkSampler &textureSampler);

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
                               VkImageLayout newLayout);

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
};
#endif
