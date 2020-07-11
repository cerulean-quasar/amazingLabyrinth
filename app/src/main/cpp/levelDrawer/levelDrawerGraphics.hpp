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

#include <glm/glm.hpp>

#include "modelTable/modelLoader.hpp"
#include "textureTable/textureLoader.hpp"
#include "../renderDetails/renderDetails.hpp"

#include "levelDrawer.hpp"
#include "textureTable/textureTableVulkan.hpp"

namespace levelDrawer {
    template<typename traits>
    class LevelDrawerGraphics : public LevelDrawer {
    public:
        enum ObjectType {
            STARTER,
            LEVEL,
            FINISHER
        };

        size_t addObject(
                ObjectType type,
                std::shared_ptr<ModelDescription> const &modelDescription,
                std::shared_ptr<TextureDescription> const &textureDescription)
        {
            std::shared_ptr<traits::ModelDataType> modelData = m_modelTable.addModel(
                    modelDescription);
            std::shared_ptr<traits::TextureDataType> textureData{};

            if (textureDescription) {
                textureData = m_textureTable.addTexture(textureDescription);
            }

            switch (type) {
            case ObjectType::STARTER:
                m_drawObjectTableStarter.addObject(modelData, textureData);
                break;
            case ObjectType::LEVEL:
                m_drawObjectTableStarter.addObject(modelData, textureData);
                break;
            case ObjectType::FINISHER:
            default:
                m_drawObjectTableStarter.addObject(modelData, textureData);
            }
        }

        size_t addObject(
                ObjectType type,
                std::shared_ptr<ModelDescription> const &modelDescription,
                std::shared_ptr<TextureDescription> const &textureDescription,
                std::string const &renderDetailsName) {
            std::shared_ptr<traits::ModelDataType> modelData = m_modelTable.addModel(
                    modelDescription);
            std::shared_ptr<traits::TextureDataType> textureData{};

            if (textureDescription) {
                textureData = m_textureTable.addTexture(textureDescription);
            }

            switch (type) {
            case ObjectType::STARTER:
                m_drawObjectTableStarter.addObject(
                        m_renderLoader->load(m_gameRequester, renderDetailsName, m_renderDetailsParameters),
                        modelData,
                        textureData);
                break;
            case ObjectType::LEVEL:
                m_drawObjectTableLevel.addObject(
                        m_renderLoader->load(m_gameRequester, renderDetailsName, m_renderDetailsParameters),
                        modelData,
                        textureData);
                break;
            case ObjectType::FINISHER:
            default:
                m_drawObjectTableFinisher.addObject(
                        m_renderLoader->load(m_gameRequester, renderDetailsName, m_renderDetailsParameters),
                        modelData,
                        textureData);
            }
        }

        size_t addModelMatrixForObject(ObjectType type, size_t objsIndex, glm::mat4 const &modelMatrix) override {
            std::shared_ptr<typename traits::DrawObjectType> drawObj;

            switch (type) {
            case ObjectType::STARTER:
                drawObj = m_drawObjectTableStarter.drawObject(objsIndex);
                break;
            case ObjectType::LEVEL:
                drawObj = m_drawObjectTableLevel.drawObject(objsIndex);
                break;
            case ObjectType::FINISHER:
                drawObj = m_drawObjectTableFinisher.drawObject(objsIndex);
                break;
            }

            auto renderDetailsRef = drawObj->renderDetailsReference();

            auto const &renderDetails = renderDetailsRef.renderDetails ?
                                        renderDetailsRef.renderDetails :
                                        m_renderDetailsReference.renderDetails;

            if (renderDetails == nullptr) {
                throw runtime_error("Render details not initialized!");
            }

            std::shared_ptr<TextureDataVulkan> textureData = drawObj->textureData();

            // we need to add new objects data.
            std::shared_ptr<typename traits::DrawObjectDataType> objData =
                    renderDetails->createDrawObject(textureData, glm::mat4(1.0f));

            drawObj->addObjectData(objData);

            return drawObj->numberObjectsData() - 1;
        }

        void resizeObjectsData(ObjectType type, size_t objsIndex, size_t newSize) override {
            std::shared_ptr<typename traits::DrawObjectType> drawObj;

            switch (type) {
            case ObjectType::STARTER:
                drawObj = m_drawObjectTableStarter.drawObject(objsIndex);
                break;
            case ObjectType::LEVEL:
                drawObj = m_drawObjectTableLevel.drawObject(objsIndex);
                break;
            case ObjectType::FINISHER:
                drawObj = m_drawObjectTableFinisher.drawObject(objsIndex);
                break;
            }

            size_t nbrObjsData = drawObj->numberObjectsData();
            if (nbrObjsData == newSize) {
                return;
            } else if (nbrObjsData > newSize) {
                drawObj->trimBack(newSize);
                return;
            }

            auto renderDetailsRef = drawObj->renderDetailsReference();

            auto const &renderDetails = renderDetailsRef.renderDetails ?
                                        renderDetailsRef.renderDetails :
                                        m_renderDetailsReference.renderDetails;

            if (renderDetails == nullptr) {
                throw runtime_error("Render details not initialized!");
            }

            std::shared_ptr<TextureDataVulkan> textureData = drawObj->textureData();
            // we need to add new objects data.
            while (nbrObjsData < newSize) {
                std::shared_ptr<typename traits::DrawObjectDataType> objData =
                        renderDetails->createDrawObject(textureData, glm::mat4(1.0f));
                drawObj->addObjectData(objData);
            }
        }

        size_t numberObjectsDataForObject(ObjectType type, size_t objsIndex) override {
            std::shared_ptr<typename traits::DrawObjectType> drawObj;

            switch (type) {
            case ObjectType::STARTER:
                return m_drawObjectTableStarter.numberObjectsDataForObject(objsIndex);
            case ObjectType::LEVEL:
                return m_drawObjectTableLevel.numberObjectsDataForObject(objsIndex);
            case ObjectType::FINISHER:
            default:
                return m_drawObjectTableFinisher.numberObjectsDataForObject(objsIndex);
            }
        }

        void requestRenderDetails(std::string const &name) override {
            m_renderDetailsReference = m_renderLoader.load(name, m_renderDetailsParameters);
        }

        virtual void draw(typename traits::DrawArgumentType info) = 0;

    private:
        typename traits::RenderDetailsReferenceType m_renderDetailsReference;
        typename traits::ModelTableType m_modelTable;
        typename traits::TextureTableType m_textureTable;
        typename traits::DrawObjectTableType m_drawObjectTableStarter;
        typename traits::DrawObjectTableType m_drawObjectTableLevel;
        typename traits::DrawObjectTableType m_drawObjectTableFinisher;
        typename traits::RenderLoaderType m_renderLoader;
        typename traits::RenderDetailsParameters m_renderDetailsParameters;
        std::shared_ptr<GameRequester> m_gameRequester;
    };
}
#endif // AMAZING_LABYRINTH_LEVEL_DRAWER_GRAPHICS_HPP
