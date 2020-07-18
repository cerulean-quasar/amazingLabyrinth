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
#include "levelDrawerGL.hpp"
#include "levelDrawerGraphics.hpp"

namespace levelDrawer {
    template <>
    LevelDrawerGraphics<LevelDrawerGLTraits>::LevelDrawerGraphics(
            LevelDrawerGLTraits::NeededForDrawingType /* no extra members needed for drawing */,
            std::shared_ptr<LevelDrawerGLTraits::RenderLoaderType> inRenderLoader,
            std::shared_ptr<GameRequester> inGameRequester)
            : m_modelTable{},
            m_textureTable{},
            m_drawObjectTableList{
                std::make_shared<LevelDrawerGLTraits::DrawObjectTableType>(),
                std::make_shared<LevelDrawerGLTraits::DrawObjectTableType>(),
                std::make_shared<LevelDrawerGLTraits::DrawObjectTableType>() },
            m_renderLoader{std::move(inRenderLoader)},
            m_gameRequester{std::move(inGameRequester)}
    {}
}