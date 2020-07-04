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
#ifndef AMAZING_LABYRINTH_RENDER_LOADER_VULKAN_HPP
#define AMAZING_LABYRINTH_RENDER_LOADER_VULKAN_HPP

#include <memory>
#include <list>

#include "../graphicsVulkan.hpp"

#include "../renderDetails/basic/renderDetailsData.hpp"
#include "renderLoader.hpp"

class RenderLoaderVulkan : public RenderLoader<renderDetails::RenderDetailsVulkan> {
public:
    ~RenderLoaderVulkan() override = default;

protected:
    std::shared_ptr<renderDetails::RenderDetailsVulkan> loadNew(std::string const &name,
            uint32_t width, uint32_t height)
    {
        auto loaderFcnIt = getRenderDetailsVulkanMap().find(name);
        if (loaderFcnIt == getRenderDetailsVulkanMap().end()) {
            throw std::runtime_error("RenderDetails not registered.");
        }

        auto renderDetails = loaderFcnIt->second(m_device);
    }

    void reload(
            std::shared_ptr<renderDetails::RenderDetailsVulkan> const &renderDetails,
            uint32_t width,
            uint32_t height) override
    {
        renderDetails.reload(width, height);
    }

private:
    std::shared_ptr<vulkan::Device> m_device;
};

#endif // AMAZING_LABYRINTH_RENDER_LOADER_HPP
