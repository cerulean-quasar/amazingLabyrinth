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
#include <stb_image.h>

#include "graphicsVulkan.hpp"

std::string const SHADER_VERT_FILE("shaders/shader.vert.spv");
std::string const SHADER_FRAG_FILE("shaders/shader.frag.spv");

namespace vulkan {
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
        auto deleter = [capInstance, capWindow](VkSurfaceKHR surfaceRaw) {
            vkDestroySurfaceKHR(capInstance.get(), surfaceRaw, nullptr);
        };

        m_surface.reset(surfaceRaw, deleter);
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
        auto deleter = [capInstance, capDestroyDebugCallback](VkDebugReportCallbackEXT callbackRaw) {
            capDestroyDebugCallback(capInstance.get(), callbackRaw, nullptr);
        };

        m_callback.reset(callbackRaw, deleter);
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
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_instance->surface().get(),
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

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_instance->surface().get(),
                                                  &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_instance->surface().get(), &formatCount,
                                             nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_instance->surface().get(), &formatCount,
                                                 details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_instance->surface().get(),
                                                  &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_instance->surface().get(),
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
} /* namespace vulkan */

std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {};

    attributeDescriptions.resize(4);

    /* position */
    attributeDescriptions[0].binding = 0; /* binding description to use */
    attributeDescriptions[0].location = 0; /* matches the location in the vertex shader */
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    /* color */
    attributeDescriptions[1].binding = 0; /* binding description to use */
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    /* texture coordinate */
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    /* normal vector */
    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex, normal);
    return attributeDescriptions;
}

VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription = {};

    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);

    /* move to the next data entry after each vertex.  VK_VERTEX_INPUT_RATE_INSTANCE
     * moves to the next data entry after each instance, but we are not using instanced
     * rendering
     */
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

void GraphicsVulkan::init(WindowType *inWindow) {
    createSwapChain();
    createImageViews();
    createRenderPass();

    VkDescriptorSetLayout descriptorSetLayout;
    createDescriptorSetLayout(descriptorSetLayout);
    descriptorPools.setDescriptorSetLayout(descriptorSetLayout);

    createGraphicsPipeline();
    createCommandPool();

    levelTracker.setParameters(swapChainExtent.width, swapChainExtent.height);
    levelStarter = levelTracker.getLevelStarter();
    maze = levelTracker.getLevel();
    float x, y;
    maze->getLevelFinisherCenter(x, y);
    levelFinisher = levelTracker.getLevelFinisher(x,y);

    bool tc;
    maze->updateStaticDrawObjects(staticObjsData, texturesLevel);
    maze->updateDynamicDrawObjects(dynObjsData, texturesLevel, tc);
    addTextures(texturesLevel);

    levelStarter->updateStaticDrawObjects(levelStarterStaticObjsData, texturesLevelStarter);
    levelStarter->updateDynamicDrawObjects(levelStarterDynObjsData, texturesLevelStarter, tc);
    addTextures(texturesLevelStarter);

    createUniformBuffer(uniformBufferLighting, uniformBufferMemoryLighting);
    void* data;
    glm::vec3 lightingVector = maze->getLightingSource();
    vkMapMemory(m_device->logicalDevice().get(), uniformBufferMemoryLighting, 0, sizeof(lightingVector), 0, &data);
    memcpy(data, &lightingVector, sizeof(lightingVector));
    vkUnmapMemory(m_device->logicalDevice().get(), uniformBufferMemoryLighting);

    addObjects(staticObjsData, texturesLevel);
    addObjects(dynObjsData, texturesLevel);

    addObjects(levelStarterStaticObjsData, texturesLevelStarter);
    addObjects(levelStarterDynObjsData, texturesLevelStarter);

    createDepthResources();
    createFramebuffers();

    createCommandBuffers();
    createSemaphores();
}

void GraphicsVulkan::addTextures(TextureMap &textures) {
    for (TextureMap::iterator it = textures.begin(); it != textures.end(); it++) {
        if (it->second.get() == nullptr) {
            std::shared_ptr<TextureDataVulkan> texture(new TextureDataVulkan(m_device));
            createTextureImage(it->first.get(), texture->image, texture->memory);
            texture->imageView = createImageView(texture->image, VK_FORMAT_R8G8B8A8_UNORM,
                                                 VK_IMAGE_ASPECT_COLOR_BIT);
            createTextureSampler(texture->sampler);
            it->second = texture;
        }
    }
}

void GraphicsVulkan::addObjects(DrawObjectTable &objs, TextureMap &textures) {
    for (auto &&obj : objs) {
        addObject(obj, textures);
    }
}

void GraphicsVulkan::addObject(DrawObjectEntry &obj, TextureMap &textures) {
    std::shared_ptr<DrawObjectDataVulkan> objData(new DrawObjectDataVulkan(m_device));
    createVertexBuffer(obj.first->vertices, objData->vertexBuffer, objData->vertexBufferMemory);
    createIndexBuffer(obj.first->indices, objData->indexBuffer, objData->indexBufferMemory);

    obj.second = objData;

    addUniforms(obj, textures);
}

void GraphicsVulkan::addUniforms(DrawObjectEntry &obj, TextureMap &textures) {
    DrawObjectDataVulkan *data = dynamic_cast<DrawObjectDataVulkan*> (obj.second.get());
    if (obj.first->modelMatrices.size() == data->uniforms.size()) {
        return;
    }

    UniformBufferObject ubo = getViewPerspectiveMatrix();

    for (size_t i = data->uniforms.size(); i < obj.first->modelMatrices.size(); i++) {
        VkBuffer uniformBuffer;
        VkDeviceMemory uniformBufferMemory;

        ubo.model = obj.first->modelMatrices[i];
        createUniformBuffer(uniformBuffer, uniformBufferMemory);
        void *ptr;
        vkMapMemory(m_device->logicalDevice().get(), uniformBufferMemory, 0, sizeof(ubo), 0, &ptr);
        memcpy(ptr, &ubo, sizeof(ubo));
        vkUnmapMemory(m_device->logicalDevice().get(), uniformBufferMemory);

        TextureMap::iterator it = textures.find(obj.first->texture);
        if (it == textures.end()) {
            throw std::runtime_error("Could not find texture in texture map.");
        }
        VkDescriptorSet descriptorSet = descriptorPools.allocateDescriptor();
        TextureDataVulkan *textureData = static_cast<TextureDataVulkan*> (it->second.get());
        updateDescriptorSet(uniformBuffer, textureData->imageView, textureData->sampler,
                            uniformBufferLighting, descriptorSet);

        std::shared_ptr<UniformWrapper> uniform(new UniformWrapper(m_device, descriptorSet, uniformBuffer,
                                                                   uniformBufferMemory, &descriptorPools));
        data->uniforms.push_back(uniform);
    }
}

void GraphicsVulkan::cleanup() {
    cleanupSwapChain();

    // trigger destruction of the objects to draw here so that it happens in the correct order.
    staticObjsData.clear();
    dynObjsData.clear();
    levelFinisherObjsData.clear();
    texturesLevel.clear();
    texturesLevelFinisher.clear();
    texturesLevelStarter.clear();

    if (m_device->logicalDevice().get() != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device->logicalDevice().get(), uniformBufferLighting, nullptr);
        vkFreeMemory(m_device->logicalDevice().get(), uniformBufferMemoryLighting, nullptr);

        descriptorPools.destroyResources();
        vkDestroySemaphore(m_device->logicalDevice().get(), renderFinishedSemaphore, nullptr);
        vkDestroySemaphore(m_device->logicalDevice().get(), imageAvailableSemaphore, nullptr);
        vkDestroyCommandPool(m_device->logicalDevice().get(), commandPool, nullptr);
        vkDestroyDevice(m_device->logicalDevice().get(), nullptr);
    }

    m_instance.reset();
}

void GraphicsVulkan::recreateSwapChain() {
    vkDeviceWaitIdle(m_device->logicalDevice().get());

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createDepthResources();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandBuffers();
}

/* buffer for the MVP matrix - updated every frame so don't copy in the data here */
void GraphicsVulkan::createUniformBuffer(VkBuffer &uniformBuffer, VkDeviceMemory &uniformBufferMemory) {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffer, uniformBufferMemory);
}

void GraphicsVulkan::createVertexBuffer(std::vector<Vertex> const &vertices, VkBuffer &vertexBuffer, VkDeviceMemory &vertexBufferMemory) {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    /* use a staging buffer in the CPU accessable memory to copy the data into graphics card
     * memory.  Then use a copy command to copy the data into fast graphics card only memory.
     */
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(m_device->logicalDevice().get(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(m_device->logicalDevice().get(), stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(m_device->logicalDevice().get(), stagingBuffer, nullptr);
    vkFreeMemory(m_device->logicalDevice().get(), stagingBufferMemory, nullptr);
}

/* buffer for the indices - used to reference which vertex in the vertex array (by index) to
 * draw.  This way normally duplicated vertices would not need to be specified twice.
 * Only one index buffer per pipeline is allowed.  Put all dice in the same index buffer.
 */
void GraphicsVulkan::createIndexBuffer(std::vector<uint32_t> const &indices, VkBuffer &indexBuffer, VkDeviceMemory &indexBufferMemory) {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    char* data;
    vkMapMemory(m_device->logicalDevice().get(), stagingBufferMemory, 0, bufferSize, 0, (void**)&data);
    memcpy(data, indices.data(), (size_t) sizeof(indices[0]) * indices.size());
    vkUnmapMemory(m_device->logicalDevice().get(), stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(m_device->logicalDevice().get(), stagingBuffer, nullptr);
    vkFreeMemory(m_device->logicalDevice().get(), stagingBufferMemory, nullptr);
}

void GraphicsVulkan::updatePerspectiveMatrix() {
    maze->updatePerspectiveMatrix(swapChainExtent.width, swapChainExtent.height);
}

void GraphicsVulkan::cleanupSwapChain() {
    if (m_device->logicalDevice().get() == VK_NULL_HANDLE) {
        return;
    }
    for (auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(m_device->logicalDevice().get(), framebuffer, nullptr);
    }
    if (graphicsPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(m_device->logicalDevice().get(), graphicsPipeline, nullptr);
    }

    if (pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(m_device->logicalDevice().get(), pipelineLayout, nullptr);
    }

    if (renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(m_device->logicalDevice().get(), renderPass, nullptr);
    }

    if (depthImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(m_device->logicalDevice().get(), depthImageView, nullptr);
    }

    if (depthImage != VK_NULL_HANDLE) {
        vkDestroyImage(m_device->logicalDevice().get(), depthImage, nullptr);
    }

    if (depthImageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device->logicalDevice().get(), depthImageMemory, nullptr);
    }

    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(m_device->logicalDevice().get(), imageView, nullptr);
    }

    if (swapChain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_device->logicalDevice().get(), swapChain, nullptr);
    }
}

/**
 * Choose the image format.  We want SRGB color space and RGB format.
 */
VkSurfaceFormatKHR GraphicsVulkan::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
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
VkPresentModeKHR GraphicsVulkan::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) {
    VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        } else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            bestMode = availablePresentMode;
        }
    }

    return bestMode;
}

/**
 * create the swap chain.
 */
void GraphicsVulkan::createSwapChain() {
    /* chose details of the swap chain and get information about what is supported */
    vulkan::Device::SwapChainSupportDetails swapChainSupport = m_device->querySwapChainSupport();

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    /* Decide on the number of images in the swap chain.  The implementation specifies the
     * minimum amount, but we try to have more than that to implement triple buffering
     * properly.
     */
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    /* set the create structure up. */
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_instance->surface().get();
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
    vulkan::Device::QueueFamilyIndices indices = m_device->findQueueFamilies();
    uint32_t queueFamilyIndices[] = {(uint32_t) indices.graphicsFamily, (uint32_t)indices.presentFamily};
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

    /*
     * specifies if the alpha channel should be used for blending with other windows in the
     * window system.  Ignore the alpha channel.
     */
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

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

    if (vkCreateSwapchainKHR(m_device->logicalDevice().get(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    /* retrieve the image handles from the swap chain. - handles cleaned up by Vulkan
     * Note: Vulkan may have chosen to create more images than specified in minImageCount,
     * so we have to requery the image count here.
     */
    vkGetSwapchainImagesKHR(m_device->logicalDevice().get(), swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device->logicalDevice().get(), swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void GraphicsVulkan::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i=0; i < swapChainImages.size(); i++) {
        swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

VkShaderModule GraphicsVulkan::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();

    /* vector.data is 32 bit aligned as is required. */
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_device->logicalDevice().get(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

void GraphicsVulkan::createRenderPass() {
    /* color buffer attachment descriptions: use a single attachment represented by
     * one of the images from the swap chain.
     */
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapChainImageFormat;
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
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

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

    if (vkCreateRenderPass(m_device->logicalDevice().get(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void GraphicsVulkan::createGraphicsPipeline() {
    auto vertShaderCode = readFile(SHADER_VERT_FILE);
    auto fragShaderCode = readFile(SHADER_FRAG_FILE);

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    /* assign shaders to stages in the graphics pipeline */
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    /* can also use pSpecializationInfo to set constants used by the shader.  This allows
     * the usage of one shader module to be configured in different ways at pipeline creation,
     * and still allows the shader byte code compiler to eliminate if statements dependent on
     * the constants.
     */
    vertShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.pSpecializationInfo = nullptr;
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    /* specify the input for the vertex shader */
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = getBindingDescription();
    auto attributeDescriptions = getAttributeDescriptions();
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
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    /* any pixels outside the scissor rectangle will be cut by the rasterizer. draw to the
     * entire framebuffer.
     */
    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

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
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;

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
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
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

    /* color blending for all the framebuffers and allows you to set blend constants used
     * as blend factors in the per framebuffer color blending operations
     */
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
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
    VkDescriptorSetLayout descriptorSetLayout = descriptorPools.getDescriptorSetLayout();
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = 0; // Optional

    if (vkCreatePipelineLayout(m_device->logicalDevice().get(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0; // index of the subpass

    /* if you want to create a pipeline from an already existing pipeline use these.
     * It is less expensive to switch between piplines derrived from each other
     */
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(m_device->logicalDevice().get(), VK_NULL_HANDLE/*pipeline cache*/, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    /* clean up */
    vkDestroyShaderModule(m_device->logicalDevice().get(), vertShaderModule, nullptr);
    vkDestroyShaderModule(m_device->logicalDevice().get(), fragShaderModule, nullptr);
}

void GraphicsVulkan::createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            swapChainImageViews[i],
            depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device->logicalDevice().get(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void GraphicsVulkan::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;

    /* this buffer will be used as a vertex buffer */
    bufferInfo.usage = usage;

    /* the buffer will only be used by the graphics queue, so use exclusive sharing mode */
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_device->logicalDevice().get(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device->logicalDevice().get(), buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = m_device->findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_device->logicalDevice().get(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(m_device->logicalDevice().get(), buffer, bufferMemory, 0 /* offset into the memory */);
}

/* copy the data from CPU readable memory in the graphics card to non-CPU readable memory */
void GraphicsVulkan::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

VkCommandBuffer GraphicsVulkan::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device->logicalDevice().get(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void GraphicsVulkan::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_device->graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    /* wait for the command to be done. */
    vkQueueWaitIdle(m_device->graphicsQueue());

    vkFreeCommandBuffers(m_device->logicalDevice().get(), commandPool, 1, &commandBuffer);
}

/* command pools are used to retrieve command buffers.  Command buffers is where the drawing
 * commands are written.
 */
void GraphicsVulkan::createCommandPool() {
    vulkan::Device::QueueFamilyIndices queueFamilyIndices = m_device->findQueueFamilies();
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
    poolInfo.flags = 0;

    if (vkCreateCommandPool(m_device->logicalDevice().get(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

/* Allocate and record commands for each swap chain immage */
void GraphicsVulkan::createCommandBuffers() {
    commandBuffers.resize(swapChainFramebuffers.size());

    /* allocate the command buffer from the command pool, freed by Vulkan when the command
     * pool is freed
     */
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

    if (vkAllocateCommandBuffers(m_device->logicalDevice().get(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    /* begin recording commands into each comand buffer */
    for (size_t i = 0; i < commandBuffers.size(); i++) {
        initializeCommandBuffer(commandBuffers[i], i);
    }
}

void GraphicsVulkan::initializeCommandBuffer(VkCommandBuffer &commandBuffer, size_t index) {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    /* pInheritanceInfo is only used if flags includes:
     * VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
     * because that would mean that there is a secondary command buffer that will be used
     * in a single render pass
     */
    beginInfo.pInheritanceInfo = nullptr; // Optional

    /* this call will reset the command buffer.  its not possible to append commands at
     * a later time.
     */
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    /* begin the render pass: drawing starts here*/
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[index];
    /* size of the render area */
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    /* the color value to use when clearing the image with VK_ATTACHMENT_LOAD_OP_CLEAR,
     * using black with 0% opacity
     */
    std::array<VkClearValue, 2> clearValues = {};
    glm::vec4 bgColor = maze->getBackgroundColor();
    clearValues[0].color = {bgColor.r, bgColor.g, bgColor.b, bgColor.a};
    clearValues[1].depthStencil = {1.0, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    /* begin recording commands - start by beginning the render pass.
     * none of these functions returns an error (they return void).  There will be no error
     * handling until recording is done.
     */
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    /* bind the graphics pipeline to the command buffer, the second parameter tells Vulkan
     * that we are binding to a graphics pipeline.
     */
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    // the objects that stay static.
    initializeCommandBufferDrawObjects(commandBuffer, staticObjsData);

    // the objects that move.
    initializeCommandBufferDrawObjects(commandBuffer, dynObjsData);

    // the level starter
    if (levelStarter.get() != nullptr) {
        initializeCommandBufferDrawObjects(commandBuffer, levelStarterStaticObjsData);
        initializeCommandBufferDrawObjects(commandBuffer, levelStarterDynObjsData);
    }

    // the level finisher objects.
    if (maze->isFinished() || levelFinisher->isUnveiling()) {
        initializeCommandBufferDrawObjects(commandBuffer, levelFinisherObjsData);
    }

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void GraphicsVulkan::initializeCommandBufferDrawObjects(VkCommandBuffer &commandBuffer, DrawObjectTable const &objs) {
    VkDeviceSize offsets[1] = {0};

    for (auto &&obj : objs) {
        DrawObjectDataVulkan *objData = dynamic_cast<DrawObjectDataVulkan*> (obj.second.get());

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &(objData->vertexBuffer), offsets);
        vkCmdBindIndexBuffer(commandBuffer, objData->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        for (auto &&uniform : objData->uniforms) {
            /* The MVP matrix and texture samplers */
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipelineLayout, 0, 1, &(uniform->descriptorSet), 0, nullptr);

            /* indexed draw command:
             * parameter 1 - Command buffer for the draw command
             * parameter 2 - the number of indices (the vertex count)
             * parameter 3 - the instance count, use 1 because we are not using instanced rendering
             * parameter 4 - offset into the index buffer
             * parameter 5 - offset to add to the indices in the index buffer
             * parameter 6 - offset for instance rendering
             */
            vkCmdDrawIndexed(commandBuffer, obj.first->indices.size(), 1, 0, 0, 0);
        }
    }
}

void GraphicsVulkan::createSemaphores() {
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(m_device->logicalDevice().get(), &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(m_device->logicalDevice().get(), &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS) {

        throw std::runtime_error("failed to create semaphores!");
    }
}

void GraphicsVulkan::drawFrame() {
    /* update the app state here */

    /* wait for presentation to finish before drawing the next frame.  Avoids a memory leak */
    vkQueueWaitIdle(m_device->presentQueue());

    uint32_t imageIndex;
    /* the third parameter is a timeout indicating how much time in nanoseconds we want to
     * wait for the image to become available (std::numeric_limits<uint64_t>::max() disables it.
     * an error from this function does not necessarily mean that we need to terminate
     * the program
     */
    VkResult result = vkAcquireNextImageKHR(m_device->logicalDevice().get(), swapChain,
        std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE,
        &imageIndex);

    //if (maze->isFinished() || levelFinisher->isUnveiling() || texturesChanged) {
        // The user completed the maze or the textures changed.  If the maze is completed, we need
        // to display the level finished animation.  Since there are additional objects, we need to
        // rewrite the command buffer to display the new objects.  If the textures changed, then we
        // need to update the descriptor sets, so the command buffers need to be rewritten.
        initializeCommandBuffer(commandBuffers[imageIndex], imageIndex);
        texturesChanged = false;
    //}

    /* If the window surface is no longer compatible with the swap chain, then we need to
     * recreate the swap chain and let the next call draw the image.
     * VK_SUBOPTIMAL_KHR means that the swap chain can still be used to present to the surface
     * but it no longer matches the window surface exactly.
     */
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    /* wait for the semaphore before writing to the color attachment.  This means that we
     * could start executing the vertex shader before the image is available.
     */
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    /* use the command buffer that corresponds to the image we just acquired */
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    /* indicate which semaphore to signal when execution is done */
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
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
    VkSwapchainKHR swapChains[] = {swapChain};
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
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    result = vkQueueWaitIdle(m_device->presentQueue());
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to wait on present queue.");
    }
}


void GraphicsVulkan::createDepthResources() {
    VkFormat depthFormat = m_device->depthFormat();

    createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

bool GraphicsVulkan::hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

/* descriptor set for the MVP matrix and texture samplers */
void GraphicsVulkan::updateDescriptorSet(VkBuffer uniformBuffer, VkImageView imageView,
                                         VkSampler textureSampler, VkBuffer lightingSource,
                                         VkDescriptorSet &descriptorSet) {
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    std::array<VkWriteDescriptorSet, 3> descriptorWrites = {};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSet;

    /* must be the same as the binding in the vertex shader */
    descriptorWrites[0].dstBinding = 0;

    /* index into the array of descriptors */
    descriptorWrites[0].dstArrayElement = 0;

    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    /* how many array elements you want to update */
    descriptorWrites[0].descriptorCount = 1;

    /* which one of these pointers needs to be used depends on which descriptorType we are
     * using.  pBufferInfo is for buffer based data, pImageInfo is used for image data, and
     * pTexelBufferView is used for decriptors that refer to buffer views.
     */
    descriptorWrites[0].pBufferInfo = &bufferInfo;
    descriptorWrites[0].pImageInfo = nullptr; // Optional
    descriptorWrites[0].pTexelBufferView = nullptr; // Optional

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = imageView;
    imageInfo.sampler = textureSampler;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    VkDescriptorBufferInfo bufferLightingSource = {};
    bufferLightingSource.buffer = lightingSource;
    bufferLightingSource.offset = 0;
    bufferLightingSource.range = sizeof (glm::vec3);

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = descriptorSet;
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pBufferInfo = &bufferLightingSource;

    vkUpdateDescriptorSets(m_device->logicalDevice().get(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

/* for accessing data other than the vertices from the shaders */
void GraphicsVulkan::createDescriptorSetLayout(VkDescriptorSetLayout &descriptorSetLayout) {
    /* MVP matrix */
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;

    /* only accessing the MVP matrix from the vertex shader */
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    /* image sampler */
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding lightingSourceBinding = {};
    lightingSourceBinding.binding = 2;
    lightingSourceBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    lightingSourceBinding.descriptorCount = 1;
    lightingSourceBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    lightingSourceBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 3> bindings = {uboLayoutBinding, samplerLayoutBinding, lightingSourceBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_device->logicalDevice().get(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void GraphicsVulkan::createTextureImage(TextureDescription *texture, VkImage &textureImage, VkDeviceMemory &textureImageMemory) {
    uint32_t texHeight;
    uint32_t texWidth;
    uint32_t texChannels;

    std::vector<char> pixels = texture->getData(texWidth, texHeight, texChannels);

    VkDeviceSize imageSize = texWidth*texHeight*texChannels;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    /* copy the image to CPU accessable memory in the graphics card.  Make sure that it has the
     * VK_BUFFER_USAGE_TRANSFER_SRC_BIT set so that we can copy from it to an image later
     */
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_device->logicalDevice().get(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels.data(), static_cast<size_t>(imageSize));
    vkUnmapMemory(m_device->logicalDevice().get(), stagingBufferMemory);

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;//VK_FORMAT_R8_UNORM;
    createImage(texWidth, texHeight, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

    /* transition the image to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL. The image was created with
     * layout: VK_IMAGE_LAYOUT_UNDEFINED, so we use that to specify the old layout.
     */
    transitionImageLayout(textureImage,  format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    /* transition the image to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL so that the
     * shader can read from it.
     */
    transitionImageLayout(textureImage,  format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    /* free staging buffer */
    vkDestroyBuffer(m_device->logicalDevice().get(), stagingBuffer, nullptr);
    vkFreeMemory(m_device->logicalDevice().get(), stagingBufferMemory, nullptr);
}

void GraphicsVulkan::createTextureSampler(VkSampler &textureSampler) {
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
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    /* use anisotropic filtering.  limit the amount of texel samples that will be used to
     * calculate the final color to 16.
     */
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;

    /* color to return when sampling beyond the image border and clamp to border is used.
     * Only black, white, or transparent can be specified in float or int formats, no other
     * colors.
     */
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    /* normalized coordinate system: the texels are addressed in the [0, 1) range on all axes.
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

    if (vkCreateSampler(m_device->logicalDevice().get(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }

}

VkImageView GraphicsVulkan::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;

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
    VkImageView imageView;
    if (vkCreateImageView(m_device->logicalDevice().get(), &createInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image views");
    }

    return imageView;
}

void GraphicsVulkan::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {

    /* copy the data to an image object because it will be easier and faster to access the
     * image from the shader.  One advantage of using an image object is that using one will
     * allowy us to use 2D coordinates.  Pixels in an image object are known as texels.
     */
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
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

    if (vkCreateImage(m_device->logicalDevice().get(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device->logicalDevice().get(), image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = m_device->findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_device->logicalDevice().get(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(m_device->logicalDevice().get(), image, imageMemory, 0);
}

/* get the image in the right layout before we execute a copy command */
void GraphicsVulkan::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

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
    barrier.image = image;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (hasStencilComponent(format)) {
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
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    endSingleTimeCommands(commandBuffer);
}

void GraphicsVulkan::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

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
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    endSingleTimeCommands(commandBuffer);
}

/**
 * Choose the resolution of the swap images in the frame buffer.  Just return
 * the current extent (same resolution as the window).
 */
VkExtent2D GraphicsVulkan::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    return capabilities.currentExtent;
}

UniformBufferObject GraphicsVulkan::getViewPerspectiveMatrix() {
    UniformBufferObject ubo = {};
    ubo.proj = maze->getProjectionMatrix();
    /* GLM has the y axis inverted from Vulkan's perspective, invert the y-axis on the
     * projection matrix.
     */
    ubo.proj[1][1] *= -1;
    ubo.view = maze->getViewMatrix();
    return ubo;
}

bool GraphicsVulkan::updateData() {
    bool drawingNecessary = false;

    UniformBufferObject ubo = getViewPerspectiveMatrix();
    if (maze->isFinished() || levelFinisher->isUnveiling()) {
        if (levelFinisher->isUnveiling()) {
            if (levelFinisher->isDone()) {
                levelFinisherObjsData.clear();
                texturesLevelFinisher.clear();
                float x, y;
                maze->getLevelFinisherCenter(x, y);
                levelFinisher = levelTracker.getLevelFinisher(x, y);
                levelStarter->start();
                return false;
            }
        } else {
            if (levelFinisher->isDone()) {

                levelTracker.gotoNextLevel();

                levelStarter = levelTracker.getLevelStarter();
                initializeLevelData(levelStarter.get(), levelStarterStaticObjsData,
                                    levelStarterDynObjsData, texturesLevelStarter);

                maze = levelTracker.getLevel();
                initializeLevelData(maze.get(), staticObjsData, dynObjsData, texturesLevel);

                levelFinisher->unveilNewLevel();
                return false;
            }
        }

        drawingNecessary = levelFinisher->updateDrawObjects(levelFinisherObjsData,
                                                            texturesLevelFinisher, texturesChanged);
        if (!drawingNecessary) {
            return false;
        }

        if (texturesChanged) {
            addTextures(texturesLevelFinisher);
            for (auto &&obj : levelFinisherObjsData) {
                DrawObjectDataVulkan *objData = static_cast<DrawObjectDataVulkan *> (obj.second.get());
                if (objData != nullptr) {
                    objData->uniforms.clear();
                }
            }
        }

        for (auto &&objData : levelFinisherObjsData) {
            DrawObjectDataVulkan *data = static_cast<DrawObjectDataVulkan *> (objData.second.get());
            if (data == nullptr) {
                // a completely new entry
                addObject(objData, texturesLevelFinisher);
            } else if (data->uniforms.size() < objData.first->modelMatrices.size()) {
                // a new model matrix (new object but same texture, vertices and indices).
                addUniforms(objData, texturesLevelFinisher);
            } else if (data->uniforms.size() > objData.first->modelMatrices.size()) {
                // a model matrix got removed...
                data->uniforms.pop_back();
            } else {
                // just copy over the model matrices into the graphics card memory... they might
                // have changed.
                for (size_t j = 0; j < objData.first->modelMatrices.size(); j++) {
                    void *ptr;
                    ubo.model = objData.first->modelMatrices[j];
                    vkMapMemory(m_device->logicalDevice().get(), data->uniforms[j]->uniformBufferMemory, 0,
                                sizeof(ubo), 0, &ptr);
                    memcpy(ptr, &ubo, sizeof(ubo));
                    vkUnmapMemory(m_device->logicalDevice().get(), data->uniforms[j]->uniformBufferMemory);
                }
            }
        }
    } else if (levelStarter.get() != nullptr) {
        drawingNecessary = updateLevelData(levelStarter.get(), levelStarterDynObjsData, texturesLevelStarter);
        if (levelStarter->isFinished()) {
            levelStarter.reset();
            levelStarterDynObjsData.clear();
            levelStarterStaticObjsData.clear();
            texturesLevelStarter.clear();
            maze->start();
            return false;
        }
    } else {
        drawingNecessary = updateLevelData(maze.get(), dynObjsData, texturesLevel);
    }

    return drawingNecessary;
}

void GraphicsVulkan::initializeLevelData(Level *level, DrawObjectTable &staticObjsData,
                                         DrawObjectTable &dynObjsData, TextureMap &textures) {
    staticObjsData.clear();
    dynObjsData.clear();
    textures.clear();
    level->updateStaticDrawObjects(staticObjsData, textures);
    bool texturesChanged;
    level->updateDynamicDrawObjects(dynObjsData, textures, texturesChanged);
    addTextures(textures);
    addObjects(staticObjsData, textures);
    addObjects(dynObjsData, textures);
}

bool GraphicsVulkan::updateLevelData(Level *level, DrawObjectTable &objsData, TextureMap &textures) {
    UniformBufferObject ubo = getViewPerspectiveMatrix();
    bool drawingNecessary = level->updateData();

    if (!drawingNecessary) {
        return false;
    }

    level->updateDynamicDrawObjects(objsData, textures, texturesChanged);
    if (texturesChanged) {
        addTextures(textures);
        for (auto &&obj : objsData) {
            DrawObjectDataVulkan *objData = static_cast<DrawObjectDataVulkan *> (obj.second.get());
            objData->uniforms.clear();
            addUniforms(obj, textures);
        }
    } else {
        for (auto &&obj : objsData) {
            DrawObjectDataVulkan *objData = static_cast<DrawObjectDataVulkan *> (obj.second.get());
            for (size_t j = 0; j < obj.first->modelMatrices.size(); j++) {
                void *data;
                ubo.model = obj.first->modelMatrices[j];
                vkMapMemory(m_device->logicalDevice().get(), objData->uniforms[j]->uniformBufferMemory, 0,
                            sizeof(ubo), 0, &data);
                memcpy(data, &ubo, sizeof(ubo));
                vkUnmapMemory(m_device->logicalDevice().get(), objData->uniforms[j]->uniformBufferMemory);
            }
        }
    }
    return true;
}

void GraphicsVulkan::updateAcceleration(float x, float y, float z) {
    if (levelStarter.get() != nullptr) {
        levelStarter->updateAcceleration(x, y, z);
    } else {
        maze->updateAcceleration(x, y, z);
    }
}
