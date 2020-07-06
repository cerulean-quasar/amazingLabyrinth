/**
 * Copyright 2020 Cerulean Quasar. All Rights Reserved.
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
#ifndef AMAZING_LABYRINTH_TEXTURE_TABLE_VULKAN_HPP
#define AMAZING_LABYRINTH_TEXTURE_TABLE_VULKAN_HPP

#include <vector>
#include <map>
#include <memory>

#include "../../graphicsVulkan.hpp"
#include "textureLoader.hpp"
#include "textureTable.hpp"

namespace levelDrawer {
    class TextureDataVulkan : public TextureData {
    public:
        TextureDataVulkan(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<vulkan::Device> const &inDevice,
                std::shared_ptr<vulkan::CommandPool> const &inCommandPool,
                std::shared_ptr<TextureDescription> const &inTextureDescription) {
            uint32_t texWidth{};
            uint32_t texHeight{};
            uint32_t texChannels{};

            std::vector<char> pixels = inTextureDescription->getData(gameRequester, texWidth,
                                                                     texHeight, texChannels);

            m_sampler = std::make_shared<vulkan::ImageSampler>(inDevice, inCommandPool, pixels,
                                                               texWidth, texHeight, texChannels);
        }

        inline std::shared_ptr<vulkan::ImageSampler> const &sampler() { return m_sampler; }

    private:
        std::shared_ptr<vulkan::ImageSampler> m_sampler;
    };

    class TextureTableVulkan : public TextureTable<TextureDataVulkan> {
    public:
        TextureTableVulkan(
                std::shared_ptr<vulkan::Device> inDevice,
                std::shared_ptr<vulkan::CommandPool> inCommandPool)
                : TextureTable{},
                  m_device{inDevice},
                  m_commandPool{inCommandPool} {}

        ~TextureTableVulkan() override = default;

    protected:
        std::shared_ptr<TextureDataVulkan> getTextureData(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<TextureDescription> const &textureDescription) override {
            return std::make_shared<TextureDataVulkan>(gameRequester, m_device, m_commandPool,
                                                       textureDescription);
        }

    private:
        std::shared_ptr<vulkan::Device> m_device;
        std::shared_ptr<vulkan::CommandPool> m_commandPool;
    };
}

#endif // AMAZING_LABYRINTH_TEXTURE_TABLE_VULKAN_HPP
