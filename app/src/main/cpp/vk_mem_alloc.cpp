/**
 * Copyright 2026 Cerulean Quasar. All Rights Reserved.
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

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnullability-completeness"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#define VMA_IMPLEMENTATION
/*
 * Needs to be set to Vulkan version: 1.0.0 for now
 * consider increasing when minSdk (in build.gradle - app) increases beyond 33.
 *
 * format for VMA_VULKAN_VERSION: AAABBBCCC:
 *    AAA = major version number (zero padded to 3 digits)
 *    BBB = minor version number (zero padded to 3 digits)
 *    CCC = patch version number (zero padded to 3 digits)
 */
#define VMA_VULKAN_VERSION 1000000
#include <vk_mem_alloc.h>
#pragma clang diagnostic pop
