/**
 * Copyright 2022 Cerulean Quasar. All Rights Reserved.
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
// vulkan_wrapper.h must come before vk_mem_alloc.h because vk_mem_alloc.h includes vulkan/vulkan.h
// and I don't want the prototypes from there, I want the ones from vulkan_wrapper.h.
#include <vulkan_wrapper.h>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnullability-completeness"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include "vk_mem_alloc.h"
#pragma clang diagnostic pop


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

#include <glm/glm.hpp>
#include <map>

#include "graphicsCpuArch.hpp"
#include "android.hpp"
#include "levels/finisher/types.hpp"
#include "levelTracker/levelTracker.hpp"
#include "levels/basic/level.hpp"

namespace vulkan {
#ifdef DEBUG
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif

    const std::vector<const char *> validationLayers = {
            /* required for checking for errors and getting error messages */
            //"VK_LAYER_LUNARG_standard_validation"

            //"VK_LAYER_GOOGLE_threading",
            //"VK_LAYER_LUNARG_parameter_validation",
            //"VK_LAYER_LUNARG_object_tracker",
            //"VK_LAYER_LUNARG_core_validation",
            //"VK_LAYER_GOOGLE_unique_objects"

            "VK_LAYER_KHRONOS_validation"
    };

    class VulkanLibrary {
    public:
        VulkanLibrary() {
            if (!InitVulkan()) {
                throw std::runtime_error("Could not find vulkan library.");
            }
        }
    };

    class Instance {
    public:
        Instance(std::shared_ptr<WindowType> inWindow)
                : m_loader{},
                  m_window{std::move(inWindow)},
                  m_instance{},
                  m_callback{},
                  m_surface{} {
            createInstance();
            setupDebugCallback();
            createSurface();
        }

        inline std::shared_ptr<VkInstance_T> const &instance() { return m_instance; }

        inline std::shared_ptr<VkSurfaceKHR_CQ> const &surface() { return m_surface; }

        inline std::shared_ptr<VkDebugReportCallbackEXT_CQ> const &callback() { return m_callback; }

    private:
        VulkanLibrary m_loader;
        std::shared_ptr<WindowType> m_window;
        std::shared_ptr<VkInstance_T> m_instance;
        std::shared_ptr<VkDebugReportCallbackEXT_CQ> m_callback;
        std::shared_ptr<VkSurfaceKHR_CQ> m_surface;

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
                VkDebugReportFlagsEXT,
                VkDebugReportObjectTypeEXT,
                uint64_t,
                size_t,
                int32_t,
                const char *,
                const char *msg,
                void *) {

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

        struct DeviceProperties {
            std::string m_name;
            std::string m_vulkanAPIVersion;
            DeviceProperties(std::string inName, uint32_t version)
                    :m_name{std::move(inName)},
                     m_vulkanAPIVersion{std::to_string(VK_VERSION_MAJOR(version)) + "." +
                                        std::to_string(VK_VERSION_MINOR(version)) + "." +
                                        std::to_string(VK_VERSION_PATCH(version))} {
            }
        };

        Device(std::shared_ptr<Instance> const &inInstance)
                : m_instance (inInstance),
                  m_physicalDevice{},
                  m_logicalDevice{},
                  m_allocator{},
                  m_graphicsQueue{},
                  m_presentQueue{},
                  m_depthFormat{} {
            pickPhysicalDevice();
            createLogicalDevice();
            m_depthFormat = findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                                                 VK_FORMAT_D24_UNORM_S8_UINT},
                                                VK_IMAGE_TILING_OPTIMAL,
                                                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

            createAllocator();
        }

        DeviceProperties properties();

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

        inline std::shared_ptr<VmaAllocator_T> const &allocator() { return m_allocator; }

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

        // Vulkan Memory Allocator objects
        std::shared_ptr<VmaAllocator_T> m_allocator;

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

        void createAllocator();

        bool isDeviceSuitable(VkPhysicalDevice device);

        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    };

    class SwapChain {
    public:
        SwapChain(std::shared_ptr<Device> inDevice, uint32_t width = 0, uint32_t height = 0)
                : m_device(inDevice),
                  m_swapChain{VK_NULL_HANDLE},
                  m_imageFormat{},
                  m_extent{},
                  m_preTransform{} {
            createSwapChain(width, height);
        }

        inline std::shared_ptr<Device> const &device() { return m_device; }
        inline std::shared_ptr<VkSwapchainKHR_CQ> const &swapChain() {return m_swapChain; }
        inline VkFormat imageFormat() { return m_imageFormat; }
        inline VkExtent2D extent() { return m_extent; }
        inline VkSurfaceTransformFlagsKHR preTransform() { return m_preTransform; }
    private:
        std::shared_ptr<Device> m_device;
        std::shared_ptr<VkSwapchainKHR_CQ> m_swapChain;
        VkFormat m_imageFormat;
        VkExtent2D m_extent;
        VkSurfaceTransformFlagsKHR m_preTransform;

        void createSwapChain(uint32_t width, uint32_t height);

        VkSurfaceFormatKHR
        chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

        VkPresentModeKHR
        chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities,
                                    uint32_t width, uint32_t height);
    };

    class RenderPass {
    public:
        struct ImageAttachmentInfo {
            VkAttachmentLoadOp loadOp;
            VkAttachmentStoreOp storeOp;
            VkFormat format;
            VkImageLayout initialLayout;
            VkImageLayout finalLayout;

            ImageAttachmentInfo(
                VkAttachmentLoadOp inLoadOp,
                VkAttachmentStoreOp inStoreOp,
                VkFormat inFormat,
                VkImageLayout inInitialLayout,
                VkImageLayout inFinalLayout)
                : loadOp{inLoadOp},
                  storeOp{inStoreOp},
                  format{inFormat},
                  initialLayout{inInitialLayout},
                  finalLayout{inFinalLayout}
            {}

            ImageAttachmentInfo(ImageAttachmentInfo const &other) noexcept
                    : loadOp{other.loadOp},
                      storeOp{other.storeOp},
                      format{other.format},
                      initialLayout{other.initialLayout},
                      finalLayout{other.finalLayout}
            {}

            ImageAttachmentInfo(ImageAttachmentInfo &&other) noexcept
                : loadOp{other.loadOp},
                  storeOp{other.storeOp},
                  format{other.format},
                  initialLayout{other.initialLayout},
                  finalLayout{other.finalLayout}
            {}
        };
        inline std::shared_ptr<VkRenderPass_CQ> const &renderPass() { return m_renderPass; }
        inline size_t numberColorAttachments() { return m_colorAttachmentFormats.size(); }

        // For a normal render pass that includes a color and depth attachment
        static std::shared_ptr<RenderPass> createRenderPass(std::shared_ptr<Device> const &inDevice,
                                                            std::shared_ptr<SwapChain> const &inSwapChain) {
            return std::shared_ptr<RenderPass>{new RenderPass(inDevice, inSwapChain)};
        }

        // for a render pass that only uses the depth attachment (to read the z-buffer for shadow
        // mapping and to read the depth of an object.
        static std::shared_ptr<RenderPass> createDepthTextureRenderPass(
                std::shared_ptr<Device> const &inDevice,
                std::vector<ImageAttachmentInfo> const &info,
                std::shared_ptr<ImageAttachmentInfo> const &depthInfo)
        {
            return std::shared_ptr<RenderPass>{new RenderPass(inDevice, info, depthInfo)};
        }

        bool isCompatible(std::shared_ptr<RenderPass> const &other) {
            if (other->m_hasDepthAttachment != m_hasDepthAttachment) {
                return false;
            }

            if (m_hasDepthAttachment && (other->m_depthAttachmentFormat != m_depthAttachmentFormat)) {
                return false;
            }

            // its ok if the number of attachments don't match up, just the matching pairs have
            // to be of the same format.
            for (size_t i = 0;
                 i < m_colorAttachmentFormats.size() && i < other->m_colorAttachmentFormats.size();
                 i++)
            {
                if (m_colorAttachmentFormats[i] != other->m_colorAttachmentFormats[i]) {
                    return false;
                }
            }

            return true;
        }
    private:
        // for creating a render pass that uses a color and depth attachment (i.e. the normal render pass)
        RenderPass(std::shared_ptr<Device> const &inDevice, std::shared_ptr<SwapChain> const &swapChain)
                : m_device{inDevice},
                  m_renderPass{}
        {
            createRenderPass(swapChain);
        }

        // for creating a render pass with only a depth attachment.
        RenderPass(std::shared_ptr<Device> const &inDevice,
                std::vector<ImageAttachmentInfo> const &colorImageInfo,
                std::shared_ptr<ImageAttachmentInfo> const &depthInfo)
                : m_device{inDevice},
                  m_renderPass{}
        {
            createRenderPassDepthTexture(colorImageInfo, depthInfo);
        }

        std::shared_ptr<Device> m_device;
        std::shared_ptr<VkRenderPass_CQ> m_renderPass;
        size_t m_nbrColorAttachments;
        std::vector<VkFormat> m_colorAttachmentFormats;
        VkFormat m_depthAttachmentFormat;
        bool m_hasDepthAttachment;

        void createRenderPass(std::shared_ptr<SwapChain> const &swapChain);
        void createRenderPassDepthTexture(std::vector<ImageAttachmentInfo> const &colorImageInfo,
                std::shared_ptr<ImageAttachmentInfo> const &depthInfo);
    };

    class DescriptorPools;

    CQ_DEFINE_VULKAN_CREATOR(VkDescriptorSet)
    class DescriptorSet {
        friend DescriptorPools;
    private:
        std::shared_ptr<DescriptorPools> m_descriptorPools;
        std::shared_ptr<VkDescriptorSet_CQ> m_descriptorSet;

        DescriptorSet(std::shared_ptr<DescriptorPools> inDescriptorPools,
                      std::shared_ptr<VkDescriptorSet_CQ> const &inDsc)
                : m_descriptorPools{inDescriptorPools},
                  m_descriptorSet{inDsc} {
        }

    public:
        inline std::shared_ptr<VkDescriptorSet_CQ> const &descriptorSet() { return m_descriptorSet; }
    };

    // Descriptor Set Layout creators and deleters used by render details files, so declare them here
    CQ_DEFINE_VULKAN_CREATOR(VkDescriptorSetLayout)
    inline void deleteVkDescriptorSetLayout_CQ(std::shared_ptr<Device> const &inDevice, VkDescriptorSetLayout_CQ *layout) {
        vkDestroyDescriptorSetLayout(inDevice->logicalDevice().get(), getVkType<>(layout), nullptr);
        deleteIfNecessary(layout);
    }

    class DescriptorSetLayout {
    public:
        virtual std::shared_ptr<VkDescriptorSetLayout_CQ> const &descriptorSetLayout() = 0;
        virtual uint32_t numberOfDescriptors() = 0;
        virtual VkDescriptorPoolCreateInfo const &poolCreateInfo() = 0;
        virtual ~DescriptorSetLayout() {}
    };

    CQ_DEFINE_VULKAN_CREATOR(VkDescriptorPool)
    inline void deleteVkDescriptorPool_CQ(std::shared_ptr<Device> const &inDevice, VkDescriptorPool_CQ *pool) {
        vkDestroyDescriptorPool(inDevice->logicalDevice().get(), getVkType<>(pool), nullptr);
        deleteIfNecessary(pool);
    }
    class DescriptorPool {
        friend DescriptorPools;
    private:
        std::shared_ptr<Device> m_device;
        std::shared_ptr<VkDescriptorPool_CQ> m_descriptorPool;
        uint32_t m_totalDescriptorsInPool;
        uint32_t m_totalDescriptorsAllocated;

        DescriptorPool(std::shared_ptr<Device> const &inDevice, uint32_t totalDescriptors,
                       VkDescriptorPoolCreateInfo const &poolInfo)
                : m_device{inDevice},
                  m_descriptorPool{},
                  m_totalDescriptorsInPool{totalDescriptors},
                  m_totalDescriptorsAllocated{0} {
            // no more descriptors in all the descriptor pools.  create another descriptor pool...

            VkDescriptorPool descriptorPoolRaw;
            if (vkCreateDescriptorPool(m_device->logicalDevice().get(), &poolInfo, nullptr,
                                       &descriptorPoolRaw) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor pool!");
            }

            auto deleter = [inDevice](VkDescriptorPool_CQ *pool) {
                deleteVkDescriptorPool_CQ(inDevice, pool);
            };

            m_descriptorPool.reset(createVkDescriptorPool_CQ(descriptorPoolRaw), deleter);
        }

        VkDescriptorSet_CQ *allocateDescriptor(std::shared_ptr<DescriptorSetLayout> const &layout) {
            if (m_totalDescriptorsAllocated == m_totalDescriptorsInPool) {
                return createVkDescriptorSet_CQ(VK_NULL_HANDLE);
            } else {
                VkDescriptorSet descriptorSet;
                VkDescriptorSetLayout layouts[] = {getVkType<>(layout->descriptorSetLayout().get())};
                VkDescriptorSetAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = getVkType<>(m_descriptorPool.get());
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
                return createVkDescriptorSet_CQ(descriptorSet);
            }
        }

        bool hasAvailableDescriptorSets() {
            /* Purposely returning that we are out of descriptors one less than we have to because
             * on Google Pixel 4, it returns that there are no more descriptors when there should be
             * one left.
             */
            return m_totalDescriptorsAllocated < m_totalDescriptorsInPool - 1;
        }
    };

/* for passing data other than the vertex data to the vertex shader */
    class DescriptorPools : public std::enable_shared_from_this<DescriptorPools> {
        friend DescriptorSet;
    private:
        std::shared_ptr<Device> m_device;
        std::shared_ptr<DescriptorSetLayout> m_descriptorSetLayout;
        std::vector<DescriptorPool> m_descriptorPools;
        std::vector<VkDescriptorSet_CQ*> m_unusedDescriptors;

    public:
        DescriptorPools(std::shared_ptr<Device> const &inDevice,
                        std::shared_ptr<DescriptorSetLayout> const &inDescriptorSetLayout)
                : m_device{inDevice},
                  m_descriptorSetLayout{inDescriptorSetLayout},
                  m_descriptorPools{},
                  m_unusedDescriptors{} {
        }

        inline std::shared_ptr<VkDescriptorSetLayout_CQ> const &descriptorSetLayout() {
            return m_descriptorSetLayout->descriptorSetLayout();
        }

        std::shared_ptr<DescriptorSet> allocateDescriptor() {
            if (m_descriptorSetLayout == nullptr) {
                throw (std::runtime_error(
                        "DescriptorPool::allocateDescriptor - no descriptor set layout"));
            }

            auto deleter = [this](VkDescriptorSet_CQ *descSet) {
                m_unusedDescriptors.push_back(descSet);
            };

            if (m_unusedDescriptors.size() > 0) {
                VkDescriptorSet_CQ *descriptorSet = m_unusedDescriptors.back();
                m_unusedDescriptors.pop_back();

                return std::shared_ptr<DescriptorSet>{new DescriptorSet{shared_from_this(),
                                                                        std::shared_ptr<VkDescriptorSet_CQ>{descriptorSet, deleter}}};
            } else {
                for (auto &&descriptorPool : m_descriptorPools) {
                    if (descriptorPool.hasAvailableDescriptorSets()) {
                        VkDescriptorSet_CQ *descriptorSet = descriptorPool.allocateDescriptor(
                                m_descriptorSetLayout);
                        return std::shared_ptr<DescriptorSet>{new DescriptorSet{shared_from_this(),
                                std::shared_ptr<VkDescriptorSet_CQ>{descriptorSet, deleter}}};
                    }
                }


                DescriptorPool newDescriptorPool(m_device, m_descriptorSetLayout->numberOfDescriptors(),
                    m_descriptorSetLayout->poolCreateInfo());
                m_descriptorPools.push_back(newDescriptorPool);
                VkDescriptorSet_CQ *descriptorSet = newDescriptorPool.allocateDescriptor(
                        m_descriptorSetLayout);
                return std::shared_ptr<DescriptorSet>(new DescriptorSet(shared_from_this(),
                        std::shared_ptr<VkDescriptorSet_CQ>(descriptorSet, deleter)));
            }
        }
    };

    class Shader {
        std::shared_ptr<Device> m_device;

        std::shared_ptr<VkShaderModule_CQ> m_shaderModule;

        void createShaderModule(std::shared_ptr<FileRequester> const &inRequester, std::string const &codeFile);

    public:
        Shader(std::shared_ptr<FileRequester> const &inRequester,
               std::shared_ptr<Device> const &inDevice, std::string const &codeFile)
                : m_device(inDevice),
                  m_shaderModule{} {
            createShaderModule(inRequester, codeFile);
        }

        inline std::shared_ptr<VkShaderModule_CQ> const &shader() { return m_shaderModule; }
    };

    class Pipeline {
    public:
        Pipeline(std::shared_ptr<FileRequester> const &requester,
                 std::shared_ptr<Device> const &inDevice,
                 VkExtent2D const &extent,
                 std::shared_ptr<RenderPass> const &inRenderPass,
                 std::shared_ptr<DescriptorPools> const &inDescriptorPools,
                 VkVertexInputBindingDescription const &bindingDescription,
                 std::vector<VkVertexInputAttributeDescription> const &attributeDescription,
                 std::string const &vertShader,
                 std::string const &fragShader,
                 std::shared_ptr<Pipeline> const &derivedPipeline,
                 VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT)
                : m_device{inDevice},
                  m_renderPass{inRenderPass},
                  m_descriptorPools{inDescriptorPools},
                  m_extent{extent},
                  m_pipelineLayout{},
                  m_pipeline{} {
            createGraphicsPipeline(requester, bindingDescription, attributeDescription,
                    vertShader, fragShader, derivedPipeline, cullMode);
        }

        inline std::shared_ptr<RenderPass> const &renderPass() { return m_renderPass; }
        inline std::shared_ptr<DescriptorPools> const &descriptorPools() { return m_descriptorPools; }
        inline std::shared_ptr<VkPipeline_CQ> const &pipeline() { return m_pipeline; }
        inline std::shared_ptr<VkPipelineLayout_CQ> const &layout() { return m_pipelineLayout; }
        inline VkExtent2D extent() { return m_extent; }

        ~Pipeline() {
            m_pipeline.reset();
            m_pipelineLayout.reset();
        }

    private:
        std::shared_ptr<Device> m_device;
        std::shared_ptr<RenderPass> m_renderPass;
        std::shared_ptr<DescriptorPools> m_descriptorPools;
        VkExtent2D m_extent;

        std::shared_ptr<VkPipelineLayout_CQ> m_pipelineLayout;
        std::shared_ptr<VkPipeline_CQ> m_pipeline;

        void createGraphicsPipeline(std::shared_ptr<FileRequester> const &requester,
                VkVertexInputBindingDescription const &bindingDescription,
                std::vector<VkVertexInputAttributeDescription> const &attributeDescriptions,
                std::string const vertShader,
                std::string const fragShader,
                std::shared_ptr<Pipeline> const &derivedPipeline,
                VkCullModeFlags cullMode);
    };

    class CommandPool {
    public:
        CommandPool(std::shared_ptr<Device> inDevice)
                : m_device{inDevice},
                  m_commandPool{} {
            createCommandPool();
        }

        inline std::shared_ptr<VkCommandPool_CQ> const &commandPool() { return m_commandPool; }
        inline std::shared_ptr<Device> const &device() { return m_device; }

    private:
        std::shared_ptr<Device> m_device;
        std::shared_ptr<VkCommandPool_CQ> m_commandPool;

        void createCommandPool();
    };

    class Semaphore {
        std::shared_ptr<Device> m_device;

        std::shared_ptr<VkSemaphore_CQ> m_semaphore;

        void createSemaphore();

    public:
        Semaphore(std::shared_ptr<Device> const &inDevice)
                : m_device{inDevice},
                  m_semaphore{} {
            createSemaphore();
        }

        inline std::shared_ptr<VkSemaphore_CQ> const &semaphore() { return m_semaphore; }
    };

    class CommandBuffer {
    public:
        CommandBuffer(std::shared_ptr<Device> inDevice,
                      std::shared_ptr<CommandPool> commandPool,
                      VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
                : m_device{inDevice},
                  m_pool{commandPool},
                  m_commandBuffer{},
                  m_usageFlag{usage}
        {
            create();
        }

        void create();
        void begin();
        void end();
        void end(Semaphore &waitSemaphore, VkPipelineStageFlags pipelineStage, Semaphore &signalSemaphore);
        void end(Semaphore &signalSemaphore);

        inline std::shared_ptr<VkCommandBuffer_T> commandBuffer() { return m_commandBuffer; }
    private:
        std::shared_ptr<Device> m_device;
        std::shared_ptr<CommandPool> m_pool;
        std::shared_ptr<VkCommandBuffer_T> m_commandBuffer;
        VkCommandBufferUsageFlags m_usageFlag;
    };

    class Buffer {
    public:
        Buffer(std::shared_ptr<Device> inDevice, VkDeviceSize size, VkBufferUsageFlags usage,
               VkMemoryPropertyFlags properties)
                : m_device{inDevice},
                  m_buffer{},
                  m_allocation{} {
            createBuffer(size, usage, properties);
        }

        void copyTo(std::shared_ptr<CommandPool> cmds, std::shared_ptr<Buffer> const &srcBuffer,
                    VkDeviceSize size) {
            copyTo(cmds, *srcBuffer, size);
        }

        void copyTo(std::shared_ptr<CommandPool> cmds, Buffer const &srcBuffer, VkDeviceSize size);

        void copyRawTo(void const *dataRaw, size_t size);
        void copyRawFrom(void *dataRaw, size_t size) const;

        inline VkBuffer &buffer() { return m_buffer; }
        inline VkBuffer const &cbuffer() const { return m_buffer; }

        ~Buffer() {
            /* ensure order of destruction.
             * free the memory after the buffer has been destroyed because the buffer is bound to
             * the memory, so the buffer is still using the memory until the buffer is destroyed.
             */
            vmaDestroyBuffer(m_device->allocator().get(), m_buffer, m_allocation);
        }

    private:
        std::shared_ptr<Device> m_device;

        VkBuffer m_buffer;
        VmaAllocation m_allocation;

        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    };

    class Image {
    public:
        Image(std::shared_ptr<Device> const &inDevice, uint32_t inWidth,
              uint32_t inHeight, VkFormat inFormat,
              VkImageTiling tiling, VkImageUsageFlags usage,
              VkMemoryPropertyFlags properties)
                : m_device{inDevice},
                  m_image{},
                  m_allocation{},
                  m_format{inFormat},
                  m_width{inWidth},
                  m_height{inHeight},
                  m_needsFree{true} {
            createImage(inFormat, tiling, usage, properties);
        }

        // Image was created by another object, but we are to manage it.
        Image(std::shared_ptr<Device> const &inDevice, VkImage inImage,
              VkFormat inFormat, uint32_t inWidth, uint32_t inHeight)
                : m_device{inDevice},
                  m_image{inImage},
                  m_allocation{},
                  m_format{inFormat},
                  m_width{inWidth},
                  m_height{inHeight},
                  m_needsFree{false} {
        }

        void copyBufferToImage(Buffer &buffer, std::shared_ptr<CommandPool> const &pool);

        void copyImageToBuffer(Buffer &buffer, std::shared_ptr<CommandPool> const &pool);

        void transitionImageLayout(VkImageLayout oldLayout,
                                   VkImageLayout newLayout, std::shared_ptr<CommandPool> const &pool);

        virtual ~Image() {
            if (m_needsFree) {
                vmaDestroyImage(m_device->allocator().get(), m_image, m_allocation);
            }
        }

        inline VkFormat format() { return m_format; }
        inline std::shared_ptr<Device> const &device() { return m_device; }
        inline VkImage &image() { return m_image; }
        inline uint32_t width() { return m_width; }
        inline uint32_t height() { return m_height; }
    protected:
        std::shared_ptr<Device> m_device;

        VkImage m_image;
        VmaAllocation m_allocation;
        VkFormat m_format;
        uint32_t m_width;
        uint32_t m_height;
        bool m_needsFree;

        void createImage(VkFormat format, VkImageTiling tiling,
                         VkImageUsageFlags usage, VkMemoryPropertyFlags properties);

        bool hasStencilComponent(VkFormat format) {
            return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
        }
    };

    class ImageView {
    public:
        static std::shared_ptr<ImageView> createImageViewAndImage(
                std::shared_ptr<Device> const &inDevice,
                uint32_t inWidth,
                uint32_t inHeight,
                VkFormat format,
                VkImageTiling tiling,
                VkImageUsageFlags usage,
                VkMemoryPropertyFlags properties,
                VkImageAspectFlags aspectFlags)
        {
            return std::make_shared<ImageView>(std::make_shared<Image>(inDevice, inWidth, inHeight,
                    format, tiling, usage, properties), aspectFlags);
        }

        static std::shared_ptr<ImageView> createImageViewAndImage(
                std::shared_ptr<Device> const &inDevice,
                VkExtent2D const &extent,
                VkFormat format,
                VkImageTiling tiling,
                VkImageUsageFlags usage,
                VkMemoryPropertyFlags properties,
                VkImageAspectFlags aspectFlags)
        {
            return std::make_shared<ImageView>(std::make_shared<Image>(inDevice, extent.width, extent.height,
                                                                       format, tiling, usage, properties), aspectFlags);
        }

        ImageView(std::shared_ptr<Image> const &inImage,
                  VkImageAspectFlags aspectFlags)
                : m_image{inImage},
                  m_imageView{} {
            createImageView(inImage->format(), aspectFlags);
        }

        inline std::shared_ptr<VkImageView_CQ> const &imageView() { return m_imageView; }
        inline std::shared_ptr<Image> const &image() { return m_image; }

    protected:
        std::shared_ptr<Image> m_image;
        std::shared_ptr<VkImageView_CQ> m_imageView;

    private:
        inline VkDevice logicalDevice() { return m_image->device()->logicalDevice().get(); }

        void createImageView(VkFormat format, VkImageAspectFlags aspectFlags);
    };

    class ImageFactory {
    public:
        static std::shared_ptr<Image> createTextureImage(
                std::shared_ptr<Device> const &inDevice,
                std::shared_ptr<CommandPool> const &cmdPool,
                std::vector<char> const &pixels,
                uint32_t texWidth,
                uint32_t texHeight,
                uint32_t texChannels);

        static std::shared_ptr<Image> createDepthImage(std::shared_ptr<SwapChain> const &inSwapChain) {
            return createDepthImage(inSwapChain->device(), VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    inSwapChain->extent());
        }

        static std::shared_ptr<Image> createDepthImage(
            std::shared_ptr<Device> const &device,
            VkImageUsageFlags usage,
            VkExtent2D const &extent)
        {
            return createDepthImage(device, usage, extent.width, extent.height);
        }

        static std::shared_ptr<Image> createDepthImage(
            std::shared_ptr<Device> const &device,
            VkImageUsageFlags usage,
            uint32_t width,
            uint32_t height)
        {
            std::shared_ptr<Image> img{new Image{device, width,
                                                 height,
                                                 device->depthFormat(),
                                                 VK_IMAGE_TILING_OPTIMAL,
                                                 usage,
                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT}};

            return img;
        }
    };

    class ImageSampler {
    public:
        ImageSampler(std::shared_ptr<Device> const &inDevice,
                     std::shared_ptr<CommandPool> const &pool,
                     std::vector<char> const &pixels,
                     uint32_t texWidth,
                     uint32_t texHeight,
                     uint32_t texChannels,
                     VkSamplerAddressMode beyondBorderSampling = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                     VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK)
                : m_device{inDevice},
                  m_image{},
                  m_imageView{},
                  m_sampler{} {
            m_image = ImageFactory::createTextureImage(m_device, pool, pixels, texWidth, texHeight, texChannels);

            m_imageView.reset(new ImageView(m_image, VK_IMAGE_ASPECT_COLOR_BIT));

            createTextureSampler(beyondBorderSampling,
                                 borderColor);
        }

        ImageSampler(std::shared_ptr<Device> const &inDevice,
            std::shared_ptr<ImageView> const &inImageView,
            VkSamplerAddressMode beyondBorderSampling = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK)
            : m_device{inDevice},
              m_image{inImageView->image()},
              m_imageView{inImageView}
        {
            createTextureSampler(beyondBorderSampling, borderColor);
        }

        inline std::shared_ptr<VkSampler_CQ> const &sampler() { return m_sampler; }
        inline std::shared_ptr<Image> const &image() { return m_image; }
        inline std::shared_ptr<ImageView> const &imageView() { return m_imageView; }

        inline void updateImageView(std::shared_ptr<ImageView> const &imgview) {
            m_imageView = imgview;
            m_image = imgview->image();
        }
    private:
        std::shared_ptr<Device> m_device;
        std::shared_ptr<Image> m_image;
        std::shared_ptr<ImageView> m_imageView;
        std::shared_ptr<VkSampler_CQ> m_sampler;

        void createTextureSampler(
                VkSamplerAddressMode beyondBorderSampling,
                VkBorderColor borderColor);
    };


    // creators
    CQ_DEFINE_VULKAN_CREATOR(VkFramebuffer)

    // deleters
    inline void deleteVkFramebuffer_CQ(std::shared_ptr<Device> const &inDevice, VkFramebuffer_CQ *frameBuffer) {
        vkDestroyFramebuffer(inDevice->logicalDevice().get(), getVkType<>(frameBuffer), nullptr);
        deleteIfNecessary(frameBuffer);
    }

    class Framebuffer {
    public:
        static std::shared_ptr<VkFramebuffer_CQ> createRawFramebuffer(
                std::shared_ptr<Device> const &inDevice,
                std::shared_ptr<RenderPass> const &inRenderPass,
                std::vector<std::shared_ptr<ImageView>> inAttachments,
                uint32_t width,
                uint32_t height)
        {
            std::vector<VkImageView> attachments;
            attachments.reserve(inAttachments.size());
            for (auto const &attachment : inAttachments) {
                attachments.push_back(getVkType<>(attachment->imageView().get()));
            }
            return createRawFramebuffer(inDevice, inRenderPass, attachments, width, height);
        }

        static std::shared_ptr<VkFramebuffer_CQ> createRawFramebuffer(
                std::shared_ptr<Device> const &inDevice,
                std::shared_ptr<RenderPass> const &inRenderPass,
                std::vector<VkImageView> inAttachments,
                uint32_t width,
                uint32_t height)
        {
            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = getVkType<>(inRenderPass->renderPass().get());
            framebufferInfo.attachmentCount = static_cast<uint32_t>(inAttachments.size());
            framebufferInfo.pAttachments = inAttachments.data();
            framebufferInfo.width = width;
            framebufferInfo.height = height;
            framebufferInfo.layers = 1;

            VkFramebuffer framebuf;
            if (vkCreateFramebuffer(inDevice->logicalDevice().get(), &framebufferInfo, nullptr,
                                    &framebuf) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }

            auto deleter = [inDevice](VkFramebuffer_CQ *frameBuf) {
                deleteVkFramebuffer_CQ(inDevice, frameBuf);
            };

            std::shared_ptr<VkFramebuffer_CQ> framebuffer(createVkFramebuffer_CQ(framebuf), deleter);

            return framebuffer;
        }

        Framebuffer(std::shared_ptr<Device> const &inDevice,
                std::shared_ptr<RenderPass> const &inRenderPass,
                std::vector<std::shared_ptr<ImageView>> inAttachments,
                uint32_t inWidth,
                uint32_t inHeight)
            : m_device{inDevice},
              m_framebuffer{createRawFramebuffer(inDevice, inRenderPass, inAttachments, inWidth, inHeight)},
              m_width{inWidth},
              m_height{inHeight}
              {}
        inline VkExtent2D extent() { return VkExtent2D{m_width, m_height}; }
        inline std::shared_ptr<Device> const &device() { return m_device; }
        inline std::shared_ptr<VkFramebuffer_CQ> const &framebuffer() { return m_framebuffer; }
    private:
        std::shared_ptr<Device> m_device;
        std::shared_ptr<VkFramebuffer_CQ> m_framebuffer;
        uint32_t m_width;
        uint32_t m_height;
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
            vkGetSwapchainImagesKHR(m_swapChain->device()->logicalDevice().get(), getVkType<>(m_swapChain->swapChain().get()),
                                    &imageCount, nullptr);
            m_images.resize(imageCount);
            vkGetSwapchainImagesKHR(m_swapChain->device()->logicalDevice().get(), getVkType<>(m_swapChain->swapChain().get()),
                                    &imageCount, m_images.data());


            for (auto &img : m_images) {
                VkExtent2D ext = m_swapChain->extent();
                ImageView imageView{std::make_shared<Image>(m_swapChain->device(), img,
                                m_swapChain->imageFormat(), ext.width, ext.height),
                        VK_IMAGE_ASPECT_COLOR_BIT};
                m_imageViews.push_back(imageView);
            }
            createFramebuffers(m_renderPass, m_depthImage);
            createCommandBuffers();
        }

        inline size_t size() { return m_images.size(); }
        inline VkFramebuffer frameBuffer(size_t index) { return getVkType<>(m_framebuffers[index].get()); }
        inline VkCommandBuffer commandBuffer(size_t index) { return m_commandBuffers[index]; }

        ~SwapChainCommands() {
            vkFreeCommandBuffers(m_swapChain->device()->logicalDevice().get(), getVkType(m_pool->commandPool().get()),
                                 m_commandBuffers.size(), m_commandBuffers.data());
            m_commandBuffers.clear();
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
        std::vector<std::shared_ptr<VkFramebuffer_CQ>> m_framebuffers;
        std::vector<VkCommandBuffer> m_commandBuffers;

        void createFramebuffers(std::shared_ptr<RenderPass> &renderPass,
                                std::shared_ptr<ImageView> &depthImage);

        void createCommandBuffers();
    };

    template <typename VertexType>
    void copyVerticesToBuffer(std::shared_ptr<vulkan::CommandPool> const &cmdpool,
                              std::vector<VertexType> const &vertices,
                              Buffer &buffer)
    {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        /* use a staging buffer in the CPU accessable memory to copy the data into graphics card
         * memory.  Then use a copy command to copy the data into fast graphics card only memory.
         */
        vulkan::Buffer stagingBuffer(cmdpool->device(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        stagingBuffer.copyRawTo(vertices.data(), bufferSize);

        buffer.copyTo(cmdpool, stagingBuffer, bufferSize);
    }

    void copyIndicesToBuffer(std::shared_ptr<vulkan::CommandPool> const &cmdpool,
                             std::vector<uint32_t> const &indices,
                             Buffer &buffer);

    struct SurfaceDetails {
        std::shared_ptr<vulkan::RenderPass> renderPass;
        glm::mat4 preTransform;
        uint32_t surfaceWidth;
        uint32_t surfaceHeight;
    };
} /* namespace vulkan */
#endif

