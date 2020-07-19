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

#include "levelDrawer.hpp"

namespace levelDrawer {
    template<typename traits>
    class LevelDrawerGraphics : public LevelDrawer {
    public:
        static size_t constexpr m_numberDrawObjectTables = 3;

        using DrawObjectTableList =
            std::array<std::shared_ptr<typename traits::DrawObjectTableType>, m_numberDrawObjectTables>;
        using IndicesForDrawList = std::array<std::vector<size_t>, m_numberDrawObjectTables>;
        using CommonObjectDataList =
            std::array<std::shared_ptr<typename traits::CommonObjectDataType>, m_numberDrawObjectTables>;

        void clearDrawObjectTable(ObjectType type) {
            m_drawObjectTableList[type]->clear();
        }

        size_t addObject(
                ObjectType type,
                std::shared_ptr<ModelDescription> const &modelDescription,
                std::shared_ptr<TextureDescription> const &textureDescription) override
        {
            std::shared_ptr<typename traits::ModelDataType> modelData = m_modelTable.addModel(
                    modelDescription);
            std::shared_ptr<typename traits::TextureDataType> textureData{};

            if (textureDescription) {
                textureData = m_textureTable.addTexture(textureDescription);
            }

            m_drawObjectTableList[type].addObject(modelData, textureData);
        }

        size_t addObject(
                ObjectType type,
                std::shared_ptr<ModelDescription> const &modelDescription,
                std::shared_ptr<TextureDescription> const &textureDescription,
                std::string const &renderDetailsName) override {
            std::shared_ptr<typename traits::ModelDataType> modelData = m_modelTable.addModel(
                    modelDescription);
            std::shared_ptr<typename traits::TextureDataType> textureData{};

            if (textureDescription) {
                textureData = m_textureTable.addTexture(textureDescription);
            }

            m_drawObjectTableList[type]->addObject(
                m_renderLoader->load(
                        m_gameRequester,
                        renderDetailsName,
                        m_gameRequester->getParametersForRenderDetailsName(renderDetailsName)),
                modelData,
                textureData);
        }

        size_t addModelMatrixForObject(ObjectType type, size_t objsIndex, glm::mat4 const &modelMatrix) override {
            std::shared_ptr<typename traits::DrawObjectType> drawObj =
                    m_drawObjectTableList[type]->drawObject(objsIndex);

            auto renderDetailsRef = drawObj->renderDetailsReference();

            std::shared_ptr<typename traits::TextureDataType> textureData = drawObj->textureData();

            std::shared_ptr<typename traits::DrawObjectDataType> objData;
            if (renderDetailsRef.renderDetails != nullptr) {
                // we need to add new objects data.
                 objData = renderDetailsRef.createDrawObjectData(textureData, modelMatrix);
            } else {
                auto ref = m_drawObjectTableList[type]->renderDetailsReference();

                if (ref.renderDetails == nullptr) {
                    throw std::runtime_error("Render details not initialized!");
                }

                objData = ref.createDrawObjectData(textureData, modelMatrix);
            }

            drawObj->addObjectData(objData);

            return drawObj->numberObjectsData() - 1;
        }

        void updateModelMatrixForObject(
                ObjectType type,
                size_t objIndex,
                size_t objDataIndex,
                glm::mat4 const &modelMatrix) override
        {
            m_drawObjectTableList[type]->updateObjectData(objIndex, objDataIndex, modelMatrix);
        }

        void resizeObjectsData(ObjectType type, size_t objsIndex, size_t newSize) override {
            std::shared_ptr<typename traits::DrawObjectType> drawObj =
                    m_drawObjectTableList[type]->drawObject(objsIndex);

            size_t nbrObjsData = drawObj->numberObjectsData();
            if (nbrObjsData == newSize) {
                return;
            } else if (nbrObjsData > newSize) {
                drawObj->trimBack(newSize);
                return;
            }

            auto overridingRef = drawObj->renderDetailsReference();
            auto globalRef = m_drawObjectTableList[type]->renderDetailsReference();

            std::shared_ptr<typename traits::TextureDataType> textureData = drawObj->textureData();
            // we need to add new objects data.
            while (nbrObjsData < newSize) {
                std::shared_ptr<typename traits::DrawObjectDataType> objData;
                if (overridingRef.renderDetails != nullptr) {
                    objData = overridingRef(textureData, glm::mat4(1.0f));
                } else {
                    objData = globalRef(textureData, glm::mat4(1.0f));
                }
                drawObj->addObjectData(objData);
                nbrObjsData++;
            }
        }

        size_t numberObjectsDataForObject(ObjectType type, size_t objsIndex) override {
            return m_drawObjectTableList[type]->numberObjectsDataForObject(objsIndex);
        }

        void requestRenderDetails(ObjectType type, std::string const &name) override {
            m_drawObjectTableList[type]->loadRenderDetails(m_renderLoader.load(m_gameRequester, name,
                    m_gameRequester->getParametersForRenderDetailsName(name)));
        }

        std::pair<glm::mat4, glm::mat4> getProjectionView(ObjectType type) override {
            auto globalRef = m_drawObjectTableList[type]->renderDetailsReference();
            if (globalRef.renderDetails == nullptr) {
                throw std::runtime_error("Render details reference not set for level type");
            }

            return globalRef.getProjViewForLevel();
        }

        void draw(typename traits::DrawArgumentType info);

        void drawToBuffer(
            std::string const &renderDetailsName,
            std::vector<std::pair<std::shared_ptr<ModelDescription>, std::shared_ptr<TextureDescription>>> modelTexture,
            float width,
            float height,
            uint32_t nbrSamplesForWidth,
            float farthestDepth,
            float nearestDepth,
            std::vector<float> &results);

        LevelDrawerGraphics(typename traits::NeededForDrawingType neededForDrawing,
                            std::shared_ptr<typename traits::RenderLoaderType> inRenderLoader,
                            std::shared_ptr<GameRequester> inGameRequester);

        ~LevelDrawerGraphics() override = default;
    private:
        struct DrawRules {
            std::shared_ptr<typename traits::RenderDetailsType> renderDetails;
            std::array<std::shared_ptr<typename traits::CommonObjectDataType>, m_numberDrawObjectTables> commonObjectDataList;
            std::array<std::vector<size_t>, m_numberDrawObjectTables> indicesPerLevelType;
        };

        typename traits::ModelTableType m_modelTable;
        typename traits::TextureTableType m_textureTable;
        DrawObjectTableList m_drawObjectTableList;
        std::shared_ptr<typename traits::RenderLoaderType> m_renderLoader;
        std::shared_ptr<GameRequester> m_gameRequester;
        typename traits::NeededForDrawingType m_neededForDrawing;
        glm::vec4 m_bgColor;

        std::vector<DrawRules> getDrawRules() {
            std::map<std::string, DrawRules> rulesGroup{};
            for (size_t i = 0; i < m_numberDrawObjectTables; i++) {
                auto rules = m_drawObjectTableList[i].getDrawRules();

                for (auto const &rule : rules) {
                    std::array<std::pair<std::shared_ptr<typename traits::CommonObjectData>, std::vector<size_t>>, m_numberDrawObjectTables> drawRulesLevelList{};
                    auto insertResult = rulesGroup.emplace(rule.renderDetails, drawRulesLevelList);

                    insertResult.first->second.commonObjectDataList[i] = rule.commonObjectData;
                    insertResult.first->second.indicesPerLevelType[i] = rule.drawObjectIndices;
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
