/**
 * Copyright 2022 Cerulean Quasar. All Rights Reserved.
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
#ifndef AMAZING_LABYRINTH_RENDER_LOADER_REGISTER_GL_HPP
#define AMAZING_LABYRINTH_RENDER_LOADER_REGISTER_GL_HPP

#include <memory>
#include <map>

#include "../common.hpp"
#include "../renderDetails/renderDetails.hpp"
#include "../levelDrawer/textureTable/textureTableGL.hpp"
#include "../graphicsGL.hpp"

namespace renderDetails {
    class RenderDetailsGL;
    class DrawObjectDataGL;
    using ReferenceGL = Reference<RenderDetailsGL, levelDrawer::TextureDataGL, DrawObjectDataGL>;
    struct ParametersGL;
}

// Register RenderDetailsGL types
class RenderLoaderGL;
struct RenderDetailsGLRetrieveFcns {
    using RenderDetailsReferenceGL = renderDetails::ReferenceGL;
    using RenderDetailsLoadNewFcn = std::function<renderDetails::ReferenceGL(
        std::shared_ptr<GameRequester> const &,
        std::shared_ptr<RenderLoaderGL> const &renderLoader,
        std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails,
        std::shared_ptr<renderDetails::Parameters> const &)>;

    using RenderDetailsLoadExistingFcn = std::function<renderDetails::ReferenceGL(
        std::shared_ptr<GameRequester> const &,
        std::shared_ptr<RenderLoaderGL> const &renderLoader,
        std::shared_ptr<renderDetails::RenderDetailsGL> const &,
        std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails,
        std::shared_ptr<renderDetails::Parameters> const &)>;

    RenderDetailsLoadNewFcn renderDetailsLoadNewFcn;
    RenderDetailsLoadExistingFcn renderDetailsLoadExistingFcn;
};

using RenderDetailsGLRetrieveMap =
    std::map<std::string, std::function<RenderDetailsGLRetrieveFcns()>>;

RenderDetailsGLRetrieveMap &getRenderDetailsGLMap();

template <typename RenderDetailsBaseType, typename RenderDetailsType>
class RegisterGL {
public:
    RegisterGL(
            char const *name,
            std::vector<char const *> shaders) {
        getRenderDetailsGLMap().emplace(
            name,
            std::function<RenderDetailsGLRetrieveFcns()> (
                [name, shaders]() -> RenderDetailsGLRetrieveFcns {
                    RenderDetailsGLRetrieveFcns fcns;
                    fcns.renderDetailsLoadNewFcn = RenderDetailsGLRetrieveFcns::RenderDetailsLoadNewFcn (
                            [name, shaders] (std::shared_ptr<GameRequester> const &gameRequester,
                                    std::shared_ptr<RenderLoaderGL> const &renderLoader,
                                      std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails,
                                      std::shared_ptr<renderDetails::Parameters> const &parameters) -> RenderDetailsGLRetrieveFcns::RenderDetailsReferenceGL
                            {
                                return RenderDetailsType::loadNew(name, shaders,
                                                                  gameRequester, renderLoader, surfaceDetails, parameters);
                            });
                    fcns.renderDetailsLoadExistingFcn = RenderDetailsGLRetrieveFcns::RenderDetailsLoadExistingFcn (
                            [] (std::shared_ptr<GameRequester> const &gameRequester,
                                      std::shared_ptr<RenderLoaderGL> const &renderLoader,
                                      std::shared_ptr<RenderDetailsBaseType> const &rd,
                                      std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails,
                                      std::shared_ptr<renderDetails::Parameters> const &parameters) -> RenderDetailsGLRetrieveFcns::RenderDetailsReferenceGL
                            {
                                return RenderDetailsType::loadExisting(gameRequester, renderLoader,
                                        rd, surfaceDetails, parameters);
                            });
                    return std::move(fcns);
                }
            )
        );
    }
};

#endif // AMAZING_LABYRINTH_RENDER_LOADER_REGISTER_GL_HPP