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

#include "graphicsVulkan.hpp"

namespace vulkan {

    // define Vulkan creators
    CQ_DEFINE_VULKAN_CREATOR(VkSurfaceKHR)
    CQ_DEFINE_VULKAN_CREATOR(VkDebugReportCallbackEXT)
    CQ_DEFINE_VULKAN_CREATOR(VkSwapchainKHR)
    CQ_DEFINE_VULKAN_CREATOR(VkRenderPass)
    CQ_DEFINE_VULKAN_CREATOR(VkShaderModule)
    CQ_DEFINE_VULKAN_CREATOR(VkPipelineLayout)
    CQ_DEFINE_VULKAN_CREATOR(VkPipeline)
    CQ_DEFINE_VULKAN_CREATOR(VkCommandPool)
    CQ_DEFINE_VULKAN_CREATOR(VkSemaphore)
    CQ_DEFINE_VULKAN_CREATOR(VkImageView)
    CQ_DEFINE_VULKAN_CREATOR(VkSampler)

    // define Vulkan deleters

    // surface
    inline void deleteVkSurfaceKHR_CQ(std::shared_ptr<VkInstance_T> const &instance, VkSurfaceKHR_CQ *surface) {
        vkDestroySurfaceKHR(instance.get(), getVkType<>(surface), nullptr);
        deleteIfNecessary(surface);
    }

    // Debug Callback
    inline void deleteVkDebugReportCallbackEXT_CQ(std::shared_ptr<VkInstance_T> const &instance,
                                               void (destroyDebugCallback)(VkInstance instance,
                                                       VkDebugReportCallbackEXT callback,
                                                       const VkAllocationCallbacks *pAllocator),
                                               VkDebugReportCallbackEXT_CQ *callback) {
        destroyDebugCallback(instance.get(), getVkType<>(callback), nullptr);
        deleteIfNecessary(callback);
    }

    // swapchain
    inline void deleteVkSwapchainKHR_CQ(std::shared_ptr<Device> const &device, VkSwapchainKHR_CQ *swapchain) {
        vkDestroySwapchainKHR(device->logicalDevice().get(), getVkType<>(swapchain), nullptr);
        deleteIfNecessary(swapchain);
    }

    // render pass
    inline void deleteVkRenderPass_CQ(std::shared_ptr<Device> const &device, VkRenderPass_CQ *renderPass) {
        vkDestroyRenderPass(device->logicalDevice().get(), getVkType<>(renderPass), nullptr);
        deleteIfNecessary(renderPass);
    }

    // shader module
    inline void deleteVkShaderModule_CQ(std::shared_ptr<Device> const &inDevice, VkShaderModule_CQ *shaderModule) {
        vkDestroyShaderModule(inDevice->logicalDevice().get(), getVkType<>(shaderModule), nullptr);
        deleteIfNecessary(shaderModule);
    }

    // pipeline layout
    inline void deleteVkPipelineLayout_CQ(std::shared_ptr<Device> const &inDevice, VkPipelineLayout_CQ *pipelineLayout) {
        vkDestroyPipelineLayout(inDevice->logicalDevice().get(), getVkType<>(pipelineLayout), nullptr);
        deleteIfNecessary(pipelineLayout);
    }

    // pipeline
    inline void deleteVkPipeline_CQ(std::shared_ptr<Device> const &inDevice, VkPipeline_CQ *pipeline) {
        vkDestroyPipeline(inDevice->logicalDevice().get(), getVkType<>(pipeline), nullptr);
        deleteIfNecessary(pipeline);
    }

    // Command pool
    inline void deleteVkCommandPool_CQ(std::shared_ptr<Device> const &inDevice, VkCommandPool_CQ *commandPool) {
        vkDestroyCommandPool(inDevice->logicalDevice().get(), getVkType<>(commandPool), nullptr);
        deleteIfNecessary(commandPool);
    }

    // Semaphores
    inline void deleteVkSemaphore_CQ(std::shared_ptr<Device> const &inDevice, VkSemaphore_CQ *inSemaphore) {
        vkDestroySemaphore(inDevice->logicalDevice().get(), getVkType<>(inSemaphore), nullptr);
        deleteIfNecessary(inSemaphore);
    }

    // Image View
    inline void deleteVkImageView_CQ(std::shared_ptr<Device> const &inDevice, VkImageView_CQ *imageView) {
        vkDestroyImageView(inDevice->logicalDevice().get(), getVkType<>(imageView), nullptr);
        deleteIfNecessary(imageView);
    }

    // Sampler
    inline void deleteVkSampler_CQ(std::shared_ptr<Device> const &inDevice, VkSampler_CQ *sampler) {
        vkDestroySampler(inDevice->logicalDevice().get(), getVkType<>(sampler), nullptr);
        deleteIfNecessary(sampler);
    }

    /**
     * Call used to allocate a debug report callback so that you can get error
     * messages from Vulkan. This Vulkan function is from an extension, so you
     * need to get the function pointer to make the call.
     */
    VkResult CreateDebugReportCallbackEXT(VkInstance instance,
                                          const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator,
                                          VkDebugReportCallbackEXT *pCallback) {
        auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(
                instance, "vkCreateDebugReportCallbackEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pCallback);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void Instance::createSurface() {
        VkAndroidSurfaceCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
        createInfo.window = m_window.get();

        VkSurfaceKHR surfaceRaw;
        if (vkCreateAndroidSurfaceKHR(m_instance.get(), &createInfo, nullptr, &surfaceRaw) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }

        // the surface requires the window, so, pass it into the deleter.
        std::shared_ptr<VkInstance_T> const &capInstance = m_instance;
        std::shared_ptr<WindowType> const &capWindow = m_window;
        auto deleter = [capInstance, capWindow](VkSurfaceKHR_CQ *surfaceRaw) {
            deleteVkSurfaceKHR_CQ(capInstance, surfaceRaw);
        };

        m_surface.reset(createVkSurfaceKHR_CQ(surfaceRaw), deleter);
    }

    void Instance::setupDebugCallback() {
        if (!enableValidationLayers) return;

        VkDebugReportCallbackCreateInfoEXT createInfo = {};
        createInfo.sType =
                VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.flags =
                VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        createInfo.pfnCallback = debugCallback;

        VkDebugReportCallbackEXT callbackRaw;
        if (createDebugReportCallbackEXT(&createInfo, nullptr, &callbackRaw) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug callback!");
        }

        auto &capInstance = m_instance;
        auto capDestroyDebugCallback = destroyDebugReportCallbackEXT;
        auto deleter = [capInstance, capDestroyDebugCallback](VkDebugReportCallbackEXT_CQ *callbackRaw) {
            deleteVkDebugReportCallbackEXT_CQ(capInstance, capDestroyDebugCallback, callbackRaw);
        };

        m_callback.reset(createVkDebugReportCallbackEXT_CQ(callbackRaw), deleter);
    }

    void Instance::createInstance() {
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "AmazingLabyrinth";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        if (!checkExtensionSupport()) {
            throw std::runtime_error(std::string("failed to find extension: "));
        }

        if (enableValidationLayers) {
            if (!checkValidationLayerSupport()) {
                throw std::runtime_error(std::string("failed to find validation layers: "));
            }
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkInstance instanceRaw;
        if (vkCreateInstance(&createInfo, nullptr, &instanceRaw) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }

        auto deleter = [](VkInstance instanceRaw) {
            vkDestroyInstance(instanceRaw, nullptr);
        };

        m_instance.reset(instanceRaw, deleter);
    }

    bool Instance::checkExtensionSupport() {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
        std::cout << "list of extensions supported:\n";
        for (const auto &item : extensions) {
            std::cout << "\t" << item.extensionName << "\n";
        }

        std::cout << "\nneeded extensions:\n";
        for (const auto &extension : getRequiredExtensions()) {
            std::cout << "\t" << extension << "\n";
            bool found = false;
            for (const auto &item : extensions) {
                if (strcmp(item.extensionName, extension) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return false;
            }
        }

        return true;
    }

    bool Instance::checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        std::cout << "\nlist of validation layers supported:\n";
        for (const auto &item : availableLayers) {
            std::cout << "\t" << item.layerName << "\n";
        }

        std::cout << "\nneeded validation layers:\n";
        for (const auto &layerName : validationLayers) {
            std::cout << "\t" << layerName << "\n";
            bool layerFound = false;

            for (const auto &layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    std::vector<const char *> Instance::getRequiredExtensions() {
        std::vector<const char *> extensions;

        extensions.push_back("VK_KHR_surface");
        extensions.push_back("VK_KHR_android_surface");

        /* required to get debug messages from Vulkan */
        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }

        return extensions;
    }

/**
 * Call used to allocate a debug report callback so that you can get error
 * messages from Vulkan. This Vulkan function is from an extension, so you
 * need to get the function pointer to make the call.
 */
    VkResult Instance::createDebugReportCallbackEXT(
            const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
            const VkAllocationCallbacks *pAllocator,
            VkDebugReportCallbackEXT *pCallback) {
        auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(
                m_instance.get(), "vkCreateDebugReportCallbackEXT");
        if (func != nullptr) {
            return func(m_instance.get(), pCreateInfo, pAllocator, pCallback);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

/**
 * Destroy the debug report callback created in CreateDebugReportCallbackEXT
 */
    void Instance::destroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                 const VkAllocationCallbacks *pAllocator) {
        auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(
                instance, "vkDestroyDebugReportCallbackEXT");
        if (func != nullptr) {
            func(instance, callback, pAllocator);
        }
    }

    /* find supported image formats for depth buffering */
    VkFormat Device::findSupportedFormat(const std::vector<VkFormat> &candidates,
                                         VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice(), format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR &&
                (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                       (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

    uint32_t Device::findMemoryType(uint32_t typeFilter,
                                    VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    Device::QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                                 nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                                 queueFamilies.data());

        int i = 0;
        for (const auto &queueFamily : queueFamilies) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i,
                                                 getVkType<>(m_instance->surface().get()),
                                                 &presentSupport);
            if (queueFamily.queueCount > 0 && presentSupport) {
                indices.presentFamily = i;
            }

            if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                indices.graphicsFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    Device::SwapChainSupportDetails Device::querySwapChainSupport(VkPhysicalDevice device) {
        Device::SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device,
                                                  getVkType<>(m_instance->surface().get()),
                                                  &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(
                device,
                getVkType<>(m_instance->surface().get()),
                &formatCount,
                nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                    device,
                    getVkType<>(m_instance->surface().get()),
                    &formatCount,
                    details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, getVkType<>(m_instance->surface().get()),
                                                  &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, getVkType<>(m_instance->surface().get()),
                                                      &presentModeCount,
                                                      details.presentModes.data());
        }

        return details;
    }

    void Device::pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_instance->instance().get(), &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_instance->instance().get(), &deviceCount, devices.data());
        for (const auto &device : devices) {
            if (isDeviceSuitable(device)) {
                m_physicalDevice = device;
                break;
            }
        }

        // debug
        for (const auto &device : devices) {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);

            std::cout << "Device name: " << deviceProperties.deviceName << "\n";
        }

        if (m_physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    Device::DeviceProperties Device::properties() {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(m_physicalDevice, &deviceProperties);

        return DeviceProperties(deviceProperties.deviceName, deviceProperties.apiVersion);
    }

    void Device::createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies();
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<int> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

        float queuePriority = 1.0f;
        for (auto queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        VkDevice logicalDeviceRaw;
        if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &logicalDeviceRaw) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        auto deleter = [](VkDevice logicalDeviceRaw) {
            vkDestroyDevice(logicalDeviceRaw, nullptr);
        };

        m_logicalDevice.reset(logicalDeviceRaw, deleter);

        vkGetDeviceQueue(m_logicalDevice.get(), indices.graphicsFamily, 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_logicalDevice.get(), indices.presentFamily, 0, &m_presentQueue);
    }

    bool Device::isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device);
        bool extensionsSupported = checkDeviceExtensionSupport(device);

        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate =
                    !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        if (deviceFeatures.samplerAnisotropy &&
            indices.isComplete() &&
            extensionsSupported &&
            swapChainAdequate) {
            std::cout << "Picking device: " << deviceProperties.deviceName << "\n";
            return true;
        } else {
            return false;
        }
    }

    bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                             availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto &extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    void Device::createAllocator() {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = m_physicalDevice;
        allocatorInfo.device = m_logicalDevice.get();
        allocatorInfo.instance = m_instance->instance().get();
        VmaVulkanFunctions vulkanFunctions = {};
        vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
        allocatorInfo.pVulkanFunctions = &vulkanFunctions;

        auto deleter = [](VmaAllocator allocatorRaw) -> void {
            vmaDestroyAllocator(allocatorRaw);
        };

        VmaAllocator allocatorRaw;
        VkResult result = vmaCreateAllocator(&allocatorInfo, &allocatorRaw);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create allocator.");
        }
        m_allocator = std::shared_ptr<VmaAllocator_T>(allocatorRaw, deleter);
    }

    /**
     * create the swap chain.
     */
    void SwapChain::createSwapChain(uint32_t width, uint32_t height) {
        /* chose details of the swap chain and get information about what is supported */
        Device::SwapChainSupportDetails swapChainSupport = m_device->querySwapChainSupport();

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, width, height);

        /* Decide on the number of images in the swap chain.  The implementation specifies the
         * minimum amount, but we try to have more than that to implement triple buffering
         * properly.
         */
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        /* set the create structure up. */
        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = getVkType<>(m_device->instance()->surface().get());
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        /* Prefer exclusive ownership (images are owned by just one queue).  But if the present
         * queue is different from the graphics queue (not usually the case), use the concurrent
         * mode to avoid having to transfer ownership. (too hard for right now)
         */
        Device::QueueFamilyIndices indices = m_device->findQueueFamilies();
        uint32_t queueFamilyIndices[] = {(uint32_t) indices.graphicsFamily,
                                         (uint32_t) indices.presentFamily};
        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        /* Specify the transform to perform (like 90 degree clockwise rotation).  For no transform
         * set this to current transform.
         */
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        m_preTransform = swapChainSupport.capabilities.currentTransform;

        /*
         * specifies if the alpha channel should be used for blending with other windows in the
         * window system.  Ignore the alpha channel.
         */
        //createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        // Todo: what should really go here.
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;

        /* set the chosen present mode */
        createInfo.presentMode = presentMode;

        /* enable clipping - this says that we don't care about the color of the pixels that
         * are obsured, (e.g. another window is in front of them).
         */
        createInfo.clipped = VK_TRUE;

        /*
         * oldSwapchain is for recreating the swap chain if it becomes invalid or unoptimized.
         * if recreating the swap chain, you need to pass the old one in this field.  For now,
         * we'll assume that we'll only ever create one swap chain.
         */
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        VkSwapchainKHR swapChainRaw;
        if (vkCreateSwapchainKHR(m_device->logicalDevice().get(), &createInfo, nullptr, &swapChainRaw) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        auto const &capDevice = m_device;
        auto deleter = [capDevice](VkSwapchainKHR_CQ *swapChainRaw) {
            deleteVkSwapchainKHR_CQ(capDevice, swapChainRaw);
        };

        m_swapChain.reset(createVkSwapchainKHR_CQ(swapChainRaw), deleter);

        m_imageFormat = surfaceFormat.format;
        m_extent = extent;
    }

    /**
     * Choose the image format.  We want SRGB color space and RGB format.
     */
    VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
            return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        }

        for (const auto &availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];

    }

    /**
     * Choose the swap change present mode.
     *
     * We prefer VK_PRESENT_MODE_MAILBOX_KHR because it uses triple
     * buffering and avoids tearing.  If we can't get that, we look for
     * VK_PRESENT_MODE_IMMEDIATE_KHR next because although
     * VK_PRESENT_MODE_FIFO_KHR is guaranteed to be available, not all video
     * cards implement it correctly.
     */
    VkPresentModeKHR
    SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) {
        VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

        for (const auto &availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            } else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                bestMode = availablePresentMode;
            }
        }

        return bestMode;
    }

    /**
     * Choose the resolution of the swap images in the frame buffer.  If we were passed in a
     * value, try to use it, otherwise just return the current extent.
     */
    VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities,
            uint32_t width, uint32_t height)
    {
        if (width != 0 && height != 0) {
            if (capabilities.minImageExtent.height < height && capabilities.maxImageExtent.height > height &&
                capabilities.minImageExtent.width < width && capabilities.maxImageExtent.width > width) {
                VkExtent2D extent = { width, height};
                return extent;
            }
        }

        return capabilities.currentExtent;
    }

    void RenderPass::createRenderPass(std::shared_ptr<SwapChain> const &swapchain) {
        /* color buffer attachment descriptions: use a single attachment represented by
         * one of the images from the swap chain.
         */
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = swapchain->imageFormat();
        /* stick to one sample since we are not using multisampling */
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        /* clear the contents of the attachment to a constant at the start */
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        /* store the rendered contents in memory so they can be read later */
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        /* we don't care about the stencil buffer for this app */
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        /* we don't care which layout the image was in */
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        /* images to be presented in the swap chain */
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        m_colorAttachmentFormats.push_back(colorAttachment.format);

        /* subpasses and attachment references:
         * a render pass may consist of many subpasses. For example, post processing tasks.
         */
        VkAttachmentReference colorAttachmentRef = {};
        /* specify which attachment by its index in the attachment descriptions array */
        colorAttachmentRef.attachment = 0;

        /* specify the layout to use for the attachment during a subpass that uses this reference
         * VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL gives the best performance for an attachment
         * functioning as a color buffer
         */
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        /* depth attachment */
        VkAttachmentDescription depthAttachment = {};
        depthAttachment.format = m_device->depthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

        /* we won't be using the depth data after the drawing has finished, so use DONT_CARE for
         * store operation
         */
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        /* dont care about the previous contents */
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        m_hasDepthAttachment = true;
        m_depthAttachmentFormat = depthAttachment.format;

        VkAttachmentReference depthAttachmentRef = {};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        /* render subpass */
        VkSubpassDescription subpass = {};
        /* specify a graphics subpass (as opposed to a compute subpass) */
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        /* create a render subpass dependency because we need the render pass to wait for the
         * VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage of the graphics pipeline
         */
        VkSubpassDependency dependency = {};

        /* The following two fields specify the indices of the dependency and the dependent
         * subpass. The special value VK_SUBPASS_EXTERNAL refers to the implicit subpass before
         * or after the render pass depending on whether it is specified in srcSubpass or
         * dstSubpass. The index 0 refers to our subpass, which is the first and only one. The
         * dstSubpass must always be higher than srcSubpass to prevent cycles in the
         * dependency graph.
         */
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;

        /* wait for the VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage */
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;

        /* prevent the transition from happening until when we want to start writing colors to
         * the color attachment.
         */
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask =
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        /* create the render pass */
        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VkRenderPass renderPassRaw;
        if (vkCreateRenderPass(m_device->logicalDevice().get(), &renderPassInfo, nullptr, &renderPassRaw) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }

        auto const &capDevice = m_device;
        auto deleter = [capDevice](VkRenderPass_CQ *renderPassCq) {
            deleteVkRenderPass_CQ(capDevice, renderPassCq);
        };

        m_renderPass.reset(createVkRenderPass_CQ(renderPassRaw), deleter);
    }

    void RenderPass::createRenderPassDepthTexture(
        std::vector<ImageAttachmentInfo> const &colorInfos,
        std::shared_ptr<ImageAttachmentInfo> const &depthInfo)
    {
        /* color buffer attachment descriptions: use a single attachment represented by
         * one of the images from the swap chain.
         */

        int i = 0;
        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkAttachmentReference> colorAttachmentRefs;
        for (auto const &colorInfo : colorInfos) {
            VkAttachmentDescription colorAttachment = {};
            colorAttachment.format = colorInfo.format;
            /* stick to one sample since we are not using multisampling */
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            /* clear the contents of the attachment to a constant at the start */
            colorAttachment.loadOp = colorInfo.loadOp;
            /* store the rendered contents in memory so they can be read later */
            colorAttachment.storeOp = colorInfo.storeOp;
            /* we don't care about the stencil buffer for this app */
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            /* we don't care which layout the image was in */
            colorAttachment.initialLayout = colorInfo.initialLayout;

            /* images to be presented in the swap chain */
            colorAttachment.finalLayout = colorInfo.finalLayout;

            m_colorAttachmentFormats.push_back(colorInfo.format);

            attachments.push_back(colorAttachment);

            /* subpasses and attachment references:
             * a render pass may consist of many subpasses. For example, post processing tasks.
             */
            VkAttachmentReference colorAttachmentRef = {};
            /* specify which attachment by its index in the attachment descriptions array */
            colorAttachmentRef.attachment = i;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachmentRefs.push_back(colorAttachmentRef);
            i++; // todo: get rid of i, use colorAttachmentRefs.size() instead
        }

        /* depth attachment */
        VkAttachmentDescription depthAttachment = {};
        VkAttachmentReference depthAttachmentRef = {};
        if (depthInfo) {
            depthAttachment.format = depthInfo->format;
            depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp = depthInfo->loadOp;

            depthAttachment.storeOp = depthInfo->storeOp;

            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            depthAttachment.initialLayout = depthInfo->initialLayout;
            depthAttachment.finalLayout = depthInfo->finalLayout;

            attachments.push_back(depthAttachment);

            depthAttachmentRef.attachment = i;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            m_hasDepthAttachment = true;
            m_depthAttachmentFormat = depthAttachment.format;
        }

        /* render subpass */
        VkSubpassDescription subpass = {};
        /* specify a graphics subpass (as opposed to a compute subpass) */
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = colorAttachmentRefs.size();
        subpass.pColorAttachments = colorAttachmentRefs.data();
        if (depthInfo) {
            subpass.pDepthStencilAttachment = &depthAttachmentRef;
        }

        /* create a render subbass dependency because we need the render pass to wait for the
         * VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage of the graphics pipeline
         */
        VkSubpassDependency dependency = {};

        /* The following two fields specify the indices of the dependency and the dependent
         * subpass. The special value VK_SUBPASS_EXTERNAL refers to the implicit subpass before
         * or after the render pass depending on whether it is specified in srcSubpass or
         * dstSubpass. The index 0 refers to our subpass, which is the first and only one. The
         * dstSubpass must always be higher than srcSubpass to prevent cycles in the
         * dependency graph.
         */
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;

        /* wait for the VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage */
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;

        /* prevent the transition from happening until when we want to start writing colors to
         * the color attachment.
         */
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask =
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        //std::array<VkAttachmentDescription, 2> attachments{ colorAttachment, depthAttachment };

        /* create the render pass */
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = attachments.size();
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VkRenderPass renderPassRaw;
        if (vkCreateRenderPass(m_device->logicalDevice().get(), &renderPassInfo, nullptr, &renderPassRaw) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }

        auto const &capDevice = m_device;
        auto deleter = [capDevice](VkRenderPass_CQ *renderPassCq) {
            deleteVkRenderPass_CQ(capDevice, renderPassCq);
        };

        m_renderPass.reset(createVkRenderPass_CQ(renderPassRaw), deleter);
    }

    void Shader::createShaderModule(std::shared_ptr<FileRequester> const &requester,
            std::string const &codeFile) {
        auto code = readFile(requester, codeFile);

        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();

        /* vector.data is 32 bit aligned as is required. */
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shaderModuleRaw;
        if (vkCreateShaderModule(m_device->logicalDevice().get(), &createInfo, nullptr, &shaderModuleRaw) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        auto const &capDevice = m_device;
        auto deleter = [capDevice](VkShaderModule_CQ *shaderModule) {
            deleteVkShaderModule_CQ(capDevice, shaderModule);
        };

        m_shaderModule.reset(createVkShaderModule_CQ(shaderModuleRaw), deleter);
    }

    void Pipeline::createGraphicsPipeline(
        std::shared_ptr<FileRequester> const &requester,
        VkVertexInputBindingDescription const &bindingDescription,
        std::vector<VkVertexInputAttributeDescription> const &attributeDescriptions,
        std::string const vertShader,
        std::string const fragShader,
        std::shared_ptr<Pipeline> const &derivedPipeline,
        VkCullModeFlags cullMode)
    {
        std::shared_ptr<Shader> vertShaderModule;
        std::shared_ptr<Shader> fragShaderModule;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

        if (!vertShader.empty()) {
            /* assign shaders to stages in the graphics pipeline */
            vertShaderModule = std::make_shared<Shader>(requester, m_device, vertShader);
            VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
            vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertShaderStageInfo.module = getVkType<>(vertShaderModule->shader().get());
            vertShaderStageInfo.pName = "main";
            /* can also use pSpecializationInfo to set constants used by the shader.  This allows
             * the usage of one shader module to be configured in different ways at pipeline creation,
             * and still allows the shader byte code compiler to eliminate if statements dependent on
             * the constants.
             */
            vertShaderStageInfo.pSpecializationInfo = nullptr;
            shaderStages.push_back(vertShaderStageInfo);
        }

        if (!fragShader.empty()) {
            fragShaderModule = std::make_shared<Shader>(requester, m_device, fragShader);
            VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
            fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragShaderStageInfo.module = getVkType<>(fragShaderModule->shader().get());
            fragShaderStageInfo.pName = "main";
            fragShaderStageInfo.pSpecializationInfo = nullptr;
            shaderStages.push_back(fragShaderStageInfo);
        }

        /* specify the input for the vertex shader */
        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        /* specify the type of thing to draw: points, lines, triangles, line strip, or triangle
         * strip.  Strips continue the line using the last vertex of the last line as the first
         * vertex of the next line or the last two vertices from the previous triangle as the
         * first two vertices for the next triangle.  primitiveRestartEnable lets you break a
         * strip in the middle by using a special index of 0xFFFF or 0xFFFFFFFF
         */
        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        /* use the full framebuffer to output the image to */
        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) m_extent.width;
        viewport.height = (float) m_extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        /* any pixels outside the scissor rectangle will be cut by the rasterizer. draw to the
         * entire framebuffer.
         */
        VkRect2D scissor = {};
        scissor.offset = {0, 0};
        scissor.extent = m_extent;

        /* can specify multiple viewports and scissors here */
        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;

        /* if VK_TRUE, then geometry never passes through the rasterizer stage. This basically
         * disables output to the framebuffer.
         */
        rasterizer.rasterizerDiscardEnable = VK_FALSE;

        /* can also just draw the lines or points with VK_POLYGON_MODE_LINE or
         * VK_POLYGON_MODE_POINT respectively.  Using a mode other than fill requires a GPU feature.
         */
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

        /* how wide the line is.  How thin you can make it depends on hardware and if
         * thicker than 1.0 is required, need to enable wideLines GPU feature.
         */
        rasterizer.lineWidth = 1.0f;

        /* can disable culling, cull front and/or back faces */
        rasterizer.cullMode = cullMode;

        /* specifies the vertex order for faces to be considered front facing */
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        /* can alter depth values by adding constant or biasing based off fragment slope,
         * we don't use it
         */
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;
        rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = 0.0f;

        /* multisampling: one of the ways to perform anti-aliasing.  Works by combining the
         * fragment shader reasults of multiple polygons that rasterize to the same pixel.
         * Need to enable a GPU feature to use it.
         */
        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        /* color blending: after a fragment shader has returned a color it needs to be combined
         * with the color that is already in the framebuffer. Can either mix the old and new value
         * or combine old and new value using a bitwise operation.
         */

        /* per attached framebuffer color blending information */
        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;

        for (size_t i = 0 ; i < m_renderPass->numberColorAttachments(); i++) {
            VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
            colorBlendAttachment.colorWriteMask =
                    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

            /* alpha blending: new color should be blended with the old color based on its opacity */
            colorBlendAttachment.blendEnable = VK_TRUE;
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

            /* differs from what they said to do in the Vulkan tutorial.  We want both the destination and
             * the source alpha channel to be considered when choosing the final alpha channel value in case
             * the top surface is see through and we need to see the bottom one through it.
             */
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_MAX;

            colorBlendAttachments.push_back(colorBlendAttachment);
        }

        /* color blending for all the framebuffers and allows you to set blend constants used
         * as blend factors in the per framebuffer color blending operations
         */
        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional

        colorBlending.attachmentCount = colorBlendAttachments.size();
        colorBlending.pAttachments = colorBlendAttachments.data();
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        /* depth buffering */
        VkPipelineDepthStencilStateCreateInfo depthStencil = {};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;

        /* We want to keep the fragments whose depth is less so, use compare
         * op: VK_COMPARE_OP_LESS
         */
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

        /* depth bounds test allows you to only keep fragments that fall in the specified range */
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional

        /* for stencil buffer operations */
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional

        /* dynamic state: if you want to change the viewport size, line width or blend constants
         * without recreating the graphics pipeline, then use the below struct when creating the
         * pipeline (otherwise specify nullptr for it.  If dynamic state info is used, then you
         * have to specify this info at drawing time - the configuration of these will be ignored.
        VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_LINE_WIDTH
        };

        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;
         */

        /* pipeline layout: used to pass uniform values to shaders at drawing time (like the
         * transformation matrix
         */
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        /* the descriptor set layout for the MVP matrix */
        pipelineLayoutInfo.setLayoutCount = 1;
        VkDescriptorSetLayout descriptorSetLayout = getVkType<>(m_descriptorPools->descriptorSetLayout().get());
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        VkPipelineLayout pipelineLayoutRaw;
        if (vkCreatePipelineLayout(m_device->logicalDevice().get(), &pipelineLayoutInfo, nullptr,
                                   &pipelineLayoutRaw) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        auto const &capDevice = m_device;
        auto layoutDeleter = [capDevice](VkPipelineLayout_CQ *pipelineLayoutRaw) {
            deleteVkPipelineLayout_CQ(capDevice, pipelineLayoutRaw);
        };

        m_pipelineLayout.reset(createVkPipelineLayout_CQ(pipelineLayoutRaw), layoutDeleter);

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = getVkType<>(m_pipelineLayout.get());
        pipelineInfo.renderPass = getVkType<>(m_renderPass->renderPass().get());
        pipelineInfo.subpass = 0; // index of the subpass

        /* if you want to create a pipeline from an already existing pipeline use these.
         * It is less expensive to switch between pipelines derived from each other
         */
        if (derivedPipeline == nullptr) {
            pipelineInfo.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        } else {
            pipelineInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
            pipelineInfo.basePipelineHandle = getVkType<>(derivedPipeline->pipeline().get());
        }

        // Can use index or handle to refer to the base pipeline.  We use the handle so, set this to -1.
        pipelineInfo.basePipelineIndex = -1;

        VkPipeline pipelineRaw;
        if (vkCreateGraphicsPipelines(m_device->logicalDevice().get(), VK_NULL_HANDLE/*pipeline cache*/, 1,
                                      &pipelineInfo, nullptr, &pipelineRaw) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        auto pipelineDeleter = [capDevice](VkPipeline_CQ *pipelineRaw) {
            deleteVkPipeline_CQ(capDevice, pipelineRaw);
        };

        m_pipeline.reset(createVkPipeline_CQ(pipelineRaw), pipelineDeleter);
    }

/* command pools are used to retrieve command buffers.  Command buffers is where the drawing
 * commands are written.
 */
    void CommandPool::createCommandPool() {
        Device::QueueFamilyIndices queueFamilyIndices = m_device->findQueueFamilies();
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

        /* each command pool can only allocate command buffers that are submitted on a single type
         * of queue.  Select the graphics queue family for drawing.
         */
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

        /* possible flags:
         *      VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded
         * with new commands very often (may change memory allocation behavior)
         *      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command buffers to be
         * rerecorded individually, without this flag they all have to be reset together
         */
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VkCommandPool commandsRaw;
        if (vkCreateCommandPool(m_device->logicalDevice().get(), &poolInfo, nullptr, &commandsRaw) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }

        auto const &capDevice = m_device;
        auto deleter = [capDevice](VkCommandPool_CQ *commandsRaw) {
            deleteVkCommandPool_CQ(capDevice, commandsRaw);
        };

        m_commandPool.reset(createVkCommandPool_CQ(commandsRaw), deleter);
    }

    void CommandBuffer::create() {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = getVkType<>(m_pool->commandPool().get());
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBufferRaw;
        VkResult result = vkAllocateCommandBuffers(m_device->logicalDevice().get(), &allocInfo,
                                                   &commandBufferRaw);
        if (result != VK_SUCCESS) {
            throw (std::runtime_error("Failed to allocate command buffer"));
        }

        auto const &capDevice = m_device;
        auto const &capCommandPool = m_pool;
        auto deleter = [capDevice, capCommandPool](VkCommandBuffer commandBufferRaw) {
            vkFreeCommandBuffers(capDevice->logicalDevice().get(),
                                 getVkType<>(capCommandPool->commandPool().get()), 1, &commandBufferRaw);
        };

        m_commandBuffer.reset(commandBufferRaw, deleter);
    }

    void CommandBuffer::begin() {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = m_usageFlag;

        vkBeginCommandBuffer(m_commandBuffer.get(), &beginInfo);
    }

    void CommandBuffer::end(
        Semaphore &waitSemaphore,
        VkPipelineStageFlags pipelineStage,
        Semaphore &signalSemaphore)
    {
        vkEndCommandBuffer(m_commandBuffer.get());

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;

        VkSemaphore waitSemaphores[] = {getVkType<>(waitSemaphore.semaphore().get())};
        VkPipelineStageFlags waitStages[] = {pipelineStage};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        VkSemaphore signalSemaphores[] = {getVkType<>(signalSemaphore.semaphore().get())};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        // command is expecting an array of command buffers.  commandBufferRaw is passed in
        // as const and will not be modified.
        VkCommandBuffer commandBufferRaw = m_commandBuffer.get();
        submitInfo.pCommandBuffers = &commandBufferRaw;

        VkResult result = vkQueueSubmit(m_device->graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to submit command buffer!");
        }
    }

    void CommandBuffer::end(
        Semaphore &signalSemaphore)
    {
        vkEndCommandBuffer(m_commandBuffer.get());

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;

        VkSemaphore signalSemaphores[] = {getVkType<>(signalSemaphore.semaphore().get())};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        // command is expecting an array of command buffers.  commandBufferRaw is passed in
        // as const and will not be modified.
        VkCommandBuffer commandBufferRaw = m_commandBuffer.get();
        submitInfo.pCommandBuffers = &commandBufferRaw;

        VkResult result = vkQueueSubmit(m_device->graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to submit command buffer!");
        }
    }

    void CommandBuffer::end() {
        vkEndCommandBuffer(m_commandBuffer.get());

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;

        // command is expecting an array of command buffers.  commandBufferRaw is passed in
        // as const and will not be modified.
        VkCommandBuffer commandBufferRaw = m_commandBuffer.get();
        submitInfo.pCommandBuffers = &commandBufferRaw;

        VkResult result = vkQueueSubmit(m_device->graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to submit command buffer!");
        }

        /* wait for the command to be done. */
        result = vkQueueWaitIdle(m_device->graphicsQueue());
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to wait on graphics queue!");
        }
    }

    void Buffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                              VkMemoryPropertyFlags propertyFlags) {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;

        /* this buffer will be used as a vertex buffer */
        bufferInfo.usage = usage;

        /* the buffer will only be used by the graphics queue, so use exclusive sharing mode */
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.requiredFlags = propertyFlags;

        VkResult result = vmaCreateBuffer(m_device->allocator().get(), &bufferInfo, &allocInfo, &m_buffer, &m_allocation, nullptr);
        if (result != VK_SUCCESS) {
            throw (std::runtime_error("Failed to bind buffer to memory!"));
        }
    }

    /* copy the data from CPU readable memory in the graphics card to non-CPU readable memory */
    void Buffer::copyTo(std::shared_ptr<CommandPool> pool, Buffer const &srcBuffer,
                        VkDeviceSize size) {
        CommandBuffer cmds(m_device, pool);
        cmds.begin();

        VkBufferCopy copyRegion = {};
        copyRegion.size = size;
        vkCmdCopyBuffer(cmds.commandBuffer().get(), srcBuffer.m_buffer,
                        m_buffer, 1, &copyRegion);

        cmds.end();
    }

    /* copy data from CPU memory to graphics memory */
    void Buffer::copyRawTo(void const *dataRaw, size_t size) {
        void *data;
        VkResult result = vmaMapMemory(m_device->allocator().get(), m_allocation, &data);
        if (result != VK_SUCCESS) {
            throw (std::runtime_error("Can't map memory."));
        }
        memcpy(data, dataRaw, size);
        vmaUnmapMemory(m_device->allocator().get(), m_allocation);
    }

    /* copy data from graphics memory to CPU memory */
    void Buffer::copyRawFrom(void *dataRaw, size_t size) const {
        void *data;
        VkResult result = vmaMapMemory(m_device->allocator().get(), m_allocation, &data);
        if (result != VK_SUCCESS) {
            throw (std::runtime_error("Can't map memory."));
        }
        memcpy(dataRaw, data, size);
        vmaUnmapMemory(m_device->allocator().get(), m_allocation);
    }

    void Semaphore::createSemaphore() {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkSemaphore semaphoreRaw;
        if (vkCreateSemaphore(m_device->logicalDevice().get(), &semaphoreInfo, nullptr, &semaphoreRaw) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphores!");
        }

        auto const &capDevice = m_device;
        auto deleter = [capDevice](VkSemaphore_CQ *semaphoreRaw) {
            deleteVkSemaphore_CQ(capDevice, semaphoreRaw);
        };

        m_semaphore.reset(createVkSemaphore_CQ(semaphoreRaw), deleter);
    }

    void Image::createImage(VkFormat format, VkImageTiling tiling,
                            VkImageUsageFlags usage, VkMemoryPropertyFlags properties) {

        /* copy the data to an image object because it will be easier and faster to access the
         * image from the shader.  One advantage of using an image object is that using one will
         * allowy us to use 2D coordinates.  Pixels in an image object are known as texels.
         */
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = m_width;
        imageInfo.extent.height = m_height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;

        /* must be the same format as the pixels in the buffer otherwise the copy will fail */
        imageInfo.format = format;

        /* texels are layed out in an implementation defined order for optimal access.  If we
         * wanted it to be in a row-major order, then we should use VK_IMAGE_TILING_LINEAR
         */
        imageInfo.tiling = tiling;

        /* the first transition will discard the texels that were already there */
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        /* the image will be used as the destination for the image copy from the buffer, so use
         * VK_IMAGE_USAGE_TRANSFER_DST_BIT. The image also needs to be accessable from the shader
         * to color the mesh, so use VK_IMAGE_USAGE_SAMPLED_BIT.
         */
        imageInfo.usage = usage;

        /* only accessing the image from the graphic queue which also supports transfer
         * operations.
         */
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        /* the samples flag is for multisampling.  This is only relevant for images that will
         * be used as an attachment, so stick to one sample.
         */
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        /* for sparse images.  We don't use it here. */
        imageInfo.flags = 0; // Optional


        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.requiredFlags = properties;

        VkResult result = vmaCreateImage(m_device->allocator().get(), &imageInfo, &allocInfo,
                &m_image, &m_allocation, nullptr);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Could not allocate image!");
        }
    }

    /* get the image in the right layout before we execute a copy command */
    void Image::transitionImageLayout(VkImageLayout oldLayout,
                                      VkImageLayout newLayout, std::shared_ptr<CommandPool> const &pool) {
        CommandBuffer cmds{m_device, pool};
        cmds.begin();

        /* use an image barrier to transition the layout */
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;

        /* for transfer of queue family ownership.  we are not transfering queue family ownership
         * so set these to VK_QUEUE_FAMILY_IGNORED.
         */
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        /* define the image that is affected and the part of the image.  our image is not an array
         * and does not have mipmapping levels.  Only specify one level and layer.
         */
        barrier.image = m_image;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (hasStencilComponent(m_format)) {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        } else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;


        /* There are two transitions that we handle:
         *
         * undefined -> transfer destination: transfer writes don't need to wait on anything
         *
         * transfer destination -> shader reading: fragment shader reads need to wait on
         *      transfer writes
         */
        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
            newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                   newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
                   newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
/*        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&  // TODO: condition can be removed when testing done
                   newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; */
        } else {
            throw std::runtime_error("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
                cmds.commandBuffer().get(),
                sourceStage, destinationStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
        );

        cmds.end();
    }

    void Image::copyBufferToImage(Buffer &buffer, std::shared_ptr<CommandPool> const &pool) {
        CommandBuffer cmds{m_device, pool};
        cmds.begin();

        VkBufferImageCopy region = {};

        /* offset in the buffer where the image starts. */
        region.bufferOffset = 0;

        /* specifies how the pixels are layed out in memory.  We could have some padding between
         * the rows.  But we don't in our case, so set both below to 0.
         */
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        /* indicate what part of the image we want to copy */
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        /* indicate what part of the image we want to copy */
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
                m_width,
                m_height,
                1
        };

        vkCmdCopyBufferToImage(cmds.commandBuffer().get(), buffer.buffer(),
                               m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        cmds.end();
    }

    void Image::copyImageToBuffer(Buffer &buffer, std::shared_ptr<CommandPool> const &pool) {
        CommandBuffer cmds{m_device, pool};
        cmds.begin();

        VkBufferImageCopy region = {};

        /* offset in the buffer where the image starts. */
        region.bufferOffset = 0;

        /* specifies how the pixels are layed out in memory.  We could have some padding between
         * the rows.  But we don't in our case, so set both below to 0.
         */
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        /* indicate what part of the image we want to copy */
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        /* indicate what part of the image we want to copy */
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
                m_width,
                m_height,
                1
        };

        vkCmdCopyImageToBuffer(cmds.commandBuffer().get(),
                               m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               buffer.buffer(), 1, &region);

        cmds.end();
    }

    void ImageView::createImageView(VkFormat format, VkImageAspectFlags aspectFlags) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_image->image();

        /* specify how the image data should be interpreted. viewType allows you
         * to treat images as 1D textures, 2D textures 3D textures and cube maps.
         */
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format;

        /* components enables swizzling the color channels around.  Can map to other channels
         * or use the constant values of 0 and 1.  Use the default mapping...
         */
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        /* subresourcesRange describes the image's purpose and which part of the image
         * should be accessed.  Use the images as color targets without any mimapping levels
         * or multiple layers.
         */
        createInfo.subresourceRange.aspectMask = aspectFlags;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        /* create the image view: destroy when done. */
        VkImageView imageViewRaw;
        if (vkCreateImageView(logicalDevice(), &createInfo, nullptr, &imageViewRaw) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create image views");
        }

        auto const &capImage = m_image;
        auto deleter = [capImage] (VkImageView_CQ *imageViewRaw) {
            deleteVkImageView_CQ(capImage->device(), imageViewRaw);
        };

        m_imageView.reset(createVkImageView_CQ(imageViewRaw), deleter);
    }

    std::shared_ptr<Image> ImageFactory::createTextureImage(std::shared_ptr<Device> const &device,
                                                            std::shared_ptr<CommandPool> const &cmdPool,
                                                            std::vector<char> const &pixels,
                                                            uint32_t texWidth,
                                                            uint32_t texHeight,
                                                            uint32_t texChannels)
    {
        VkDeviceSize imageSize = texWidth * texHeight * texChannels;

        /* copy the image to CPU accessable memory in the graphics card.  Make sure that it has the
         * VK_BUFFER_USAGE_TRANSFER_SRC_BIT set so that we can copy from it to an image later
         */
        Buffer staging(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        staging.copyRawTo(pixels.data(), static_cast<size_t>(imageSize));

        VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;//VK_FORMAT_R8_UNORM;
        auto image = std::make_shared<Image>(device, texWidth, texHeight, format,
                          VK_IMAGE_TILING_OPTIMAL,
                          VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        /* transition the image to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL. The image was created with
         * layout: VK_IMAGE_LAYOUT_UNDEFINED, so we use that to specify the old layout.
         */
        image->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED,
                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmdPool);

        image->copyBufferToImage(staging, cmdPool);

        /* transition the image to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL so that the
         * shader can read from it.
         */
        image->transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cmdPool);
        return image;
    }

    void ImageSampler::createTextureSampler(
        VkSamplerAddressMode beyondBorderSampling,
        VkBorderColor borderColor)
    {
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        /* algorithm to use in the case of oversampling - more texels than fragments */
        samplerInfo.magFilter = VK_FILTER_LINEAR;

        /* algorithm to use in the case of undersampling - fewer texels than fragments */
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        /* Specify what to do when sampling beyond the image border.  Modes listed below:
         *
         * VK_SAMPLER_ADDRESS_MODE_REPEAT: Repeat the texture when going beyond the image dimensions.
         * VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: Like repeat, but inverts the coordinates
         *      to mirror the image when going beyond the dimensions.
         * VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE: Take the color of the edge closest to the
         *      coordinate beyond the image dimensions.
         * VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE: Like clamp to edge, but instead uses
         *      the edge opposite to the closest edge.
         * VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER: Return a solid color when sampling beyond
         *      the dimensions of the image.
         */
        samplerInfo.addressModeU = beyondBorderSampling;
        samplerInfo.addressModeV = beyondBorderSampling;
        samplerInfo.addressModeW = beyondBorderSampling;

        /* use anisotropic filtering.  limit the amount of texel samples that will be used to
         * calculate the final color to 16.
         */
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 16;

        /* color to return when sampling beyond the image border and clamp to border is used.
         * Only black, white, or transparent can be specified in float or int formats, no other
         * colors.
         */
        samplerInfo.borderColor = borderColor;

        /* normalized coordinate system: the texels are addressed in the [0, 1) range on all axises.
         * unnormalized coordinates: the texels are addressed with the ranges: [0, texWidth)
         * and [0, texHeight).  Most real world applications use normalized coordinates.
         */
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        /* comparison function.  Texels will be first compared to a value and the result of the
         * comparison will be used in filtering operations.
         */
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        /* for mipmapping */
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        VkSampler textureSamplerRaw;
        if (vkCreateSampler(m_device->logicalDevice().get(), &samplerInfo, nullptr, &textureSamplerRaw) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }

        auto const &capDevice = m_device;
        auto deleter = [capDevice](VkSampler_CQ *textureSamplerRaw) {
            deleteVkSampler_CQ(capDevice, textureSamplerRaw);
        };

        m_sampler.reset(createVkSampler_CQ(textureSamplerRaw), deleter);
    }

    void SwapChainCommands::createFramebuffers(std::shared_ptr<RenderPass> &renderPass,
                                               std::shared_ptr<ImageView> &depthImage) {
        for (size_t i = 0; i < m_imageViews.size(); i++) {
            std::vector<VkImageView> attachments = {
                    getVkType<>(m_imageViews[i].imageView().get()),
                    getVkType<>(depthImage->imageView().get())
            };
            m_framebuffers.push_back(Framebuffer::createRawFramebuffer(m_swapChain->device(),
                    renderPass, attachments, m_swapChain->extent().width, m_swapChain->extent().height));
        }
    }

    /* Allocate and record commands for each swap chain immage */
    void SwapChainCommands::createCommandBuffers() {
        m_commandBuffers.resize(m_framebuffers.size());

        /* allocate the command buffer from the command pool, freed by Vulkan when the command
         * pool is freed
         */
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = getVkType<>(m_pool->commandPool().get());
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) m_commandBuffers.size();

        if (vkAllocateCommandBuffers(m_swapChain->device()->logicalDevice().get(), &allocInfo,
                                     m_commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void copyIndicesToBuffer(std::shared_ptr<vulkan::CommandPool> const &cmdpool,
                             std::vector<uint32_t> const &indices,
                             Buffer &buffer)
    {
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        vulkan::Buffer stagingBuffer(cmdpool->device(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        stagingBuffer.copyRawTo(indices.data(), bufferSize);

        buffer.copyTo(cmdpool, stagingBuffer, bufferSize);
    }
} /* namespace vulkan */
