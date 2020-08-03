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
            return ret;
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
            } else {
                m_objsIndicesWithGlobalRenderDetails.insert(objRef);
                m_drawObjects.emplace(
                        objRef,
                        std::make_shared<DrawObject<traits>>(
                            std::move(modelData), std::move(textureData)));
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

            m_zValueReferernces.erase(ZValueReference(boost::none, objReference, boost::none));
            m_drawObjects.erase(itObjRef);
        }

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

            return it->addObjectData(std::move(objData));
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
                it1->second->removeObjectData(objDataRef);
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
                    return boost::optional<DrawObjDataReference>(it2->second->addObjectData(objData));
                }
            }

            return boost::none;
        }

        void updateObjectData(DrawObjReference objRef, DrawObjDataReference objDataRef, glm::mat4 const &modelMatrix) {
            auto it = m_drawObjects.find(objRef);
            if (it == m_drawObjects.end()) {
                throw std::runtime_error("Invalid draw object reference on update");
            }
            it->second->updateObjectData(objDataRef, modelMatrix);
        }

        void removeObjectData(DrawObjReference objRef, DrawObjDataReference objDataRef) {
            auto it = m_drawObjects.find(objRef);
            if (it == m_drawObjects.end()) {
                throw std::runtime_error("Invalid draw object reference on remove");
            }
            it->second->removeObjectData(objDataRef);
        }

        void loadRenderDetails(typename traits::RenderDetailsReferenceType ref) {
            m_renderDetailsReference = std::move(ref);
        }

        std::vector<DrawRule> getDrawRules() {
            // a null render details may mean that no one requested a general render details.
            // this probably means that this table is for the level starter and a level starter was
            // not needed.
            if (m_renderDetailsReference.renderDetails == nullptr) {
                return std::vector<DrawRule>{};
            }

            std::vector<DrawRule> rules;
            rules.emplace_back(DrawRule{m_renderDetailsReference.renderDetails,
                    m_renderDetailsReference.commonObjectData,
                    std::vector<DrawObjReference>(m_objsIndicesWithGlobalRenderDetails.begin(),
                            m_objsIndicesWithGlobalRenderDetails.end())});

            std::map<std::string, DrawRule> alreadyAdded;
            for (auto drawObjRef : m_objsIndicesWithOverridingRenderDetails) {
                auto it = m_drawObjects.find(drawObjRef);
                if (it == m_drawObjects.end()) {
                    throw std::runtime_error("Invalid draw object reference in get draw rules");
                }
                auto const &ref = it->second->renderDetailsReference();
                auto ret = alreadyAdded.emplace(ref.renderDetails->nameString(),
                        DrawRule{ref.renderDetails, ref.commonObjectData, std::vector<DrawObjReference>{}});
                ret.first->second.drawObjectIndices.push_back(drawObjRef);
            }

            for (auto item : alreadyAdded) {
                rules.push_back(item.second);
            }

            return std::move(rules);
        }

        DrawObjectTable()
                : m_nextDrawObjReference{0}
        {}

    private:
        typename traits::RenderDetailsReferenceType m_renderDetailsReference;

        DrawObjReference m_nextDrawObjReference;
        std::unordered_map<DrawObjReference, std::shared_ptr<DrawObject<traits>>> m_drawObjects;
        std::unordered_set<DrawObjReference> m_objsIndicesWithOverridingRenderDetails;
        std::unordered_set<DrawObjReference> m_objsIndicesWithGlobalRenderDetails;
        std::set<ZValueReference> m_zValueReferernces;
    };
}
#endif // AMAZING_LABYRINTH_DRAW_OBJECT_TABLE_HPP
