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
#include "../renderDetails/renderDetails.hpp"

namespace levelDrawer {
    class LevelDrawer {
    public:
        enum ObjectType {
            STARTER,
            LEVEL,
            FINISHER
        };

        // returns index of new object
        virtual size_t addObject(
                ObjectType type,
                std::shared_ptr <ModelDescription> const &modelDescription,
                std::shared_ptr <TextureDescription> const &textureDescription) = 0;

        // returns index of new object
        virtual size_t addObject(
                ObjectType type,
                std::shared_ptr <ModelDescription> const &modelDescription,
                std::shared_ptr <TextureDescription> const &textureDescription,
                std::string const &renderDetailsName) = 0;

        // returns index of new object data.
        virtual size_t addModelMatrixForObject(ObjectType type, size_t objsIndex, glm::mat4 const &modelMatrix) = 0;

        virtual void resizeObjectsData(ObjectType type, size_t objsIndex, size_t newSize) = 0;

        virtual size_t numberObjectsDataForObject(ObjectType type, size_t objsIndex) = 0;

        virtual void requestRenderDetails(ObjectType type, std::string const &name) = 0;
    };
}

#endif // AMAZING_LABYRINTH_LEVEL_DRAWER_HPP
