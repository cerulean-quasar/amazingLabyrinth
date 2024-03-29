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
#ifndef AMAZING_LABYRINTH_DRAW_OBJECT_TABLE_HPP
#define AMAZING_LABYRINTH_DRAW_OBJECT_TABLE_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <glm/glm.hpp>
#include <boost/optional.hpp>

#include "../../renderDetails/renderDetails.hpp"

namespace levelDrawer {
    template <typename traits>
    class DrawObjectTable;

    template<typename traits>
    class DrawObject {
        friend DrawObjectTable<traits>;
    public:
        bool hasOverridingRenderDetailsReference() {
            return m_renderDetailsReference.renderDetails != nullptr;
        }

        typename traits::RenderDetailsReferenceType const &renderDetailsReference() {
            return m_renderDetailsReference;
        }

        std::shared_ptr<typename traits::ModelDataType> const &modelData() { return m_modelData; }

        std::shared_ptr<typename traits::TextureDataType> const &textureData() { return m_textureData; }

        std::vector<DrawObjDataReference> drawObjDataRefs() {
            std::vector<DrawObjDataReference> ret;
            for (auto const &objData : m_objsData) {
                ret.push_back(objData.first);
            }
            return std::move(ret);
        }

        std::shared_ptr<typename traits::DrawObjectDataType> const &objData(DrawObjDataReference objDataRef) {
            auto it = m_objsData.find(objDataRef);
            if (it == m_objsData.end()) {
                throw std::runtime_error("Invalid draw object data reference on retrieve.");
            }

            return it->second;
        }

        size_t numberObjectsData() const {
            return m_objsData.size();
        }

        DrawObject(
                typename traits::RenderDetailsReferenceType renderDetailsReference_,
                std::shared_ptr<typename traits::ModelDataType> modelData_,
                std::shared_ptr<typename traits::TextureDataType> textureData_)
                : m_nextDrawObjDataReference{0},
                  m_renderDetailsReference{std::move(renderDetailsReference_)},
                  m_modelData{std::move(modelData_)},
                  m_textureData{textureData_} {}

        DrawObject(
                std::shared_ptr<typename traits::ModelDataType> modelData_,
                std::shared_ptr<typename traits::TextureDataType> textureData_)
                : m_nextDrawObjDataReference{0},
                  m_renderDetailsReference{},
                  m_modelData{modelData_},
                  m_textureData{textureData_} {}

    private:
        DrawObjDataReference addObjectData(std::shared_ptr<typename traits::DrawObjectDataType> objectData) {
            DrawObjDataReference objDataRef = m_nextDrawObjDataReference++;
            m_objsData.emplace(objDataRef, objectData);
            return objDataRef;
        }

        void updateObjectData(DrawObjDataReference objDataRef, glm::mat4 const &modelMatrix) {
            auto it = m_objsData.find(objDataRef);
            if (it == m_objsData.end()) {
                throw std::runtime_error("Invalid draw object data reference on update.");
            }
            it->second->update(modelMatrix);
        }

        void removeObjectData(DrawObjDataReference objDataRef) {
            m_objsData.erase(objDataRef);
        }

        DrawObjReference m_nextDrawObjDataReference;
        typename traits::RenderDetailsReferenceType m_renderDetailsReference;
        std::shared_ptr<typename traits::ModelDataType> m_modelData;
        std::shared_ptr<typename traits::TextureDataType> m_textureData;
        std::unordered_map<DrawObjDataReference, std::shared_ptr<typename traits::DrawObjectDataType>> m_objsData;
    };

    template <typename traits>
    class DrawObjectTable {
    public:
        struct DrawRule {
            std::shared_ptr<typename traits::RenderDetailsType> renderDetails;
            std::shared_ptr<typename traits::CommonObjectDataType> commonObjectData;
            std::vector<DrawObjReference> drawObjectIndices;
        };

        bool emptyOfDrawObjects() {
            return m_drawObjects.empty();
        }

        void clear() {
            m_renderDetailsReference = typename traits::RenderDetailsReferenceType{};
            m_drawObjects.clear();
            m_objsIndicesWithOverridingRenderDetails.clear();
            m_objsIndicesWithGlobalRenderDetails.clear();
            m_zValueReferernces.clear();
            m_nextDrawObjReference = 0;
        }

        // returns index of added object.
        DrawObjReference addObject(
                std::shared_ptr<typename traits::ModelDataType> modelData,
                std::shared_ptr<typename traits::TextureDataType> textureData)
        {
            if (m_renderDetailsReference.renderDetails == nullptr) {
                throw std::runtime_error("Default render details must be requested before adding a draw object that uses the default render details.");
            }
            DrawObjReference objRef = m_nextDrawObjReference++;
            m_drawObjects.emplace(objRef, std::make_shared<DrawObject<traits>>(
                    std::move(modelData), std::move(textureData)));

            m_objsIndicesWithGlobalRenderDetails.insert(objRef);
            return objRef;
        }

        // returns index of added object.
        DrawObjReference addObject(
                typename traits::RenderDetailsReferenceType renderDetailsReference,
                std::shared_ptr<typename traits::ModelDataType> modelData,
                std::shared_ptr<typename traits::TextureDataType> textureData)
        {
            DrawObjReference objRef = m_nextDrawObjReference++;

            if (renderDetailsReference.renderDetails != nullptr) {
                m_objsIndicesWithOverridingRenderDetails.insert(objRef);
                m_drawObjects.emplace(
                        objRef,
                        std::make_shared<DrawObject<traits>>(
                            std::move(renderDetailsReference), std::move(modelData),
                            std::move(textureData)));
            } else if (m_renderDetailsReference.renderDetails != nullptr) {
                m_objsIndicesWithGlobalRenderDetails.insert(objRef);
                m_drawObjects.emplace(
                        objRef,
                        std::make_shared<DrawObject<traits>>(
                            std::move(modelData), std::move(textureData)));
            } else {
                throw std::runtime_error("Default render details must be requested before adding a draw object that uses the default render details.");
            }



            return objRef;
        }

        void removeObject(DrawObjReference objReference) {
            auto itObjRef = m_drawObjects.find(objReference);
            if (itObjRef == m_drawObjects.end()) {
                return;
            }

            if (itObjRef->second->hasOverridingRenderDetailsReference()) {
                m_objsIndicesWithOverridingRenderDetails.erase(objReference);
            } else {
                m_objsIndicesWithGlobalRenderDetails.erase(objReference);
            }

            ZValueReference zRefToFind(boost::none, objReference, boost::none);
            for (auto it = m_zValueReferernces.begin(); it != m_zValueReferernces.end(); ) {
                if (*it == zRefToFind) {
                    it = m_zValueReferernces.erase(it);
                } else {
                    it++;
                }
            }
            m_drawObjects.erase(itObjRef);
        }

        std::set<ZValueReference> const &zValueReferences() { return m_zValueReferernces; }

        std::vector<DrawObjReference> objsIndicesWithOverridingRenderDetails() {
            return std::vector<DrawObjReference>(m_objsIndicesWithOverridingRenderDetails.begin(),
                    m_objsIndicesWithOverridingRenderDetails.end());
        }

        std::vector<DrawObjReference> objsIndicesWithGlobalRenderDetails() {
            return std::vector<DrawObjReference>(m_objsIndicesWithGlobalRenderDetails.begin(),
                    m_objsIndicesWithGlobalRenderDetails.end());
        }

        typename traits::RenderDetailsReferenceType const &renderDetailsReference() {
            return m_renderDetailsReference;
        }

        typename traits::RenderDetailsReferenceType const &renderDetailsReference(DrawObjReference drawObjRef) {
            auto it = m_drawObjects.find(drawObjRef);
            if (it == m_drawObjects.end()) {
                throw std::runtime_error("Invalid draw object reference on finding the number of draw object data.");
            }

            if (it->second->hasOverridingRenderDetailsReference()) {
                return it->second->renderDetailsReference();
            }

            return m_renderDetailsReference;
        }

        size_t numberObjects() { return m_drawObjects.size(); }

        size_t numberObjectsDataForObject(DrawObjReference drawObjRef) {
            auto it = m_drawObjects.find(drawObjRef);
            if (it == m_drawObjects.end()) {
                throw std::runtime_error("Invalid draw object reference on finding the number of draw object data.");
            }

            return it->second->numberObjectsData();
        }

        std::shared_ptr<DrawObject<traits>> const &drawObject(DrawObjReference drawObjRef) {
            auto it = m_drawObjects.find(drawObjRef);
            if (it == m_drawObjects.end()) {
                throw std::runtime_error("Invalid draw object reference on retrieve draw object.");
            }

            return it->second;
        }

        DrawObjDataReference addDrawObjData(DrawObjReference drawObjRef, std::shared_ptr<typename traits::DrawObjectDataType> objData) {
            auto it = m_drawObjects.find(drawObjRef);
            if (it == m_drawObjects.end()) {
                throw std::runtime_error("Invalid draw object reference on add object data");
            }

            float zVal = zValue(objData);
            auto objDataRef = it->second->addObjectData(std::move(objData));
            auto ret = m_zValueReferernces.emplace(zVal, drawObjRef, objDataRef);

            if (!ret.second) {
                throw std::runtime_error("draw object data already in the Z value reference table!");
            }

            return objDataRef;
        }

        // returns boost::none if it failed.
        boost::optional<DrawObjDataReference> transferObject(DrawObjReference objRef1, DrawObjDataReference objDataRef, DrawObjReference objRef2) {
            auto it1 = m_drawObjects.find(objRef1);
            auto it2 = m_drawObjects.find(objRef2);
            if (it1 == m_drawObjects.end() || it2 == m_drawObjects.end()) {
                throw std::runtime_error("Invalid draw object reference on transfer");
            }

            if ((!it1->second->hasOverridingRenderDetailsReference() &&
                 !it2->second->hasOverridingRenderDetailsReference()) ||
                (it1->second->hasOverridingRenderDetailsReference() &&
                 it2->second->hasOverridingRenderDetailsReference() &&
                 it1->second->renderDetailsReference().renderDetails->nameString() ==
                 it2->second->renderDetailsReference().renderDetails->nameString()))
            {
                // object is of the same render details, it is ok to move.
                std::shared_ptr<typename traits::DrawObjectDataType> objData = it1->second->objData(objDataRef);
                bool succeeded = false;
                if (it1->second->hasOverridingRenderDetailsReference()) {
                    succeeded = objData->updateTextureData(
                            it1->second->renderDetailsReference().commonObjectData,
                            it2->second->textureData());
                } else {
                    succeeded = objData->updateTextureData(
                            m_renderDetailsReference.commonObjectData,
                            it2->second->textureData());
                }

                if (succeeded) {
                    it1->second->removeObjectData(objDataRef);

                    auto objDataRefNew = it2->second->addObjectData(objData);
                    float oldZ = zValue(objData);
                    m_zValueReferernces.erase(ZValueReference(oldZ, objRef1, objDataRef));
                    m_zValueReferernces.emplace(oldZ, objRef2, objDataRefNew);
                    return boost::optional<DrawObjDataReference>(objDataRefNew);
                }
            }

            return boost::none;
        }

        void updateObjectData(DrawObjReference objRef, DrawObjDataReference objDataRef, glm::mat4 const &modelMatrix) {
            auto it = m_drawObjects.find(objRef);
            if (it == m_drawObjects.end()) {
                throw std::runtime_error("Invalid draw object reference on update");
            }
            float oldZ = zValue(it->second->objData(objDataRef)->modelMatrix(0));
            it->second->updateObjectData(objDataRef, modelMatrix);

            size_t nbrRemoved = m_zValueReferernces.erase(ZValueReference(oldZ, objRef, objDataRef));
            if (nbrRemoved != 1) {
                throw std::runtime_error("Unexpected number of items removed!");
            }

            m_zValueReferernces.emplace(zValue(modelMatrix), objRef, objDataRef);
        }

        void removeObjectData(DrawObjReference objRef, DrawObjDataReference objDataRef) {
            auto it = m_drawObjects.find(objRef);
            if (it == m_drawObjects.end()) {
                throw std::runtime_error("Invalid draw object reference on remove");
            }
            float oldZ = zValue(it->second->objData(objDataRef)->modelMatrix(0));
            it->second->removeObjectData(objDataRef);
            size_t nbrRemoved = m_zValueReferernces.erase(ZValueReference(oldZ, objRef, objDataRef));
            if (nbrRemoved != 1) {
                throw std::runtime_error("Unexpected number of items removed!");
            }
        }

        void loadRenderDetails(typename traits::RenderDetailsReferenceType ref) {
            m_renderDetailsReference = std::move(ref);
        }

        DrawObjectTable()
                : m_nextDrawObjReference{0}
        {}

    private:
        float zValue(std::shared_ptr<typename traits::DrawObjectDataType> const &objData) {
            return zValue(objData->modelMatrix(0));
        }

        float zValue(glm::mat4 const &modelMatrix) {
            glm::vec4 zVec = modelMatrix * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f};
            return zVec.z / zVec.w;
        }

        typename traits::RenderDetailsReferenceType m_renderDetailsReference;

        DrawObjReference m_nextDrawObjReference;
        std::unordered_map<DrawObjReference, std::shared_ptr<DrawObject<traits>>> m_drawObjects;
        std::unordered_set<DrawObjReference> m_objsIndicesWithOverridingRenderDetails;
        std::unordered_set<DrawObjReference> m_objsIndicesWithGlobalRenderDetails;
        std::set<ZValueReference> m_zValueReferernces;
    };
}
#endif // AMAZING_LABYRINTH_DRAW_OBJECT_TABLE_HPP
