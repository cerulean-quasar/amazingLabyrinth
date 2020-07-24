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
#ifndef AMAZING_LABYRINTH_RENDER_LOADER_REGISTER_VULKAN_HPP
#define AMAZING_LABYRINTH_RENDER_LOADER_REGISTER_VULKAN_HPP

#include <memory>
#include <map>
#include <string>
#include <functional>

#include "../graphicsVulkan.hpp"
#include "../renderDetails/renderDetailsVulkan.hpp"

// Register RenderDetailsVulkan types
struct RenderDetailsVulkanRetrieveFcns {
    using RenderDetailsReferenceVulkan = renderDetails::ReferenceVulkan;
    using RenderDetailsLoadNewFcn = std::function<renderDetails::ReferenceVulkan(
        std::shared_ptr<GameRequester> const &,
        std::shared_ptr<RenderLoaderVulkan> const &,
        std::shared_ptr<vulkan::Device>,
        std::shared_ptr<renderDetails::Parameters> const &)>;

    using RenderDetailsLoadExistingFcn = std::function<renderDetails::ReferenceVulkan(
        std::shared_ptr<GameRequester> const &,
        std::shared_ptr<RenderLoaderVulkan> const &,
        std::shared_ptr<renderDetails::RenderDetailsVulkan> const &,
        std::shared_ptr<renderDetails::Parameters> const &)>;

    RenderDetailsLoadNewFcn renderDetailsLoadNewFcn;
    RenderDetailsLoadExistingFcn renderDetailsLoadExistingFcn;
};

using RenderDetailsVulkanRetrieveMap =
    std::map<std::string, std::function<RenderDetailsVulkanRetrieveFcns()>>;

RenderDetailsVulkanRetrieveMap &getRenderDetailsVulkanMap();

class RenderLoaderVulkan;

template <typename RenderDetailsBaseType, typename RenderDetailsType, typename ConfigType>
class RegisterVulkan {
    RegisterVulkan() {
        getRenderDetailsVulkanMap().emplace(
            RenderDetailsType::name(),
            std::function<RenderDetailsVulkanRetrieveFcns()> (
                []() -> RenderDetailsVulkanRetrieveFcns {
                    RenderDetailsVulkanRetrieveFcns fcns;
                    ConfigType config;
                    fcns.renderDetailsLoadNewFcn = RenderDetailsVulkanRetrieveFcns::RenderDetailsLoadNewFcn (
                            [config] (std::shared_ptr<GameRequester> const &gameRequester,
                                      std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
                                      std::shared_ptr<vulkan::Device> inDevice,
                                      std::shared_ptr<renderDetails::Parameters> const &parameters) -> RenderDetailsVulkanRetrieveFcns::RenderDetailsReferenceVulkan
                            {
                                return RenderDetailsType::loadNew(gameRequester, renderLoader,
                                        inDevice, parameters, config);
                            });
                    fcns.renderDetailsLoadExistingFcn = RenderDetailsVulkanRetrieveFcns::RenderDetailsLoadExistingFcn (
                            [config] (std::shared_ptr<GameRequester> const &gameRequester,
                                      std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
                                      std::shared_ptr<RenderDetailsBaseType> const &renderDetails,
                                      std::shared_ptr<renderDetails::Parameters> const &parameters) -> RenderDetailsVulkanRetrieveFcns::RenderDetailsReferenceVulkan
                            {
                                return RenderDetailsType::loadExisting(gameRequester, renderLoader,
                                        renderDetails, parameters, config);
                            });
                    return std::move(fcns);
                }
            )
        );
    }
};

#endif // AMAZING_LABYRINTH_RENDER_LOADER_REGISTER_VULKAN_HPP
