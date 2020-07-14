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
        virtual size_t addModelMatrixForObject(
                ObjectType type,
                size_t objsIndex,
                glm::mat4 const &modelMatrix) = 0;

        virtual void updateModelMatrixForObject(
                ObjectType type,
                size_t objsIndex,
                size_t objDataIndex,
                glm::mat4 const &modelMatrix) = 0;

        virtual void resizeObjectsData(ObjectType type, size_t objsIndex, size_t newSize) = 0;

        virtual size_t numberObjectsDataForObject(ObjectType type, size_t objsIndex) = 0;

        virtual void requestRenderDetails(ObjectType type, std::string const &name) = 0;

        virtual std::pair<glm::mat4, glm::mat4> getProjectionView(ObjectType type) = 0;

        virtual ~LevelDrawer() = default;
    };

    class Adaptor {
    public:
        Adaptor(LevelDrawer::ObjectType type, std::shared_ptr<LevelDrawer> ld)
            : m_type{type},
            m_levelDrawer{std::move(ld)}
        {}

        // returns index of new object
        size_t addObject(
                std::shared_ptr <ModelDescription> const &modelDescription,
                std::shared_ptr <TextureDescription> const &textureDescription) {
            return m_levelDrawer->addObject(m_type, modelDescription, textureDescription);
        }

        // returns index of new object
        size_t addObject(
                std::shared_ptr <ModelDescription> const &modelDescription,
                std::shared_ptr <TextureDescription> const &textureDescription,
                std::string const &renderDetailsName) {
            return m_levelDrawer->addObject(m_type, modelDescription, textureDescription, renderDetailsName);
        }

        // returns index of new object data.
        size_t addModelMatrixForObject(size_t objIndex, glm::mat4 const &modelMatrix) {
            return m_levelDrawer->addModelMatrixForObject(m_type, objIndex, modelMatrix);
        }

        void updateModelMatrixForObject(size_t objIndex, size_t objDataIndex, glm::mat4 const &modelMatrix) {
            m_levelDrawer->updateModelMatrixForObject(m_type, objIndex, objDataIndex, modelMatrix);
        }

        // resize:  trim off back if newSize is smaller than current size,
        //          add glm::mat4(1.0f) to the back if larger than current size.
        void resizeObjectsData(size_t objsIndex, size_t newSize) {
            m_levelDrawer->resizeObjectsData(m_type, objsIndex,newSize);
        }

        // returns the number of object positions for an object with a specific
        // model and texture.
        size_t numberObjectsDataForObject(size_t objsIndex) {
            return m_levelDrawer->numberObjectsDataForObject(m_type, objsIndex);
        }

        // request a global render details for this level.  Can be overridden by particular
        // objects.
        void requestRenderDetails(std::string const &name) {
            return m_levelDrawer->requestRenderDetails(m_type, name);
        }

        // get the projection and view matrices for the level global render details.
        std::pair<glm::mat4, glm::mat4> getProjectionView() {
            return m_levelDrawer->getProjectionView(m_type);
        }
    private:
        LevelDrawer::ObjectType m_type;
        std::shared_ptr<LevelDrawer> m_levelDrawer;
    };
}

#endif // AMAZING_LABYRINTH_LEVEL_DRAWER_HPP
