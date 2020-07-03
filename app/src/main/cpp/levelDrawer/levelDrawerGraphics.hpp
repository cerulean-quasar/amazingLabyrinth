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
#ifndef AMAZING_LABYRINTH_LEVEL_DRAWER_GRAPHICS_HPP
#define AMAZING_LABYRINTH_LEVEL_DRAWER_GRAPHICS_HPP

#include <memory>
#include <string>

#include "modelTable/modelLoader.hpp"
#include "textureTable/textureLoader.hpp"
#include "../renderDetails/basic/renderDetailsData.hpp"

#include "levelDrawer.hpp"

template <typename traits>
class LevelDrawerGraphics : public LevelDrawer {
public:
    size_t addObject(
            std::shared_ptr<ModelDescription> const &modelDescription,
            std::shared_ptr<TextureDescription> const &textureDescription);

    size_t addModelMatrixForObject(size_t objsIndex, glm::mat4 const &modelMatrix);

    size_t resizeModelMatrices(size_t objsIndex, size_t newSize);

    size_t getNumberModelMatricesForObject(size_t objsIndex);

    void requestRenderDetails(std::string const &name);

    void requestRenderDetailsForObject(size_t objsIndex, std::string const &name);

    void draw(typename traits::DrawArgumentType info);
private:
    std::shared_ptr<typename traits::RenderDetailsReferenceType>> m_renderDetailsReference;
    typename traits::ModelTableType m_modelTable;
    typename traits::TextureTableType m_textureTable;
    typename traits::DrawObjectTableType m_drawObjectTable;
};

#endif // AMAZING_LABYRINTH_LEVEL_DRAWER_GRAPHICS_HPP
