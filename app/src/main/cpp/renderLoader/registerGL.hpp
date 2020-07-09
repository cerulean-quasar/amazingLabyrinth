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
#ifndef AMAZING_LABYRINTH_RENDER_LOADER_REGISTER_GL_HPP
#define AMAZING_LABYRINTH_RENDER_LOADER_REGISTER_GL_HPP

#include <memory>
#include <map>

#include "../common.hpp"
#include "../renderDetails/renderDetailsGL.hpp"

// Register RenderDetailsGL types
struct RenderDetailsGLRetrieveFcns {
    using RenderDetailsReferenceGL = renderDetails::RenderDetailsReference<renderDetails::RenderDetailsGL, renderDetails::CommonObjectData>;
    using RenderDetailsLoadNewFcn = std::function<std::shared_ptr<RenderDetailsReferenceGL>(
        std::shared_ptr<GameRequester> const &,
        std::shared_ptr<vulkan::Device>,
        renderDetails::RenderDetailsParametersGL const &);

    using RenderDetailsLoadExistingFcn = std::function<std::shared_ptr<RenderDetailsReferenceGL>(
        std::shared_ptr<renderDetails::RenderDetailsGL> const &,
        renderDetails::RenderDetailsParametersGL const &);

    RenderDetailsLoadNewFcn renderDetailsLoadNewFcn;
    RenderDetailsLoadExistingFcn renderDetailsLoadExistingFcn;
};

using RenderDetailsGLRetrieveMap =
    std::map<std::string, RenderDetailsRetrieveFcns<RenderDetailsLoadFcn, CommonObjectDataCreateFcn>>;

RenderDetailsGLRetrieveMap &getRenderDetailsGLMap() {
    static RenderDetailsGLRetrieveMap map{};

    return map;
}

class RenderLoaderGL;
template <typename RenderDetailsType, typename CommonObjectDataType, typename ConfigType, typename ParametersType>
class RegisterGL {
    RegisterGL() {
        getRenderDetailsGLMap().emplace(
            RenderDetailsType::name(),
            std::function<RenderDetailsGLRetrieveFcns()> (
                []() -> RenderDetailsGLRetrieveFcns {
                    RenderDetailsGLRetrieveFcns fcns;
                    ConfigType config;
                    fcns.renderDetailsLoadNewFcn = RenderDetailsGLRetrieveFcns::RenderDetailsLoadNewFcn (
                            [config] (std::shared_ptr<GameRequester> const &gameRequester,
                                    std::shared_ptr<RenderLoaderGL> const &renderLoader,
                                    ParametersType const &parameters) -> RenderDetailsGLRetrieveFcns::RenderDetailsReferenceGL
                            {
                                return RenderDetailsType::loadNew(gameRequester, renderLoader, parameters, config);
                            });
                    fcns.renderDetailsLoadExistingFcn = RenderDetailsGLRetrieveFcns::RenderDetailsLoadExistingFcn (
                            [config] (std::shared_ptr<RenderDetailsType> const &rd,
                                      std::shared_ptr<RenderLoaderGL> const &renderLoader,
                                      ParametersType const &parameters) -> RenderDetailsGLRetrieveFcns::RenderDetailsReferenceGL
                            {
                                return RenderDetailsType::loadExisting(rd, renderLoader, parameters, config);
                            });
                    return std::move(fcns);
                }
            )
        )
    }
};

#endif // AMAZING_LABYRINTH_RENDER_LOADER_REGISTER_VULKAN_HPP