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
#ifndef AMAZING_LABYRINTH_LEVEL_DRAWER_GRAPHICS_HPP
#define AMAZING_LABYRINTH_LEVEL_DRAWER_GRAPHICS_HPP

#include <memory>
#include <string>
#include <array>
#include <stdexcept>

#include <glm/glm.hpp>

#include "modelTable/modelLoader.hpp"
#include "textureTable/textureLoader.hpp"
#include "../renderDetails/renderDetails.hpp"
#include "drawObjectTable/drawObjectTable.hpp"

#include "levelDrawer.hpp"

namespace levelDrawer {
    template<typename traits>
    class LevelDrawerGraphics : public LevelDrawer {
    public:
        static size_t constexpr m_numberDrawObjectTables = 3;

        using DrawObjectTableList =
            std::array<std::shared_ptr<typename traits::DrawObjectTableType>, m_numberDrawObjectTables>;
        using DrawObjRefsForDrawList = std::array<std::vector<DrawObjReference>, m_numberDrawObjectTables>;
        using CommonObjectDataList =
            std::array<std::shared_ptr<typename traits::CommonObjectDataType>, m_numberDrawObjectTables>;

        bool emptyOfDrawObjects(ObjectType type) override {
            return m_drawObjectTableList[type]->emptyOfDrawObjects();
        }

        size_t numberObjects(ObjectType type) override {
            return m_drawObjectTableList[type]->numberObjects();
        }

        void clearDrawObjectTable(ObjectType type) override {
            m_drawObjectTableList[type]->clear();

            // We don't need to clear empty model and texture references out more than once during
            // the level cycle, they don't reference much data once they become empty.  Just clear
            // them out when switching to the new level, not the level starter or finisher.
            if (type == LEVEL) {
                m_modelTable.prune();
                m_textureTable.prune();
            }
        }

        DrawObjReference addObject(
                ObjectType type,
                std::shared_ptr<ModelDescription> const &modelDescription,
                std::shared_ptr<TextureDescription> const &textureDescription) override
        {
            std::shared_ptr<typename traits::ModelDataType> modelData = m_modelTable.addModel(
                    m_gameRequester, modelDescription);
            std::shared_ptr<typename traits::TextureDataType> textureData{};

            if (textureDescription) {
                textureData = m_textureTable.addTexture(m_gameRequester, textureDescription);
            }

            return m_drawObjectTableList[type]->addObject(modelData, textureData);

        }

        DrawObjReference addObject(
                ObjectType type,
                std::shared_ptr<ModelDescription> const &modelDescription,
                std::shared_ptr<TextureDescription> const &textureDescription,
                std::string const &renderDetailsName,
                std::shared_ptr<renderDetails::Parameters> const &parameters) override {
            std::shared_ptr<typename traits::ModelDataType> modelData = m_modelTable.addModel(
                    m_gameRequester, modelDescription);
            std::shared_ptr<typename traits::TextureDataType> textureData{};

            if (textureDescription) {
                textureData = m_textureTable.addTexture(m_gameRequester, textureDescription);
            }

            return m_drawObjectTableList[type]->addObject(
                m_renderLoader->load(m_gameRequester, renderDetailsName, m_surfaceDetails, parameters),
                modelData,
                textureData);
        }

        void removeObject(
                ObjectType type,
                DrawObjReference drawObjReference) override
        {
            m_drawObjectTableList[type]->removeObject(drawObjReference);
        }

        DrawObjDataReference addModelMatrixForObject(
                ObjectType type,
                DrawObjReference drawObjReference,
                glm::mat4 const &modelMatrix) override
        {
            return addModelMatrixToDrawObjTable(
                    m_drawObjectTableList[type], drawObjReference, modelMatrix);
        }

        void updateModelMatrixForObject(
                ObjectType type,
                DrawObjReference objIndex,
                DrawObjDataReference objDataIndex,
                glm::mat4 const &modelMatrix) override
        {
            m_drawObjectTableList[type]->updateObjectData(objIndex, objDataIndex, modelMatrix);
        }

        boost::optional<DrawObjDataReference> transferObject(
                ObjectType type,
                DrawObjReference fromObjRef,
                DrawObjDataReference objDataRef,
                DrawObjReference toObjRef) override
        {
            return m_drawObjectTableList[type]->transferObject(fromObjRef, objDataRef, toObjRef);
        }

        void removeObjectData(
                ObjectType type,
                DrawObjReference objRef,
                DrawObjDataReference objDataRef) override
        {
            m_drawObjectTableList[type]->removeObjectData(objRef, objDataRef);
        }

        size_t numberObjectsDataForObject(ObjectType type, DrawObjReference drawObjReference) override {
            return m_drawObjectTableList[type]->numberObjectsDataForObject(drawObjReference);
        }

        void requestRenderDetails(ObjectType type, std::string const &name, std::shared_ptr<renderDetails::Parameters> const &parameters) override {
            m_drawObjectTableList[type]->loadRenderDetails(
                    m_renderLoader->load(m_gameRequester, name, m_surfaceDetails, parameters));
        }

        std::pair<glm::mat4, glm::mat4> getProjectionView(ObjectType type) override {
            auto globalRef = m_drawObjectTableList[type]->renderDetailsReference();
            if (globalRef.renderDetails == nullptr) {
                throw std::runtime_error("Render details reference not set for level type");
            }

            return globalRef.getProjViewForLevel();
        }

        void setClearColor(ObjectType type, glm::vec4 const &clearColor) override {
            // only allow setting of the clear color if we are a level, ignore other requests.
            if (type == LEVEL) {
                m_bgColor = clearColor;
            }
        }

        void draw(typename traits::DrawArgumentType const &info);

        // all of the objects that are passed in should be using the same COD and renderDetails
        void updateCommonObjectData(ObjectType type,
                                  DrawObjReference const &objRef,
                                  renderDetails::Parameters const &parameters) override {
            auto ref = m_drawObjectTableList[type]->renderDetailsReference(objRef);
            ref.commonObjectData->update(parameters);
        }

        char const *getDefaultRenderDetailsName() override { return m_defaultRenderDetailsName; }

        void drawToBuffer(
            std::string const &renderDetailsName,
            ModelsTextures const &modelsTextures,
            std::vector<glm::mat4> const &modelMatrix,
            float width,
            float height,
            uint32_t nbrSamplesForWidth,
            std::shared_ptr<renderDetails::Parameters> const &parameters,
            std::vector<float> &results) override;

        LevelDrawerGraphics(typename traits::NeededForDrawingType neededForDrawing,
                            std::shared_ptr<typename traits::SurfaceDetailsType> inSurfaceDetails,
                            std::shared_ptr<typename traits::RenderLoaderType> inRenderLoader,
                            char const *defaultRenderDetailsName,
                            std::shared_ptr<GameRequester> inGameRequester);

        ~LevelDrawerGraphics() override = default;
    private:
        using ExecuteDraw = std::function<void(
                std::shared_ptr<typename traits::RenderDetailsType> const &,
                std::shared_ptr<renderDetails::CommonObjectData> const &,
                std::shared_ptr<typename traits::DrawObjectTableType> const &,
                std::set<ZValueReference>::iterator,
                std::set<ZValueReference>::iterator)>;

        typename traits::ModelTableType m_modelTable;
        typename traits::TextureTableType m_textureTable;
        DrawObjectTableList m_drawObjectTableList;
        std::shared_ptr<typename traits::RenderLoaderType> m_renderLoader;
        std::shared_ptr<GameRequester> m_gameRequester;
        typename traits::NeededForDrawingType m_neededForDrawing;
        std::shared_ptr<typename traits::SurfaceDetailsType> m_surfaceDetails;
        glm::vec4 m_bgColor;
        char const *m_defaultRenderDetailsName;

        DrawObjReference addModelMatrixToDrawObjTable(
                std::shared_ptr<typename traits::DrawObjectTableType> const &drawObjTable,
                DrawObjReference objReference,
                glm::mat4 const &modelMatrix)
        {
            auto drawObj = drawObjTable->drawObject(objReference);

            auto renderDetailsRef = drawObj->renderDetailsReference();

            std::shared_ptr<typename traits::TextureDataType> textureData = drawObj->textureData();

            std::shared_ptr<typename traits::DrawObjectDataType> objData;
            if (renderDetailsRef.renderDetails != nullptr) {
                // we need to add new objects data.
                objData = renderDetailsRef.createDrawObjectData(nullptr, textureData, modelMatrix);
            } else {
                auto ref = drawObjTable->renderDetailsReference();

                if (ref.renderDetails == nullptr) {
                    throw std::runtime_error("Render details not initialized!");
                }

                objData = ref.createDrawObjectData(nullptr, textureData, modelMatrix);
            }

            return drawObjTable->addDrawObjData(objReference, objData);
        }

        auto getRenderDetailsAndCODList()
        {
            std::unordered_map<std::string, std::pair<std::shared_ptr<typename traits::RenderDetailsType>, std::array<std::shared_ptr<renderDetails::CommonObjectData>, nbrDrawObjectTables>>> ret;
            std::pair<std::shared_ptr<typename traits::RenderDetailsType>, std::array<std::shared_ptr<renderDetails::CommonObjectData>, nbrDrawObjectTables>> value;
            for (size_t i = 0; i < nbrDrawObjectTables; i++) {
                if (m_drawObjectTableList[i]->emptyOfDrawObjects()) {
                    continue;
                }
                auto &ref = m_drawObjectTableList[i]->renderDetailsReference();
                auto result = ret.emplace(ref.renderDetails->nameString(), value);
                if (result.second) {
                    result.first->second.first = ref.renderDetails;
                }
                result.first->second.second[i] = ref.commonObjectData;

                for (auto const &index : m_drawObjectTableList[i]->objsIndicesWithOverridingRenderDetails()) {
                    auto &ref2 = m_drawObjectTableList[i]->renderDetailsReference(index);
                    auto result2 = ret.emplace(ref.renderDetails->nameString(), value);
                    if (result2.second) {
                        result2.first->second.first = ref.renderDetails;
                    }
                    result2.first->second.second[i] = ref.commonObjectData;
                }
            }

            return ret;
        }

        void performDraw(ExecuteDraw executeDraw)
        {
            for (auto table : std::vector<ObjectType>{LEVEL, STARTER, FINISHER}) {
                if (m_drawObjectTableList[table]->emptyOfDrawObjects()) {
                    continue;
                }
                typename traits::RenderDetailsReferenceType currentRenderDetails =
                        m_drawObjectTableList[table]->renderDetailsReference();
                std::string defaultRenderDetailsName = currentRenderDetails.renderDetails->nameString();
                bool isDefaultRenderDetailsReference = true;
                auto tableEnd = m_drawObjectTableList[table]->zValueReferences().end();
                auto itBegin = m_drawObjectTableList[table]->zValueReferences().begin();
                auto itEnd = itBegin;
                // find a continuous set of draw objects that use the same render details and call
                // execute draw on them.  Also, draw back to front so that objects with an alpha
                // channel that is not 1 will partially show the objects beneath them.
                while (true) {
                    if (itEnd == tableEnd) {
                        if (itBegin != itEnd) {
                            executeDraw(
                                    currentRenderDetails.renderDetails,
                                    currentRenderDetails.commonObjectData,
                                    m_drawObjectTableList[table], itBegin, itEnd);
                        }
                        break;
                    }

                    auto const &drawObj = m_drawObjectTableList[table]->drawObject(itEnd->drawObjectReference);
                    if (isDefaultRenderDetailsReference && !drawObj->hasOverridingRenderDetailsReference()) {
                        itEnd++;
                        continue;
                    }

                    auto renderDetailsRef = m_drawObjectTableList[table]->renderDetailsReference(itEnd->drawObjectReference);
                    if (renderDetailsRef.renderDetails->nameString() == currentRenderDetails.renderDetails->nameString()) {
                        itEnd++;
                        continue;
                    }

                    if (itBegin != itEnd) {
                        executeDraw(
                                currentRenderDetails.renderDetails,
                                currentRenderDetails.commonObjectData,
                                m_drawObjectTableList[table], itBegin, itEnd);
                    }

                    if (isDefaultRenderDetailsReference) {
                        isDefaultRenderDetailsReference = false;
                    } else {
                        isDefaultRenderDetailsReference = (renderDetailsRef.renderDetails->nameString() == defaultRenderDetailsName);
                    }
                    currentRenderDetails = renderDetailsRef;
                    itBegin = itEnd;
                    itEnd++;
                }
            }
        }
    };

} // namespace levelDrawer
#endif // AMAZING_LABYRINTH_LEVEL_DRAWER_GRAPHICS_HPP
