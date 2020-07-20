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
    struct NeededForDrawingGL {
        bool useIntegerSurface;
    };

    struct DrawArgumentGL {
        uint32_t width;
        uiuint32_t height;
    };

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
        using DrawRuleType = DrawObjectTableGL::DrawRule;
        using RenderLoaderType = RenderLoaderGL;
        using RenderDetailsType = renderDetails::RenderDetailsGL;
        using CommonObjectDataType = renderDetails::CommonObjectData;
        using RenderDetailsReferenceType = renderDetails::ReferenceGL;
        using ModelTableType = ModelTableGL;
        using TextureDataType = TextureDataGL;
        using TextureTableType = TextureTableGL;
        using DrawObjectTableType = DrawObjectTableGL;
        using NeededForDrawingType = NeededForDrawingGL;
        using DrawArgumentType = DrawArgumentGL;
    };

    using LevelDrawerGL = LevelDrawerGraphics<LevelDrawerGLTraits>;
}

template <>
void LevelDrawerGraphics<LevelDrawerGLTraits>::draw(
        LevelDrawerGLTraits::DrawArgumentType info);

template <>
void LevelDrawerGraphics<LevelDrawerGLTraits>::drawToBuffer(
        std::string const &renderDetailsName,
        std::vector<std::pair<std::shared_ptr<ModelDescription>, std::shared_ptr<TextureDescription>>> const &modelsTextures,
        float width,
        float height,
        uint32_t nbrSamplesForWidth,
        float farthestDepth,
        float nearestDepth,
        std::vector<float> &results);

template <>
LevelDrawerGraphics<LevelDrawerGLTraits>::LevelDrawerGraphics(
        LevelDrawerGLTraits::NeededForDrawingType neededForDrawing,
        std::shared_ptr<LevelDrawerGLTraits::RenderLoaderType> inRenderLoader,
        std::shared_ptr<GameRequester> inGameRequester);

#endif // AMAZING_LABYRINTH_LEVEL_DRAWER_GL_HPP
