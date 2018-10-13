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

        Device(std::shared_ptr<Instance> inInstance)
                : m_instance (inInstance),
                m_physicalDevice{VK_NULL_HANDLE},
                m_logicalDevice{},
                m_graphicsQueue{VK_NULL_HANDLE},
                m_presentQueue{VK_NULL_HANDLE} {
            pickPhysicalDevice();
            createLogicalDevice();
        }

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device = m_physicalDevice);

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device = m_physicalDevice);

        inline std::shared_ptr<VkDevice_T> const &logicalDevice() { return m_logicalDevice; }

        inline VkPhysicalDevice physicalDevice() { return m_physicalDevice; }

        inline VkQueue graphicsQueue() { return m_graphicsQueue; }

        inline VkQueue presentQueue() { return m_presentQueue; }

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

        const std::vector<const char *> deviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates,
                            VkImageTiling tiling, VkFormatFeatureFlags features);

        VkFormat findDepthFormat();

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

        void copyRawTo(void *dataRaw, size_t size);

        inline std::shared_ptr<VkBuffer_T> const &buffer() { return m_buffer; }

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

    private:
        std::shared_ptr<Device> m_device;
        std::shared_ptr<VkCommandPool_T> m_commandPool;

        void createCommandPool();
    };

    class Image {
    public:
        Image(std::shared_ptr<Device> inDevice, uint32_t inWidth,
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

        void copyBufferToImage(Buffer &buffer, std::shared_ptr<CommandPool> &pool);

        void transitionImageLayout(VkFormat format, VkImageLayout oldLayout,
                                   VkImageLayout newLayout, std::shared_ptr<CommandPool> &pool);

        virtual ~Image() {
            m_image.reset();
            m_imageMemory.reset();
        }

        inline std::shared_ptr<Device> const &device() { return m_device; }
    private:
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
                : image{inImage},
                  imageView{} {
            imageView = createImageView(format, aspectFlags);
        }

        ~ImageView() {
            if (imageView != VK_NULL_HANDLE) {
                vkDestroyImageView(logicalDevice(), imageView, nullptr);
            }
        }

    private:
        std::shared_ptr<Image> image;
        VkImageView imageView;

        inline VkDevice logicalDevice() { return image->device()->logicalDevice().get(); }

        VkImageView createImageView(VkFormat format, VkImageAspectFlags aspectFlags);
    };

    struct SwapChain {
        std::shared_ptr<DeviceVulkan> device;
        VkSwapchainKHR swapChain;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;

        SwapChain(std::shared_ptr<DeviceVulkan> inDevice)
                : device(inDevice),
                  swapChain{VK_NULL_HANDLE},
                  swapChainImageFormat{},
                  swapChainExtent{} {
            createSwapChain();
        }

        ~SwapChain() {
            if (swapChain != VK_NULL_HANDLE) {
                vkDestroySwapchainKHR(device->logicalDevice, swapChain, nullptr);
            }
        }

    private:
        void createSwapChain();

        VkSurfaceFormatKHR
        chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

        VkPresentModeKHR
        chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    };

    struct ImageViewDepth : public ImageViewVulkan {
        ImageViewDepth(std::shared_ptr<SwapChain> inSwapChain, std::shared_ptr<CommandPool> cmds)
                : ImageViewVulkan(std::shared_ptr(
                new ImageMemoryVulkan{inSwapChain->device, inSwapChain->swapChainExtent.width,
                                      inSwapChain->swapChainExtent.height,
                                      findDepthFormat(inSwapChain->device),
                                      VK_IMAGE_TILING_OPTIMAL,
                                      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT}),
                                  findDepthFormat(inSwapChain->device),
                                  VK_IMAGE_ASPECT_DEPTH_BIT), {
            static_cast<ImageMemoryVulkan *>(image.get())->transitionImageLayout(
                    findDepthFormat(inSwapChain->device), VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, cmds);
        }
    };

    struct RenderPass {
        std::shared_ptr<DeviceVulkan> device;

        VkRenderPass renderPass;

        RenderPass(std::shared_ptr<DeviceVulkan> inDevice, std::shared_ptr<SwapChain> &swapchain)
                : device{inDevice},
                  renderPass{VK_NULL_HANDLE} {
            createRenderPass(swapchain);
        }

    private:
        void createRenderPass(std::shared_ptr<SwapChain> &swapchain);
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
        std::shared_ptr<DeviceVulkan> device;
        VkDescriptorPool descriptorPool;
        uint32_t totalDescriptorsInPool;
        uint32_t totalDescriptorsAllocated;

        DescriptorPool(std::shared_ptr<DeviceVulkan> inDevice, uint32_t totalDescriptors)
                : device{inDevice},
                  descriptorPool{},
                  totalDescriptorsInPool{totalDescriptors},
                  totalDescriptorsAllocated{0} {
            // no more descriptors in all the descriptor pools.  create another descriptor pool...
            std::array<VkDescriptorPoolSize, 3> poolSizes = {};

            // one for each wall and +3 for the floor, ball, and hole.
            poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes[0].descriptorCount = totalDescriptorsInPool;
            poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes[1].descriptorCount = totalDescriptorsInPool;
            poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes[2].descriptorCount = totalDescriptorsInPool;
            VkDescriptorPoolCreateInfo poolInfo = {};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = totalDescriptorsInPool;

            VkDescriptorPool descriptorPool;
            if (vkCreateDescriptorPool(device->logicalDevice, &poolInfo, nullptr,
                                       &descriptorPool) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor pool!");
            }
        }

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
                int rc = vkAllocateDescriptorSets(device->logicalDevice, &allocInfo,
                                                  &descriptorSet);
                if (rc != VK_SUCCESS) {
                    throw std::runtime_error("failed to allocate descriptor set!");
                }
                totalDescriptorsAllocated++;
                return descriptorSet;
            }
        }

        bool hasAvailableDescriptorSets() {
            return totalDescriptorsAllocated < totalDescriptorsInPool;
        }

        ~DescriptorPool() {
            vkDestroyDescriptorPool(device->logicalDevice, descriptorPool, nullptr);
        }
    };

/* for passing data other than the vertex data to the vertex shader */
    class DescriptorPools {
    private:
        std::shared_ptr<DeviceVulkan> device;
        uint32_t constexpr numberOfDescriptorSetsInPool = 1024;
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        std::vector<DescriptorPool> descriptorPools;
        std::vector<VkDescriptorSet> unusedDescriptors;

        void createDescriptorSetLayout();

        void destroyResources() {
            unusedDescriptors.clear();
            descriptorPools.clear();

            if (descriptorSetLayout != VK_NULL_HANDLE) {
                vkDestroyDescriptorSetLayout(device->logicalDevice, descriptorSetLayout, nullptr);
                descriptorSetLayout = VK_NULL_HANDLE;
            }

        }

    public:
        DescriptorPools(std::shared_ptr<DeviceVulkan> inDevice)
                : device{inDevice},
                  descriptorSetLayout(VK_NULL_HANDLE),
                  descriptorPools{},
                  unusedDescriptors{} {
            createDescriptorSetLayout();
        }

        ~DescriptorPools() { destroyResources(); }

        void setDescriptorSetLayout(VkDescriptorSetLayout layout) {
            descriptorSetLayout = layout;
        }

        VkDescriptorSetLayout getDescriptorSetLayout() { return descriptorSetLayout; }

        void freeDescriptor(VkDescriptorSet descriptorSet) {
            unusedDescriptors.push_back(descriptorSet);
        }

        DescriptorSet allocateDescriptor() {
            if (descriptorSetLayout == VK_NULL_HANDLE) {
                throw (std::runtime_error(
                        "DescriptorPool::allocateDescriptor - no descriptor set layout"));
            }

            auto deleter = [&unusedDescriptors](VkDescriptorSet descSet) {
                unusedDescriptors.push_back(descSet);
            };

            if (unusedDescriptors.size() > 0) {
                VkDescriptorSet descriptorSet = unusedDescriptors.back();
                unusedDescriptors.pop_back();

                return DescriptorSet(std::shared_ptr(descriptorSet, deleter));
            } else {
                for (auto &&descriptorPool : descriptorPools) {
                    if (descriptorPool.hasAvailableDescriptorSets()) {
                        VkDescriptorSet descriptorSet = descriptorPool.allocateDescriptor(
                                descriptorSetLayout);
                        return DescriptorSet(std::shared_ptr(descriptorSet, deleter));
                    }
                }


                DescriptorPool newDescriptorPool(device, numberOfDescriptorSetsInPool);
                descriptorPools.push_back(newDescriptorPool);
                VkDescriptorSet descriptorSet = newDescriptorPool.allocateDescriptor(
                        descriptorSetLayout);
                return DescriptorSet(std::shared_ptr(descriptorSet, deleter));
            }
        }
    };

    class Semaphore {
        std::shared_ptr<DeviceVulkan> device;

        std::shared_ptr<VkSemaphore_T> semaphore;

        void createSemaphore();

    public:
        Semaphore(std::shared_ptr<DeviceVulkan> &inDevice)
                : device{inDevice},
                  semaphore{} {
            createSemaphore();
        }
    };

    class Shader {
        std::shared_ptr<DeviceVulkan> device;

        std::shared_ptr<VkShaderModule_T> shaderModule;

        void createShaderModule(std::string const &codeFile);

    public:
        Shader(std::shared_ptr<DeviceVulkan> const &inDevice, std::string const &codeFile)
                : device(inDevice) {
            createShaderModule(codeFile);
        }

        inline std::shared_ptr<VkShaderModule_T> const &shader() { return shaderModule; }
    };

    struct Pipeline {
        std::shared_ptr<VkPipelineLayout_T> pipelineLayout;
        std::shared_ptr<VkPipeline_T> pipeline;

        Pipeline(std::shared_ptr<SwapChain> &inSwapChain, std::shared_ptr<RenderPass> &inRenderPass,
                 std::shared_ptr<DescriptorPools> &inDescriptorPools)
                : pipelineLayout{},
                  pipeline{},
                  device{inSwapChain->device},
                  swapChain{inSwapChain},
                  renderPass{inRenderPass},
                  descriptorPools{inDescriptorPools} {
            createGraphicsPipeline();
        }

        ~Pipeline() {
            pipeline.reset();
            pipelineLayout.reset();
        }

    private:
        std::shared_ptr<DeviceVulkan> device;
        std::shared_ptr<SwapChain> swapChain;
        std::shared_ptr<RenderPass> renderPass;
        std::shared_ptr<DescriptorPools> descriptorPools;

        void createGraphicsPipeline();
    };

    struct SwapChainCommands {
        std::vector<ImageViewVulkan> imageViews;
        std::vector<std::shared_ptr<VkFramebuffer_T>> framebuffers;
        std::vector<VkCommandBuffer> commandBuffers;

        SwapChainCommands(std::shared_ptr<SwapChain> &inSwapChain,
                          std::shared_ptr<CommandPool> inPool,
                          std::shared_ptr<RenderPass> &inRenderPass,
                          std::shared_ptr<ImageViewVulkan> &inDepthImage,
                          glm::vec4 &bgColor)
                : swapChain{inSwapChain},
                  pool{inPool},
                  renderPass{inRenderPass},
                  depthImage{inDepthImage},
                  imageViews{},
                  framebuffers{},
                  commandBuffers{} {
            /* retrieve the image handles from the swap chain. - handles cleaned up by Vulkan
             * Note: Vulkan may have chosen to create more images than specified in minImageCount,
             * so we have to requery the image count here.
             */
            uint32_t imageCount;
            vkGetSwapchainImagesKHR(swapChain->device->logicalDevice, swapChain->swapChain,
                                    &imageCount, nullptr);
            std::vector<VkImage> swapChainImages;
            swapChainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(swapChain->device->logicalDevice, swapChain->swapChain,
                                    &imageCount, swapChainImages.data());
            for (auto &img : swapChainImages) {
                std::shared_ptr<ImageVulkan> image(img);
                ImageViewVulkan imageView{image, swapChain->swapChainImageFormat,
                                          VK_IMAGE_ASPECT_COLOR_BIT};
                imageViews.push_back(imageView);
            }
            createFramebuffers(renderPass, depthImage);
            createCommandBuffers(bgColor);
        }

        void initializeCommandBuffers(std::shared_ptr<Pipeline> &graphicsPipeline,
                                      std::shared_ptr<Level> &maze, DrawObjectTable &staticObjsData,
                                      DrawObjectTable &dynObjsData,
                                      std::shared_ptr<LevelStarter> &levelStarter,
                                      DrawObjectTable &levelStarterStaticObjsData,
                                      DrawObjectTable &levelStarterDynObjsData,
                                      std::shared_ptr<LevelFinish> &levelFinisher,
                                      DrawObjectTable &levelFinisherObjsData);

        ~SwapChainCommands() {
            vkFreeCommandBuffers(swapChain->device->logicalDevice, pool->commands,
                                 commandBuffers.size(), commandBuffers.data());
            framebuffers.clear();
            imageViews.clear();
        }

    private:
        std::shared_ptr<SwapChain> swapChain;
        std::shared_ptr<CommandPool> pool;
        std::shared_ptr<RenderPass> renderPass;
        std::shared_ptr<ImageViewVulkan> depthImage;

        void initializeCommandBufferDrawObjects(std::shared_ptr<Pipeline> &pipeline,
                                                VkCommandBuffer &commandBuffer,
                                                DrawObjectTable const &objs);

        void createFramebuffers(std::shared_ptr<RenderPass> &renderPass,
                                std::shared_ptr<ImageViewVulkan> &depthImage);

        void createCommandBuffers(glm::vec4 &bgColor);

        void initializeCommandBuffer(size_t index, std::shared_ptr<Pipeline> &graphicsPipeline,
                                     std::shared_ptr<Level> &maze, DrawObjectTable &staticObjsData,
                                     DrawObjectTable &dynObjsData,
                                     std::shared_ptr<LevelStarter> &levelStarter,
                                     DrawObjectTable &levelStarterStaticObjsData,
                                     DrawObjectTable &levelStarterDynObjsData,
                                     std::shared_ptr<LevelFinish> &levelFinisher,
                                     DrawObjectTable &levelFinisherObjsData);
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

    struct TextureDataVulkan : public TextureData {
        std::shared_ptr<DeviceVulkan> device;
        std::shared_ptr<ImageViewVulkan> img;
        VkSampler sampler;

        TextureDataVulkan(std::shared_ptr<DeviceVulkan> inDevice,
                          std::shared_ptr<TextureDescription> textureDescription
                          std::shared_ptr<CommandPool> &pool)
                : device{inDevice},
                  img{},
                  sampler{} {
            createTextureImage(textureDescription, pool);
            createTextureSampler(sampler);

        }

        virtual ~TextureDataVulkan() {
            vkDestroySampler(device->logicalDevice, sampler, nullptr);
        }

    private:
        void createTextureSampler(VkSampler &textureSampler);

        void createTextureImage(std::shared_ptr<TextureDescription> &texture,
                                std::shared_ptr<CommandPool> &pool);
    };

    struct UniformWrapper {
        /* for passing data other than the vertex data to the vertex shader */
        DescriptorSet descriptorSet;
        BufferVulkan uniformBuffer;

        UniformWrapper(DescriptorSet inDescriptorSet,
                       BufferVulkan inUniformBuffer)
                : uniformBuffer{inUniformBuffer},
                  descriptorSet{inDescriptorSet} {
        }
    };

/* vertex buffer and index buffer. the index buffer indicates which vertices to draw and in
 * the specified order.  Note, vertices can be listed twice if they should be part of more
 * than one triangle.
 */
    class DrawObjectDataVulkan : public DrawObjectData {
        std::shared_ptr<DeviceVulkan> device;
        BufferVulkan vertexBuffer
        BufferVulkan indexBuffer;
        std::vector<std::shared_ptr<UniformWrapper> > uniforms;

        void copyVerticesToBuffer(std::shared_ptr<CommandPool> const &cmdpool,
                                  std::shared_ptr<DrawObject> const &drawObj);

        void copyIndicesToBuffer(std::shared_ptr<CommandPool> const &cmdpool,
                                 std::shared_ptr<DrawObject> const &drawObj);

    public:
        DrawObjectDataVulkan(std::shared_ptr<DeviceVulkan> const &inDevice,
                             std::shared_ptr<CommandPool> const &inPool,
                             std::shared_ptr<DrawObject> const &drawObj)
                : device(inDevice),
                  vertexBuffer{sizeof(drawObj->vertices[0]) * drawObj->vertices.size(),
                               VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
                  indexBuffer{sizeof(drawObj->indices[0]) * drawObj->indices.size(),
                              VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT} {
            copyVerticesToBuffer(inPool, drawObj);
            copyIndicesToBuffer(inPool, drawObj);
        }

        void addUniforms(std::shared_ptr<DrawObject> const &obj, TextureMap &textures);
    };

    class GraphicsVulkan : public Graphics {
    public:
        GraphicsVulkan(WindowType *window, uint32_t level)
                : instance{window},
                  device{instance},
                  descriptorPools{device},
                  texturesLevel{device},
                  texturesLevelStarter{device},
                  texturesLevelFinisher{device},
                  texturesChanged{false},
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

        std::shared_ptr<InstanceVulkan> instance;
        std::shared_ptr<DeviceVulkan> device;
        std::shared_ptr<SwapChain> swapchain;

        DescriptorPools descriptorPools;

        TextureMap texturesLevel;
        TextureMap texturesLevelStarter;
        TextureMap texturesLevelFinisher;
        bool texturesChanged;


        VkBuffer uniformBufferLighting;
        VkDeviceMemory uniformBufferMemoryLighting;


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

} // namespace vk
#endif
