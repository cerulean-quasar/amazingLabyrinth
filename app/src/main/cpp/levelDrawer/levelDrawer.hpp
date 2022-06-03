/**
 * Copyright 2022 Cerulean Quasar. All Rights Reserved.
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
#include <boost/optional.hpp>

#include "common.hpp"

namespace levelDrawer {
    class LevelDrawer {
    public:
        virtual void setClearColor(ObjectType type, glm::vec4 const &clearColor) = 0;

        virtual bool emptyOfDrawObjects(ObjectType type) = 0;

        virtual size_t numberObjects(ObjectType type) = 0;

        virtual void clearDrawObjectTable(ObjectType type) = 0;

        // returns index of new object
        virtual DrawObjReference addObject(
                ObjectType type,
                std::shared_ptr <ModelDescription> const &modelDescription,
                std::shared_ptr <TextureDescription> const &textureDescription) = 0;

        // returns index of new object
        virtual DrawObjReference addObject(
                ObjectType type,
                std::shared_ptr <ModelDescription> const &modelDescription,
                std::shared_ptr <TextureDescription> const &textureDescription,
                std::string const &renderDetailsName) = 0;

        virtual void removeObject(
                ObjectType type,
                DrawObjReference objIndex) = 0;

        // returns index of new object data.
        virtual DrawObjDataReference addModelMatrixForObject(
                ObjectType type,
                DrawObjReference drawObjReference,
                glm::mat4 const &modelMatrix) = 0;

        virtual void updateModelMatrixForObject(
                ObjectType type,
                DrawObjReference drawObjReference,
                DrawObjDataReference objDataReference,
                glm::mat4 const &modelMatrix) = 0;

        // From draw object reference, draw object data reference, to draw object reference
        // returns true if successful, false otherwise
        virtual boost::optional<DrawObjDataReference> transferObject(
                ObjectType type,
                DrawObjReference fromObjRef,
                DrawObjDataReference objDataRef,
                DrawObjReference toObjRef) = 0;

        virtual void removeObjectData(
            ObjectType type,
            DrawObjReference drawObjReference,
            DrawObjDataReference objDataReference) = 0;

        virtual size_t numberObjectsDataForObject(ObjectType type, DrawObjReference drawObjReference) = 0;

        virtual void requestRenderDetails(ObjectType type, std::string const &name, std::shared_ptr<renderDetails::Parameters> const &parameters) = 0;

        virtual std::pair<glm::mat4, glm::mat4> getProjectionView(ObjectType type) = 0;

        virtual void drawToBuffer(
                std::string const &renderDetailsName,
                ModelsTextures const &modelsTextures,
                std::vector<glm::mat4> const &modelMatrix,
                float width,
                float height,
                uint32_t nbrSamplesForWidth,
                std::shared_ptr<renderDetails::Parameters> const &parameters,
                std::vector<float> &results) = 0;

        virtual ~LevelDrawer() = default;
    };

    class Adaptor {
    public:
        Adaptor(ObjectType type, std::shared_ptr<LevelDrawer> ld)
            : m_type{type},
            m_levelDrawer{std::move(ld)}
        {}

        void setClearColor(glm::vec4 const &clearColor) {
            m_levelDrawer->setClearColor(m_type, clearColor);
        }

        bool emptyOfDrawObjects() {
            return m_levelDrawer->emptyOfDrawObjects(m_type);
        }

        size_t numberObjects() {
            return m_levelDrawer->numberObjects(m_type);
        }

        // returns index of new object
        DrawObjReference addObject(
                std::shared_ptr <ModelDescription> const &modelDescription,
                std::shared_ptr <TextureDescription> const &textureDescription) {
            return m_levelDrawer->addObject(m_type, modelDescription, textureDescription);
        }

        // returns index of new object
        DrawObjReference addObject(
                std::shared_ptr <ModelDescription> const &modelDescription,
                std::shared_ptr <TextureDescription> const &textureDescription,
                std::string const &renderDetailsName) {
            return m_levelDrawer->addObject(m_type, modelDescription, textureDescription, renderDetailsName);
        }

        boost::optional<DrawObjDataReference> transferObject(
                DrawObjReference fromObjRef,
                DrawObjDataReference objDataRef,
                DrawObjReference toObjRef)
        {
            return m_levelDrawer->transferObject(m_type, fromObjRef, objDataRef, toObjRef);
        }

        void removeObject(DrawObjReference drawObjReference) {
            m_levelDrawer->removeObject(m_type, drawObjReference);
        }

        // returns index of new object data.
        DrawObjDataReference addModelMatrixForObject(DrawObjReference drawObjReference, glm::mat4 const &modelMatrix) {
            return m_levelDrawer->addModelMatrixForObject(m_type, drawObjReference, modelMatrix);
        }

        void updateModelMatrixForObject(DrawObjReference objIndex, DrawObjDataReference objDataIndex, glm::mat4 const &modelMatrix) {
            m_levelDrawer->updateModelMatrixForObject(m_type, objIndex, objDataIndex, modelMatrix);
        }

        void removeObjectData(DrawObjReference drawObjReference, DrawObjDataReference objDataReference)
        {
            m_levelDrawer->removeObjectData(m_type, drawObjReference, objDataReference);
        }

        // returns the number of object positions for an object with a specific
        // model and texture.
        size_t numberObjectsDataForObject(DrawObjReference drawObjReference) {
            return m_levelDrawer->numberObjectsDataForObject(m_type, drawObjReference);
        }

        // request a global render details for this level.  Can be overridden by particular
        // objects.
        void requestRenderDetails(std::string const &name, std::shared_ptr<renderDetails::Parameters> const &parameters) {
            return m_levelDrawer->requestRenderDetails(m_type, name, parameters);
        }

        // get the projection and view matrices for the level global render details.
        std::pair<glm::mat4, glm::mat4> getProjectionView() {
            return m_levelDrawer->getProjectionView(m_type);
        }

        void drawToBuffer(
                std::string const &renderDetailsName,
                ModelsTextures const &modelsTextures,
                std::vector<glm::mat4> const &modelMatrix,
                float width,
                float height,
                uint32_t nbrSamplesForWidth,
                std::shared_ptr<renderDetails::Parameters> const &parameters,
                std::vector<float> &results)
        {
            m_levelDrawer->drawToBuffer(renderDetailsName, modelsTextures, modelMatrix, width, height,
                    nbrSamplesForWidth, parameters, results);
        }

    private:
        ObjectType m_type;
        std::shared_ptr<LevelDrawer> m_levelDrawer;
    };
}

#endif // AMAZING_LABYRINTH_LEVEL_DRAWER_HPP
