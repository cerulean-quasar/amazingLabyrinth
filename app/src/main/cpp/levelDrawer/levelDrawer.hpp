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
#ifndef AMAZING_LABYRINTH_LEVEL_DRAWER_HPP
#define AMAZING_LABYRINTH_LEVEL_DRAWER_HPP

#include <memory>
#include <string>

#include "modelTable/modelLoader.hpp"
#include "textureTable/textureLoader.hpp"
#include "../renderDetails/basic/renderDetailsData.hpp"

template <typename RenderDetailsType, typename CommonObjectDataType>
struct RenderDetailsReference {
    std::shared_ptr<RenderDetailsType> renderDetailsData;
    std::shared_ptr<CommonObjectDataType> commonObjectData;
};

class LevelDrawer {
public:
    size_t addObject(
            std::shared_ptr <ModelDescription> const &modelDescription,
            std::shared_ptr <TextureDescription> const &textureDescription) = 0;

    size_t addModelMatrixForObject(size_t objsIndex, glm::mat4 const &modelMatrix) = 0;

    size_t resizeModelMatrices(size_t objsIndex, size_t newSize) = 0;

    size_t getNumberModelMatricesForObject(size_t objsIndex) = 0;

    void requestRenderDetails(std::string const &name) = 0;

    void requestRenderDetailsForObject(size_t objsIndex, std::string const &name) = 0;
};

#endif // AMAZING_LABYRINTH_LEVEL_DRAWER_HPP
