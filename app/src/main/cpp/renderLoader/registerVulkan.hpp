/**
 * Copyright 2024 Cerulean Quasar. All Rights Reserved.
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
#ifndef AMAZING_LABYRINTH_RENDER_LOADER_REGISTER_VULKAN_HPP
#define AMAZING_LABYRINTH_RENDER_LOADER_REGISTER_VULKAN_HPP

#include <memory>
#include <map>
#include <string>
#include <functional>

#include "../graphicsVulkan.hpp"
#include "../renderDetails/renderDetailsVulkan.hpp"
#include "../graphicsVulkan.hpp"

// Register RenderDetailsVulkan types
struct RenderDetailsVulkanRetrieveFcns {
    using RenderDetailsReferenceVulkan = renderDetails::ReferenceVulkan;
    using RenderDetailsLoadNewFcn = std::function<renderDetails::ReferenceVulkan(
        std::shared_ptr<GameRequester> const &,
        std::shared_ptr<RenderLoaderVulkan> const &,
        std::shared_ptr<vulkan::Device>,
        std::shared_ptr<vulkan::SurfaceDetails> const &,
        std::shared_ptr<renderDetails::ParametersBase> const &)>;

    using RenderDetailsLoadExistingFcn = std::function<renderDetails::ReferenceVulkan(
        std::shared_ptr<GameRequester> const &,
        std::shared_ptr<RenderLoaderVulkan> const &,
        std::shared_ptr<renderDetails::RenderDetailsVulkan> const &,
        std::shared_ptr<vulkan::SurfaceDetails> const &,
        std::shared_ptr<renderDetails::ParametersBase> const &)>;

    RenderDetailsLoadNewFcn renderDetailsLoadNewFcn;
    RenderDetailsLoadExistingFcn renderDetailsLoadExistingFcn;
};

struct RenderDetailsVulkanEntry {
    renderDetails::Description description;
    std::function<RenderDetailsVulkanRetrieveFcns()> getFunctions;
    std::shared_ptr<renderDetails::RenderDetailsVulkan> cacheEntry;

    RenderDetailsVulkanEntry(
            renderDetails::Description inDescription,
            std::function<RenderDetailsVulkanRetrieveFcns()> fcns,
            std::shared_ptr<renderDetails::RenderDetailsVulkan> inCacheEntry)
    {
        description = std::move(inDescription);
        getFunctions = std::move(fcns);
        cacheEntry = std::move(inCacheEntry);
    }
};

using RenderDetailsVulkanRegistrar = std::vector<RenderDetailsVulkanEntry>;

RenderDetailsVulkanRegistrar &getRenderDetailsVulkanRegistrar();

class RenderLoaderVulkan;

template <typename RenderDetailsBaseType, typename RenderDetailsType>
class RegisterVulkan {
public:
    RegisterVulkan(
            renderDetails::Description const &description,
            std::vector<char const*> shaders) {
        getRenderDetailsVulkanRegistrar().emplace_back(
            description,
            std::function<RenderDetailsVulkanRetrieveFcns()> (
                [description, shaders]() -> RenderDetailsVulkanRetrieveFcns {
                    RenderDetailsVulkanRetrieveFcns fcns;
                    fcns.renderDetailsLoadNewFcn = RenderDetailsVulkanRetrieveFcns::RenderDetailsLoadNewFcn (
                            [description, shaders] (std::shared_ptr<GameRequester> const &gameRequester,
                                      std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
                                      std::shared_ptr<vulkan::Device> const &inDevice,
                                      std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails,
                                      std::shared_ptr<renderDetails::ParametersBase> const &parameters) -> RenderDetailsVulkanRetrieveFcns::RenderDetailsReferenceVulkan
                            {
                                return RenderDetailsType::loadNew(description, shaders, gameRequester, renderLoader,
                                        inDevice, surfaceDetails, parameters);
                            });
                    fcns.renderDetailsLoadExistingFcn = RenderDetailsVulkanRetrieveFcns::RenderDetailsLoadExistingFcn (
                            [] (std::shared_ptr<GameRequester> const &gameRequester,
                                      std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
                                      std::shared_ptr<RenderDetailsBaseType> const &renderDetails,
                                      std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails,
                                      std::shared_ptr<renderDetails::ParametersBase> const &parameters) -> RenderDetailsVulkanRetrieveFcns::RenderDetailsReferenceVulkan
                            {
                                return RenderDetailsType::loadExisting(gameRequester, renderLoader,
                                        renderDetails, surfaceDetails, parameters);
                            });
                    return std::move(fcns);
                }
            ),
            nullptr
        );
    }
};

#endif // AMAZING_LABYRINTH_RENDER_LOADER_REGISTER_VULKAN_HPP
