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
#ifndef AMAZING_LABYRINTH_LEVEL_DRAWER_GL_HPP
#define AMAZING_LABYRINTH_LEVEL_DRAWER_GL_HPP

#include <memory>
#include <string>
#include "textureTable/textureTableGL.hpp"
#include "modelTable/modelTableGL.hpp"
#include "levelDrawerGraphics.hpp"
#include "drawObjectTable/drawObjectTable.hpp"
#include "../renderLoader/renderLoaderGL.hpp"

namespace renderDetails {
    class RenderDetailsGL;
    class CommonObjectData;
    class DrawObjectDataGL;
    class ParametersGL;
    using ReferenceGL = Reference<RenderDetailsGL, CommonObjectData, DrawObjectDataGL>;
}

namespace levelDrawer {
    struct DrawObjectGLTraits {
        using RenderDetailsParametersType = renderDetails::ParametersGL;
        using RenderDetailsType = renderDetails::RenderDetailsGL;
        using CommonObjectDataType = renderDetails::CommonObjectData;
        using RenderDetailsReferenceType = renderDetails::ReferenceGL;
        using ModelDataType = ModelDataGL;
        using TextureDataType = TextureDataGL;
        using DrawObjectDataType = renderDetails::DrawObjectDataGL;
    };

    using DrawObjectTableGL = DrawObjectTable<DrawObjectGLTraits>;

    struct LevelDrawerGLTraits {
        using RenderLoaderType = RenderLoaderGL;
        using RenderDetailsType = renderDetails::RenderDetailsGL;
        using CommonObjectDataType = renderDetails::CommonObjectData;
        using RenderDetailsReferenceType = renderDetails::ReferenceGL;
        using ModelTableType = ModelTableGL;
        using TextureDataType = TextureDataGL;
        using TextureTableType = TextureTableGL;
        using DrawObjectTableType = DrawObjectTableGL;
        struct DrawArgumentType {
        };
    };

    using LevelDrawerGL = LevelDrawerGraphics<LevelDrawerGLTraits>;
}
#endif // AMAZING_LABYRINTH_LEVEL_DRAWER_GL_HPP
