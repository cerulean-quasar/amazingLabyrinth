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
#include "../renderDetails/basic/renderDetailsGL.hpp"

// Register RenderDetailsGL types
struct RenderDetailsGLRetrieveFcns {
    using RenderDetailsFcn = std::function<std::shared_ptr<renderDetails::RenderDetailsGL>(
            std::shared_ptr<GameRequester> const &,
    std::shared_ptr<vulkan::Device>,
    renderDetails::RenderDetailsParametersGL const &);

    using CommonObjectDataFcn = std::function<std::shared_ptr<renderDetails::CommonObjectDataGL>(
            std::shared_ptr<renderDetails::RenderDetailsGL> const &,
    renderDetails::RenderDetailsParametersGL const &);

    RenderDetailsFcn renderDetailsLoadFcn;
    CommonObjectDataFcn commonObjectDataCreateFcn;
};

using RenderDetailsGLRetrieveMap =
    std::map<std::string, RenderDetailsRetrieveFcns<RenderDetailsLoadFcn, CommonObjectDataCreateFcn>>;

RenderDetailsGLRetrieveMap &getRenderDetailsGLMap() {
    static RenderDetailsGLRetrieveMap map{};

    return map;
}

template <typename RenderDetailsType, typename CommonObjectDataType, typename ConfigType, typename ParametersType>
class RegisterGL {
    RegisterGL() {
        getRenderDetailsGLMap().emplace(
            RenderDetailsType::name(),
            std::function<RenderDetailsGLRetrieveFcns()> (
                []() -> RenderDetailsGLRetrieveFcns {
                    RenderDetailsGLRetrieveFcns fcns;
                    ConfigType config;
                    fcns.renderDetailsLoadFcn = RenderDetailsGLRetrieveFcns::RenderDetailsFcn (
                            [] (std::shared_ptr<GameRequester> const &gameRequester,
                                    ParametersType const &parameters) {
                                return std::make_shared<RenderDetailsType>(gameRequester, parameters);
                            });
                    fcns.commonObjectDataCreateFcn = RenderDetailsGLRetrieveFcns::CommonObjectDataFcn (
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