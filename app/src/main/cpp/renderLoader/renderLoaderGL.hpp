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
#ifndef AMAZING_LABYRINTH_RENDER_LOADER_GL_HPP
#define AMAZING_LABYRINTH_RENDER_LOADER_GL_HPP

#include <memory>
#include <string>

#include "../graphicsVulkan.hpp"

#include "../renderDetails/basic/renderDetailsData.hpp"
#include "renderLoader.hpp"
#include "../renderDetails/basic/renderDetailsVulkan.hpp"
#include "registerVulkan.hpp"

struct RenderLoaderGLTraits {
    using RenderDetailsType = renderDetails::RenderDetailsGL;
    using CommonObjectDataType = renderDetails::CommonObjectDataGL;
    using RenderDetailsReferenceType = renderDetails::RenderDetailsReference<RenderDetailsType, CommonObjectDataType>;
    using RenderDetailsParameterType = renderDetails::RenderDetailsParametersGL;
    using RetrieveFcns = RenderDetailsGLRetrieveFcns;
    RenderDetailsGLRetrieveMap (*getRenderDetailsMap)()
};

class RenderLoaderVulkan : public RenderLoader<RenderLoaderGLTraits> {
public:
    ~RenderLoaderVulkan() override = default;

protected:
    std::shared_ptr<RenderLoaderGLTraits::RenderDetailsReferenceType> loadNew(
            RenderLoaderGLTraits::RetrieveFcns const &fcns,
            std::shared_ptr<GameRequester> const &gameRequester,
            std::string const &name,
            RenderLoaderGLTraits::RenderDetailsParameterType const &parameters)
    {
        auto renderDetails = fcns.renderDetailsLoadFcn(gameRequester, parameters);
    }

    void reload(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGLTraits::RenderDetailsType> const &renderDetails,
            RenderLoaderGLTraits::RenderDetailsParameterType const &parameters) override
    {
        renderDetails.reload(gameRequester, parameters);
    }

    std::shared_ptr<RenderLoaderGLTraits::CommonObjectDataType> allocateCommonObjectData(
            RenderLoaderGLTraits::RetrieveFcns const &fcns,
            std::shared_ptr<RenderLoaderGLTraits::RenderDetailsType> const &renderDetails,
            RenderLoaderGLTraits::RenderDetailsParameterType const &parameters)
    {
        return fcns.commonObjectDataCreateFcn(renderDetails);
    }
private:
};

#endif // AMAZING_LABYRINTH_RENDER_LOADER_GL_HPP
