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
#ifndef AMAZING_LABYRINTH_DRAW_OBJECT_TABLE_GL_HPP
#define AMAZING_LABYRINTH_DRAW_OBJECT_TABLE_GL_HPP

#include "../modelTable/modelTableGL.hpp"
#include "../textureTable/textureTableGL.hpp"
#include "../../renderDetails/renderDetails.hpp"

namespace renderDetails {
    class RenderDetailsGL;
    class CommonObjectData;
    struct ParametersGL;
    class DrawObjectDataGL;
}
namespace levelDrawer {
    struct DrawObjectGLTraits {
        using RenderDetailsParametersType = renderDetails::ParametersGL;
        using RenderDetailsType = renderDetails::RenderDetailsGL;
        using CommonObjectDataType = renderDetails::CommonObjectData;
        using RenderDetailsReferenceType = renderDetails::Reference<renderDetails::RenderDetailsGL, renderDetails::CommonObjectData, renderDetails::DrawObjectDataGL>;
        using ModelDataType = ModelDataGL;
        using TextureDataType = TextureDataGL;
        using DrawObjectDataType = renderDetails::DrawObjectDataGL;
    };
}

#endif // AMAZING_LABYRINTH_DRAW_OBJECT_TABLE_GL_HPP