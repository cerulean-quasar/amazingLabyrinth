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
                std::string const &renderDetailsName) override {
            std::shared_ptr<typename traits::ModelDataType> modelData = m_modelTable.addModel(
                    m_gameRequester, modelDescription);
            std::shared_ptr<typename traits::TextureDataType> textureData{};

            if (textureDescription) {
                textureData = m_textureTable.addTexture(m_gameRequester, textureDescription);
            }

            return m_drawObjectTableList[type]->addObject(
                m_renderLoader->load(
                        m_gameRequester,
                        renderDetailsName,
                        m_gameRequester->getParametersForRenderDetailsName(renderDetailsName.c_str())),
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
                size_t objIndex,
                size_t objDataIndex,
                glm::mat4 const &modelMatrix) override
        {
            m_drawObjectTableList[type]->updateObjectData(objIndex, objDataIndex, modelMatrix);
        }

        void removeObjectData(
                ObjectType type,
                DrawObjReference objRef,
                DrawObjDataReference objDataRef)
        {
            m_drawObjectTableList[type]->removeObjectData(objRef, objDataRef);
        }

        size_t numberObjectsDataForObject(ObjectType type, DrawObjReference drawObjReference) override {
            return m_drawObjectTableList[type]->numberObjectsDataForObject(drawObjReference);
        }

        void requestRenderDetails(ObjectType type, std::string const &name) override {
            m_drawObjectTableList[type]->loadRenderDetails(
                    m_renderLoader->load(m_gameRequester, name,
                        m_gameRequester->getParametersForRenderDetailsName(name.c_str())));
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

        void drawToBuffer(
            std::string const &renderDetailsName,
            ModelsTextures const &modelsTextures,
            std::vector<glm::mat4> const &modelMatrix,
            float width,
            float height,
            uint32_t nbrSamplesForWidth,
            float farthestDepth,
            float nearestDepth,
            std::vector<float> &results) override;

        LevelDrawerGraphics(typename traits::NeededForDrawingType neededForDrawing,
                            std::shared_ptr<typename traits::RenderLoaderType> inRenderLoader,
                            std::shared_ptr<GameRequester> inGameRequester);

        ~LevelDrawerGraphics() override = default;
    private:
        struct DrawRules {
            std::shared_ptr<typename traits::RenderDetailsType> renderDetails;
            std::array<std::shared_ptr<typename traits::CommonObjectDataType>, m_numberDrawObjectTables> commonObjectData;
            std::array<std::vector<DrawObjReference>, m_numberDrawObjectTables> drawObjRefs;
        };

        typename traits::ModelTableType m_modelTable;
        typename traits::TextureTableType m_textureTable;
        DrawObjectTableList m_drawObjectTableList;
        std::shared_ptr<typename traits::RenderLoaderType> m_renderLoader;
        std::shared_ptr<GameRequester> m_gameRequester;
        typename traits::NeededForDrawingType m_neededForDrawing;
        glm::vec4 m_bgColor;

        DrawObjReference addModelMatrixToDrawObjTable(
                std::shared_ptr<typename traits::DrawObjectTableType> const &drawObjTable,
                size_t objsIndex,
                glm::mat4 const &modelMatrix)
        {
            auto drawObj = drawObjTable->drawObject(objsIndex);

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

            return drawObj->addObjectData(objData);
        }

        std::vector<DrawRules> getDrawRules() {
            std::map<std::string, DrawRules> rulesGroup{};
            for (size_t i = 0; i < m_numberDrawObjectTables; i++) {
                auto rules = m_drawObjectTableList[i]->getDrawRules();

                for (auto const &rule : rules) {
                    std::array<std::shared_ptr<renderDetails::CommonObjectData>, m_numberDrawObjectTables> codList{};
                    std::array<std::vector<DrawObjReference>, m_numberDrawObjectTables> indicesList{};
                    auto insertResult = rulesGroup.emplace(rule.renderDetails->nameString(),
                            DrawRules{rule.renderDetails, codList, indicesList});

                    insertResult.first->second.commonObjectData[i] = rule.commonObjectData;
                    insertResult.first->second.drawObjRefs[i] = rule.drawObjectIndices;
                }
            }

            std::vector<DrawRules> regroupedRules;
            for (auto const &item : rulesGroup) {
                regroupedRules.push_back(item.second);
            }

            return std::move(regroupedRules);
        }
    };

} // namespace levelDrawer
#endif // AMAZING_LABYRINTH_LEVEL_DRAWER_GRAPHICS_HPP
