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
#include <glm/glm.hpp>
#include <boost/optional.hpp>
#include "../../renderDetails/renderDetails.hpp"

namespace levelDrawer {
    template<typename traits>
    class DrawObject {
    public:
        bool hasOverridingRenderDetailsReference() {
            return m_renderDetailsReference.renderDetails != nullptr;
        }

        typename traits::RenderDetailsReferenceType const &renderDetailsReference() {
            return m_renderDetailsReference;
        }

        std::shared_ptr<typename traits::ModelDataType> const &modelData() { return m_modelData; }

        std::shared_ptr<typename traits::TextureDataType> const &textureData() { return m_textureData; }

        std::shared_ptr<typename traits::DrawObjectDataType> const &objData(size_t index) {
            if (index >= m_objsData.size()) {
                throw std::runtime_error("Invalid draw object data index");
            }

            return m_objsData[index];
        }

        std::shared_ptr<typename traits::DrawObjectDataType> const &endObjData() {
            return m_objsData[m_objsData.size() - 1];
        }

        void addObjectData(std::shared_ptr<typename traits::DrawObjectDataType> objectData) {
            m_objsData.push_back(objectData);
        }

        void updateObjectData(size_t objDataIndex, glm::mat4 const &modelMatrix) {
            m_objsData[objDataIndex]->update(modelMatrix);
        }

        void trimBack(size_t newNbrObjects) {
            if (newNbrObjects < m_objsData.size()) {
                m_objsData.resize(newNbrObjects);
            }
        }

        size_t numberObjectsData() const {
            return m_objsData.size();
        }

        DrawObject(
                typename traits::RenderDetailsReferenceType renderDetailsReference_,
                std::shared_ptr<typename traits::ModelDataType> modelData_,
                std::shared_ptr<typename traits::TextureDataType> textureData_)
                : m_renderDetailsReference{std::move(renderDetailsReference_)},
                  m_modelData{std::move(modelData_)},
                  m_textureData{textureData_} {}

        DrawObject(
                std::shared_ptr<typename traits::ModelDataType> modelData_,
                std::shared_ptr<typename traits::TextureDataType> textureData_)
                : m_renderDetailsReference{},
                  m_modelData{modelData_},
                  m_textureData{textureData_} {}

    private:
        typename traits::RenderDetailsReferenceType m_renderDetailsReference;
        std::shared_ptr<typename traits::ModelDataType> m_modelData;
        std::shared_ptr<typename traits::TextureDataType> m_textureData;
        std::vector<std::shared_ptr<typename traits::DrawObjectDataType>> m_objsData;
    };

    template <typename traits>
    class DrawObjectTable {
    public:
        struct DrawRule {
            std::shared_ptr<typename traits::RenderDetailsType> renderDetails;
            std::shared_ptr<typename traits::CommonObjectDataType> commonObjectData;
            std::vector<size_t> drawObjectIndices;
        };

        bool emptyOfDrawObjects() {
            return m_drawObjects.empty();
        }

        void clear() {
            m_renderDetailsReference = typename traits::RenderDetailsReferenceType{};
            m_drawObjects.clear();
            m_objsIndicesWithOverridingRenderDetails.clear();
            m_objsIndicesWithGlobalRenderDetails.clear();
        }

        // returns index of added object.
        size_t addObject(
                std::shared_ptr<typename traits::ModelDataType> modelData,
                std::shared_ptr<typename traits::TextureDataType> textureData)
        {
            m_drawObjects.emplace_back(std::make_shared<DrawObject<traits>>(
                    std::move(modelData), std::move(textureData)));

            size_t newObjectIndex = m_drawObjects.size() - 1;
            m_objsIndicesWithGlobalRenderDetails.push_back(newObjectIndex);
            return newObjectIndex;
        }

        // returns index of added object.
        size_t addObject(
                typename traits::RenderDetailsReferenceType renderDetailsReference,
                std::shared_ptr<typename traits::ModelDataType> modelData,
                std::shared_ptr<typename traits::TextureDataType> textureData)
        {
            size_t newObjectIndex = m_drawObjects.size();

            if (renderDetailsReference.renderDetails != nullptr) {
                m_objsIndicesWithOverridingRenderDetails.push_back(newObjectIndex);
                m_drawObjects.emplace_back(std::make_shared<DrawObject<traits>>(
                        std::move(renderDetailsReference), std::move(modelData),
                        std::move(textureData)));
            } else {
                m_objsIndicesWithGlobalRenderDetails.push_back(newObjectIndex);
                m_drawObjects.emplace_back(std::make_shared<DrawObject<traits>>(
                        std::move(modelData), std::move(textureData)));
            }


            return newObjectIndex;
        }

        // returns index of added object.
        size_t addObject(
                std::shared_ptr<typename traits::RenderDetailsReferenceType> renderDetailsReference,
                std::shared_ptr<typename traits::ModelDataType> modelData,
                std::shared_ptr<typename traits::TextureDataType> textureData,
                std::vector<std::shared_ptr<typename traits::DrawObjectDataType>> objsData) {
            m_drawObjects.emplace_back(std::make_shared<DrawObject<traits>>(
                    std::move(renderDetailsReference), modelData, textureData,
                    std::move(objsData)));

            size_t newObjectIndex = m_drawObjects.size() - 1;
            if (renderDetailsReference != nullptr) {
                m_objsIndicesWithOverridingRenderDetails.push_back(newObjectIndex);
            } else {
                m_objsIndicesWithGlobalRenderDetails.push_back(newObjectIndex);
            }
            return newObjectIndex;
        }

        void removeObject(size_t objIndex) {
            if (m_drawObjects[objIndex]->hasOverridingRenderDetailsReference()) {
                for (auto it = m_objsIndicesWithOverridingRenderDetails.begin(); it != m_objsIndicesWithOverridingRenderDetails.end(); it++) {
                    if (*it == objIndex) {
                        m_objsIndicesWithOverridingRenderDetails.erase(it);
                    }
                }
            } else {
                for (auto it = m_objsIndicesWithGlobalRenderDetails.begin(); it != m_objsIndicesWithGlobalRenderDetails.end(); it++) {
                    if (*it == objIndex) {
                        m_objsIndicesWithGlobalRenderDetails.erase(it);
                    }
                }
            }

            m_drawObjects.erase(m_drawObjects.begin() + objIndex);
        }

        std::vector<size_t>
        objsIndicesWithOverridingRenderDetails() { return m_objsIndicesWithOverridingRenderDetails; }

        std::vector<size_t>
        objsIndicesWithGlobalRenderDetails() { return m_objsIndicesWithGlobalRenderDetails; }

        typename traits::RenderDetailsReferenceType const &renderDetailsReference() {
            return m_renderDetailsReference;
        }

        size_t numberObjects() { return m_drawObjects.size(); }

        size_t numberObjectsDataForObject(size_t drawObjectIndex) {
            if (drawObjectIndex >= m_drawObjects.size()) {
                throw std::runtime_error("Invalid draw object index.");
            }

            return m_drawObjects[drawObjectIndex]->numberObjectsData();
        }

        std::shared_ptr<DrawObject<traits>> const &drawObject(
                size_t drawObjectIndex) {
            if (drawObjectIndex >= m_drawObjects.size()) {
                throw std::runtime_error("Invalid draw object index.");
            }

            return m_drawObjects[drawObjectIndex];
        }

        // returns true if the move was successful
        bool transferObject(size_t objIndex1, size_t objIndex2) {
            if (objIndex1 >= m_drawObjects.size() || objIndex2 >= m_drawObjects.size()) {
                return false;
            }
            auto &obj1 = m_drawObjects[objIndex1];
            auto &obj2 = m_drawObjects[objIndex2];

            if ((obj1->renderDetailsReference() == nullptr &&
                 obj2->renderDetailsReference() == nullptr) ||
                (obj1->renderDetailsReference() != nullptr &&
                 obj2->renderDetailsReference() != nullptr &&
                 obj1->renderDetailsReference().renderDetails->name() ==
                 obj2->renderDetailsReference().renderDetails->name())) {
                // object is of the same render details, it is ok to move.
                std::shared_ptr<typename traits::DrawObjectDataType> objData = obj1->endObjData();
                obj1->trimBack(obj1->numberObjsData() - 1);
                objData->updateTextureData(obj2->textureData());
                obj2->addObjectData(objData);
                return true;
            }

            return false;
        }

        void
        updateObjectData(size_t objectIndex, size_t objectDataIndex, glm::mat4 const &modelMatrix) {
            m_drawObjects[objectIndex]->updateObjectData(objectDataIndex, modelMatrix);
        }

        void loadRenderDetails(typename traits::RenderDetailsReferenceType ref) {
            m_renderDetailsReference = std::move(ref);
        }

        std::vector<DrawRule> getDrawRules() {
            std::vector<DrawRule> rules;
            rules.emplace_back(DrawRule{m_renderDetailsReference.renderDetails,
                    m_renderDetailsReference.commonObjectData,
                    m_objsIndicesWithGlobalRenderDetails});

            std::map<std::string, DrawRule> alreadyAdded;
            for (auto index : m_objsIndicesWithOverridingRenderDetails) {
                auto const &ref = m_drawObjects[index]->renderDetailsReference();
                auto ret = alreadyAdded.emplace(ref.renderDetails->nameString(),
                        DrawRule{ref.renderDetails, ref.commonObjectData, std::vector<size_t>{}});
                ret.first->second.drawObjectIndices.push_back(index);
            }

            for (auto item : alreadyAdded) {
                rules.push_back(item.second);
            }

            return std::move(rules);
        }
    private:
        typename traits::RenderDetailsReferenceType m_renderDetailsReference;

        std::vector<std::shared_ptr<DrawObject<traits>>> m_drawObjects;
        std::vector<size_t> m_objsIndicesWithOverridingRenderDetails;
        std::vector<size_t> m_objsIndicesWithGlobalRenderDetails;
    };
}
#endif // AMAZING_LABYRINTH_DRAW_OBJECT_TABLE_HPP
