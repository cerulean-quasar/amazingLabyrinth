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

#include "../graphicsVulkan.hpp"
#include "../renderDetails/renderDetailsVulkan.hpp"

// Register RenderDetailsVulkan types
struct RenderDetailsVulkanRetrieveFcns {
    using RenderDetailsFcn = std::function<std::shared_ptr<renderDetails::RenderDetailsVulkan>(
            std::shared_ptr<GameRequester> const &,
    std::shared_ptr<vulkan::Device>,
    renderDetails::RenderDetailsParametersVulkan const &);

    using CommonObjectDataFcn = std::function<std::shared_ptr<renderDetails::CommonObjectDataVulkan>(
            std::shared_ptr<renderDetails::RenderDetailsVulkan> const &,
    renderDetails::RenderDetailsParametersVulkan const &);

    RenderDetailsFcn renderDetailsLoadFcn;
    CommonObjectDataFcn commonObjectDataCreateFcn;
};

using RenderDetailsVulkanRetrieveMap =
    std::map<std::string, RenderDetailsRetrieveFcns<RenderDetailsLoadFcn, CommonObjectDataCreateFcn>>;

RenderDetailsVulkanRetrieveMap &getRenderDetailsVulkanMap() {
    static RenderDetailsVulkanRetrieveMap map{};

    return map;
}

template <typename RenderDetailsType, typename CommonObjectDataType, typename ConfigType, typename ParametersType>
class RegisterVulkan {
    RegisterVulkan() {
        getRenderDetailsVulkanMap().emplace(
            RenderDetailsType::name(),
            std::function<RenderDetailsVulkanRetrieveFcns()> (
                []() -> RenderDetailsVulkanRetrieveFcns {
                    RenderDetailsVulkanRetrieveFcns fcns;
                    ConfigType config;
                    fcns.renderDetailsLoadFcn = RenderDetailsVulkanRetrieveFcns::RenderDetailsFcn (
                            [] (std::shared_ptr<GameRequester> const &gameRequester,
                                    std::shared_ptr<vulkan::Device> inDevice,
                                    ParametersType const &parameters) {
                                return std::make_shared<RenderDetailsType>(gameRequester, inDevice, parameters);
                            });
                    fcns.commonObjectDataCreateFcn = RenderDetailsVulkanRetrieveFcns::CommonObjectDataFcn (
                            [config] (std::shared_ptr<RenderDetailsType> const &renderDetails,
                                    ParametersType const &parameters) {
                                return renderDetails->createCommonObjectData(parameters, config);
                            });
                    return std::move(fcns);
                }
            )
        )
    }
};

#endif // AMAZING_LABYRINTH_RENDER_LOADER_REGISTER_VULKAN_HPP