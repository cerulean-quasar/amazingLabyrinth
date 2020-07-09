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
    using RenderDetailsReferenceType = renderDetails::RenderDetailsReference<RenderDetailsType, CommonObjectDataType>;
    using RenderDetailsParameterType = renderDetails::ParametersVulkan;
    using RetrieveFcns = RenderDetailsVulkanRetrieveFcns;
    static RenderDetailsVulkanRetrieveMap &(*getRenderDetailsMap)() = getRenderDetailsVulkanMap;
};

class RenderLoaderVulkan : public std::enable_shared_from_this<RenderLoaderVulkan>, public RenderLoader<RenderLoaderVulkanTraits> {
public:
    ~RenderLoaderVulkan() override = default;

protected:
    RenderLoaderVulkanTraits::RenderDetailsReferenceType loadNew(
        RenderLoaderVulkanTraits::RetrieveFcns const &fcns,
        std::shared_ptr<GameRequester> const &gameRequester,
        RenderLoaderVulkanTraits::RenderDetailsParameterType const &parameters) override
    {
        return fcns.renderDetailsLoadNewFcn(gameRequester, shared_from_this(), m_device, parameters);
    }

    void reload(
        std::shared_ptr<GameRequester> const &gameRequester,
        std::shared_ptr<RenderLoaderVulkanTraits::RenderDetailsType> const &renderDetails,
        RenderLoaderVulkanTraits::RenderDetailsParameterType const &parameters) override
    {
        renderDetails.reload(gameRequester, shared_from_this(), parameters);
    }

    RenderLoaderVulkanTraits::RenderDetailsReferenceType loadExisting(
        RenderLoaderVulkanTraits::RetrieveFcns const &fcns,
        std::shared_ptr<RenderLoaderVulkanTraits::RenderDetailsType> const &renderDetails,
        RenderLoaderVulkanTraits::RenderDetailsParameterType const &parameters) override
    {
        return fcns.renderDetailsLoadExistingFcn(renderDetails, parameters);
    }
private:
    std::shared_ptr<vulkan::Device> m_device;
};

#endif // AMAZING_LABYRINTH_RENDER_LOADER_VULKAN_HPP
