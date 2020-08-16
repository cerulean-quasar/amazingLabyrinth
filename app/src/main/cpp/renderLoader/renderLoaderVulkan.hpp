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

#include "../graphicsVulkan.hpp"

#include "../renderDetails/renderDetails.hpp"
#include "renderLoader.hpp"
#include "../renderDetails/renderDetailsVulkan.hpp"
#include "registerVulkan.hpp"

struct RenderLoaderVulkanTraits {
    using RenderDetailsParametersType = renderDetails::ParametersVulkan;
    using RenderDetailsType = renderDetails::RenderDetailsVulkan;
    using CommonObjectDataType = renderDetails::CommonObjectData;
    using RenderDetailsReferenceType = renderDetails::ReferenceVulkan;
    using RetrieveFcns = RenderDetailsVulkanRetrieveFcns;
    using SurfaceDetailsType = vulkan::SurfaceDetails;
    static RenderDetailsVulkanRetrieveMap &getRenderDetailsMap() {
        return getRenderDetailsVulkanMap();
    }
};

class RenderLoaderVulkan : public std::enable_shared_from_this<RenderLoaderVulkan>, public RenderLoader<RenderLoaderVulkanTraits> {
public:
    RenderLoaderVulkan(std::shared_ptr<vulkan::Device> inDevice)
        : RenderLoader<RenderLoaderVulkanTraits>(),
        m_device{std::move(inDevice)}
    {}
    ~RenderLoaderVulkan() override = default;

protected:
    RenderLoaderVulkanTraits::RenderDetailsReferenceType loadNew(
        RenderLoaderVulkanTraits::RetrieveFcns const &fcns,
        std::shared_ptr<GameRequester> const &gameRequester,
        std::shared_ptr<RenderLoaderVulkanTraits::SurfaceDetailsType> const &surfaceDetails,
        std::shared_ptr<renderDetails::Parameters> const &parameters) override
    {
        return fcns.renderDetailsLoadNewFcn(gameRequester, shared_from_this(), m_device, surfaceDetails, parameters);
    }

    void reload(
        std::shared_ptr<GameRequester> const &gameRequester,
        std::shared_ptr<RenderLoaderVulkanTraits::RenderDetailsType> const &renderDetails,
        std::shared_ptr<RenderLoaderVulkanTraits::SurfaceDetailsType> const &surfaceDetails) override
    {
        vkDeviceWaitIdle(m_device->logicalDevice().get());
        renderDetails->reload(gameRequester, shared_from_this(), surfaceDetails);
    }

    RenderLoaderVulkanTraits::RenderDetailsReferenceType loadExisting(
        RenderLoaderVulkanTraits::RetrieveFcns const &fcns,
        std::shared_ptr<GameRequester> const &gameRequester,
        std::shared_ptr<RenderLoaderVulkanTraits::RenderDetailsType> const &renderDetails,
        std::shared_ptr<RenderLoaderVulkanTraits::SurfaceDetailsType> const &surfaceDetails,
        std::shared_ptr<renderDetails::Parameters> const &parameters) override
    {
        return fcns.renderDetailsLoadExistingFcn(gameRequester, shared_from_this(), renderDetails,
                surfaceDetails, parameters);
    }
private:
    std::shared_ptr<vulkan::Device> m_device;
};

#endif // AMAZING_LABYRINTH_RENDER_LOADER_VULKAN_HPP
