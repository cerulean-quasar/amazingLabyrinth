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

#include "../renderDetails/basic/renderDetailsData.hpp"
#include "renderLoader.hpp"
#include "../renderDetails/basic/renderDetailsVulkan.hpp"
#include "registerVulkan.hpp"

struct RenderLoaderVulkanTraits {
    using RenderDetailsType = renderDetails::RenderDetailsVulkan;
    using CommonObjectDataType = renderDetails::CommonObjectDataVulkan;
    using RenderDetailsReferenceType = renderDetails::RenderDetailsReference<RenderDetailsType, CommonObjectDataType>;
    using RenderDetailsParameterType = renderDetails::RenderDetailsParametersVulkan;
    using RetrieveFcns = RenderDetailsVulkanRetrieveFcns;
    RenderDetailsVulkanRetrieveMap (*getRenderDetailsMap)()
};

class RenderLoaderVulkan : public RenderLoader<RenderLoaderVulkanTraits> {
public:
    ~RenderLoaderVulkan() override = default;

protected:
    std::shared_ptr<RenderLoaderVulkanTraits::RenderDetailsReferenceType> loadNew(
        RenderLoaderVulkanTraits::RetrieveFcns const &fcns,
        std::shared_ptr<GameRequester> const &gameRequester,
        std::string const &name,
        RenderLoaderVulkanTraits::RenderDetailsParameterType const &parameters)
    {
        auto renderDetails = fcns.renderDetailsLoadFcn(gameRequester, m_device, parameters);
    }

    void reload(
        std::shared_ptr<GameRequester> const &gameRequester,
        std::shared_ptr<RenderLoaderVulkanTraits::RenderDetailsType> const &renderDetails,
        RenderLoaderVulkanTraits::RenderDetailsParameterType const &parameters) override
    {
        renderDetails.reload(gameRequester, parameters);
    }

    std::shared_ptr<RenderLoaderVulkanTraits::CommonObjectDataType> allocateCommonObjectData(
        RenderLoaderVulkanTraits::RetrieveFcns const &fcns,
        std::shared_ptr<RenderLoaderVulkanTraits::RenderDetailsType> const &renderDetails,
        RenderLoaderVulkanTraits::RenderDetailsParameterType const &parameters)
    {
        return fcns.commonObjectDataCreateFcn(renderDetails, parameters.preTransform);
    }
private:
    std::shared_ptr<vulkan::Device> m_device;
};

#endif // AMAZING_LABYRINTH_RENDER_LOADER_VULKAN_HPP
