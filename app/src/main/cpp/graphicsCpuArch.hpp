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
#ifndef AMAZING_LABYRINTH_GRAPHICS_CPU_ARCH_HPP
#define AMAZING_LABYRINTH_GRAPHICS_CPU_ARCH_HPP

#if (UINTPTR_MAX == 0xffffffffffffffffULL)
#define CQ_64_BIT
#else
#undef CQ_64_BIT
#endif

#ifdef CQ_64_BIT
#define CQ_DEFINE_VULKAN_HANDLE(object) \
    using object ## _CQ = object ## _T;

#define CQ_DEFINE_VULKAN_CREATOR(object) \
inline object ## _CQ *create ## object ## _CQ(object obj) { \
    return reinterpret_cast<object ## _CQ *>(obj); \
}

#else
#define CQ_DEFINE_VULKAN_HANDLE(object) \
    struct object ## _CQ {              \
        using object_type = object;     \
        object handle; \
        inline object getCqVkHandle() const { return handle; } \
    };

#define CQ_DEFINE_VULKAN_CREATOR(object) \
inline object ## _CQ *create ## object ## _CQ(object obj) { \
    if (obj == VK_NULL_HANDLE) { \
        return nullptr; \
    } \
    auto *ret = new object ## _CQ; \
    ret->handle = obj; \
    return ret; \
}

#endif

// Hack to stop the compiler/IDE from complaining about how my statement is always true
// or always false depending on the platform I am on.
template <typename type1, typename type2>
inline bool cq_are_types_same() {
    return std::is_same<type1, type2>::value;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
template <typename type1>
inline void deleteIfNecessary(type1 var) {
#ifndef CQ_64_BIT
    delete var;
#endif
}
#pragma clang diagnostic pop

template <typename cqVkType>
inline auto getVkType(cqVkType in) {
    return in;
}

template <typename cqVkType>
inline typename cqVkType::object_type getVkType(cqVkType *in) {
    if (in == nullptr) {
        return VK_NULL_HANDLE;
    } else {
        return in->getCqVkHandle();
    }
}

CQ_DEFINE_VULKAN_HANDLE(VkSurfaceKHR)
CQ_DEFINE_VULKAN_HANDLE(VkDebugReportCallbackEXT)
CQ_DEFINE_VULKAN_HANDLE(VkSwapchainKHR)
CQ_DEFINE_VULKAN_HANDLE(VkRenderPass)
CQ_DEFINE_VULKAN_HANDLE(VkDescriptorSet)
CQ_DEFINE_VULKAN_HANDLE(VkDescriptorSetLayout)
CQ_DEFINE_VULKAN_HANDLE(VkDescriptorPool)
CQ_DEFINE_VULKAN_HANDLE(VkShaderModule)
CQ_DEFINE_VULKAN_HANDLE(VkPipelineLayout)
CQ_DEFINE_VULKAN_HANDLE(VkPipeline)
CQ_DEFINE_VULKAN_HANDLE(VkCommandPool)
CQ_DEFINE_VULKAN_HANDLE(VkSemaphore)
CQ_DEFINE_VULKAN_HANDLE(VkImageView)
CQ_DEFINE_VULKAN_HANDLE(VkSampler)
CQ_DEFINE_VULKAN_HANDLE(VkFramebuffer)

#endif
