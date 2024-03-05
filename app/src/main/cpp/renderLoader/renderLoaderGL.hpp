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
#ifndef AMAZING_LABYRINTH_RENDER_LOADER_GL_HPP
#define AMAZING_LABYRINTH_RENDER_LOADER_GL_HPP

#include <memory>

#include "../renderDetails/renderDetails.hpp"
#include "renderLoader.hpp"
#include "../renderDetails/renderDetailsGL.hpp"
#include "registerGL.hpp"

struct RenderLoaderGLTraits {
    using RenderDetailsParametersType = renderDetails::ParametersGL;
    using RenderDetailsType = renderDetails::RenderDetailsGL;
    using CommonObjectDataType = renderDetails::CommonObjectDataBase;
    using RenderDetailsReferenceType = renderDetails::ReferenceGL;
    using RenderDetailsParameterType = renderDetails::ParametersGL;
    using RetrieveFcns = RenderDetailsGLRetrieveFcns;
    using SurfaceDetailsType = graphicsGL::SurfaceDetails;
    static RenderDetailsGLRegistrar &getRenderDetailsRegistrar() {
        return getRenderDetailsGLRegistrar();
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
            std::shared_ptr<RenderLoaderGLTraits::SurfaceDetailsType> const &surfaceDetails,
            std::shared_ptr<renderDetails::ParametersBase> const &parameters) override
    {
        return fcns.renderDetailsLoadNewFcn(gameRequester, shared_from_this(), surfaceDetails, parameters);
    }

    bool structuralChangeNeeded(
            std::shared_ptr<RenderLoaderGLTraits::RenderDetailsType> const &renderDetails,
            std::shared_ptr<RenderLoaderGLTraits::SurfaceDetailsType> const &surfaceDetails) override
    {
        return renderDetails->structuralChangeNeeded(surfaceDetails);
    }

    RenderLoaderGLTraits::RenderDetailsReferenceType loadExisting(
            RenderLoaderGLTraits::RetrieveFcns const &fcns,
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGLTraits::RenderDetailsType> const &renderDetails,
            std::shared_ptr<RenderLoaderGLTraits::SurfaceDetailsType> const &surfaceDetails,
            std::shared_ptr<renderDetails::ParametersBase> const &parameters) override
    {
        return fcns.renderDetailsLoadExistingFcn(gameRequester, shared_from_this(), renderDetails,
                surfaceDetails, parameters);
    }
private:
};

#endif // AMAZING_LABYRINTH_RENDER_LOADER_GL_HPP
