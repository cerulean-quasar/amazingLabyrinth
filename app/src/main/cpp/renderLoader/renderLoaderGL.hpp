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

#include "../renderDetails/renderDetails.hpp"
#include "renderLoader.hpp"
#include "../renderDetails/renderDetailsGL.hpp"
#include "registerGL.hpp"

struct RenderLoaderGLTraits {
    using RenderDetailsParametersType = renderDetails::ParametersGL;
    using RenderDetailsType = renderDetails::RenderDetailsGL;
    using CommonObjectDataType = renderDetails::CommonObjectData;
    using RenderDetailsReferenceType = renderDetails::ReferenceGL;
    using RenderDetailsParameterType = renderDetails::ParametersGL;
    using RetrieveFcns = RenderDetailsGLRetrieveFcns;
    static RenderDetailsGLRetrieveMap &getRenderDetailsMap() {
        return getRenderDetailsGLMap();
    }
};

class RenderLoaderGL :  public std::enable_shared_from_this<RenderLoaderGL>, public RenderLoader<RenderLoaderGLTraits> {
public:
    void clearRenderDetails() {
        m_loadedRenderDetails.clear();
    }

    ~RenderLoaderGL() override = default;

protected:
    RenderLoaderGLTraits::RenderDetailsReferenceType loadNew(
            RenderLoaderGLTraits::RetrieveFcns const &fcns,
            std::shared_ptr<GameRequester> const &gameRequester,
            RenderLoaderGLTraits::RenderDetailsParameterType const &parameters) override
    {
        return fcns.renderDetailsLoadNewFcn(gameRequester, shared_from_this(), parameters);
    }

    void reload(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGLTraits::RenderDetailsType> const &renderDetails,
            RenderLoaderGLTraits::RenderDetailsParameterType const &parameters) override
    {
        renderDetails.reload(gameRequester, shared_from_this(), parameters);
    }

    RenderLoaderGLTraits::RenderDetailsReferenceType loadExisting(
            RenderLoaderGLTraits::RetrieveFcns const &fcns,
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGLTraits::RenderDetailsType> const &renderDetails,
            RenderLoaderGLTraits::RenderDetailsParameterType const &parameters) override
    {
        return fcns.renderDetailsLoadExistingFcn(gameRequester, shared_from_this(), renderDetails, parameters);
    }
private:
};

#endif // AMAZING_LABYRINTH_RENDER_LOADER_GL_HPP
