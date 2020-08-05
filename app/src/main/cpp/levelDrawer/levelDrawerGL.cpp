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
#include <boost/variant.hpp>
#include "../graphicsGL.hpp"
#include "levelDrawerGL.hpp"
#include "levelDrawerGraphics.hpp"
#include "../renderDetails/renderDetailsGL.hpp"

namespace levelDrawer {
    template <>
    void LevelDrawerGraphics<LevelDrawerGLTraits>::draw(
            LevelDrawerGLTraits::DrawArgumentType const &info)
    {
        auto rdAndCodList = getRenderDetailsAndCODList();

        // execute the pre main draw commands.
        for (auto const &rdAndCod : rdAndCodList) {
            rdAndCod.second.first->preMainDraw(
                    0, rdAndCod.second.second, m_drawObjectTableList,
                    m_drawObjectTableList[0]->zValueReferences(),
                    m_drawObjectTableList[1]->zValueReferences(),
                    m_drawObjectTableList[2]->zValueReferences());
        }

        glViewport(0, 0, info.width, info.height);
        checkGraphicsError();

        // The clear background color
        glClearColor(m_bgColor.r, m_bgColor.g, m_bgColor.b, m_bgColor.a);
        checkGraphicsError();

        // The clear depth buffer
        glClearDepthf(1.0f);
        checkGraphicsError();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        checkGraphicsError();

        // execute the commands for the main draw.
        performDraw(ExecuteDraw{
            [] (std::shared_ptr<LevelDrawerGLTraits::RenderDetailsType> const &rd,
                    std::shared_ptr<renderDetails::CommonObjectData> const &cod,
                    std::shared_ptr<LevelDrawerGLTraits::DrawObjectTableType> const &drawObjTable,
                    std::set<ZValueReference>::iterator zValRefBegin,
                    std::set<ZValueReference>::iterator zValRefEnd) -> void {
                rd->draw(0, cod, drawObjTable, zValRefBegin, zValRefEnd);
            }
        });
    }

    template <>
    void LevelDrawerGraphics<LevelDrawerGLTraits>::drawToBuffer(
            std::string const &renderDetailsName,
            ModelsTextures const &modelsTextures,
            std::vector<glm::mat4> const &modelMatrix,
            float width,
            float height,
            uint32_t nbrSamplesForWidth,
            float farthestDepth,
            float nearestDepth,
            std::vector<float> &results)
    {
        if (modelsTextures.size() != modelMatrix.size()) {
            throw std::runtime_error("the number of models must match the number of model matrices");
        }

        auto drawObjTable = std::make_shared<DrawObjectTableGL>();

        uint32_t imageWidth = nbrSamplesForWidth;
        uint32_t imageHeight = static_cast<uint32_t>(std::floor((imageWidth * height)/width));

        renderDetails::ParametersWithWidthHeightAtDepthGL parameters{};
        parameters.width = imageWidth;
        parameters.height = imageHeight;
        parameters.widthAtDepth = width;
        parameters.heightAtDepth = height;
        parameters.nearestDepth = nearestDepth;
        parameters.farthestDepth = farthestDepth;

        // load the render details.
        auto ref = m_renderLoader->load(m_gameRequester, renderDetailsName,
                std::make_shared<renderDetails::ParametersWithWidthHeightAtDepthGL>(parameters));

        drawObjTable->loadRenderDetails(std::move(ref));

        // add the draw objects
        size_t i = 0;
        for (auto const &modelTexture : modelsTextures) {
            auto modelData = m_modelTable.addModel(m_gameRequester, modelTexture.first);
            std::shared_ptr<LevelDrawerGLTraits::TextureDataType> textureData{};
            if (modelTexture.second) {
                textureData = m_textureTable.addTexture(m_gameRequester, modelTexture.second);
            }
            auto objIndex = drawObjTable->addObject(modelData, textureData);
            addModelMatrixToDrawObjTable(drawObjTable, objIndex, modelMatrix[i++]);
        }

        // perform the commands to be executed before the main draw.
        DrawObjectTableList drawObjTableList = {nullptr, drawObjTable, nullptr};
        CommonObjectDataList commonObjectDataList = {nullptr, ref.commonObjectData, nullptr};
        ref.renderDetails->preMainDraw(
                0, commonObjectDataList, drawObjTableList, std::set<ZValueReference>{},
                drawObjTable->zValueReferences(), std::set<ZValueReference>{});

        graphicsGL::Framebuffer::ColorImageFormat colorImageFormat{GL_RGBA32UI, GL_RGBA_INTEGER,
                                                                   GL_UNSIGNED_INT};
        if (!m_neededForDrawing.useIntegerSurface) {
            colorImageFormat = {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE};
            //graphicsGL::Framebuffer::ColorImageFormat colorImageFormat{GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE};

        }

        std::vector<graphicsGL::Framebuffer::ColorImageFormat> colorImageFormats{colorImageFormat};
        graphicsGL::Framebuffer fb(imageWidth, imageHeight, colorImageFormats);

        glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo());
        checkGraphicsError();

        // set the viewport
        glViewport(0, 0, imageWidth, imageHeight);
        checkGraphicsError();

        glm::vec4 clearColor = m_bgColor;
        ref.renderDetails->overrideClearColor(clearColor);
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        checkGraphicsError();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        checkGraphicsError();

        ref.renderDetails->draw(0, ref.commonObjectData, drawObjTable,
                drawObjTable->zValueReferences().begin(), drawObjTable->zValueReferences().end());

        glFinish();
        checkGraphicsError();

        boost::variant<std::vector<float>, std::vector<uint8_t>> dataVariant;
        if (m_neededForDrawing.useIntegerSurface) {
            /* width * height * 4 color values each a char in size. */
            std::vector<float> data(static_cast<size_t>(imageWidth * imageHeight * 4), 0.0f);
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            checkGraphicsError();
            glReadPixels(0, 0, imageWidth, imageHeight, colorImageFormat.format, colorImageFormat.type, data.data());
            checkGraphicsError();
            dataVariant = std::move(data);
        } else {
            /* width * height * 4 color values each a char in size. */
            std::vector<uint8_t> data(static_cast<size_t>(imageWidth * imageHeight * 4), 0);
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            checkGraphicsError();
            glReadPixels(0, 0, imageWidth, imageHeight, colorImageFormat.format, colorImageFormat.type, data.data());
            checkGraphicsError();
            dataVariant = std::move(data);
        }
        ref.renderDetails->postProcessImageBuffer(ref.commonObjectData, dataVariant, results);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        checkGraphicsError();
    }

    template <>
    LevelDrawerGraphics<LevelDrawerGLTraits>::LevelDrawerGraphics(
            LevelDrawerGLTraits::NeededForDrawingType neededForDrawing,
            std::shared_ptr<LevelDrawerGLTraits::RenderLoaderType> inRenderLoader,
            std::shared_ptr<GameRequester> inGameRequester)
            : m_modelTable{},
            m_textureTable{},
            m_drawObjectTableList{
                std::make_shared<LevelDrawerGLTraits::DrawObjectTableType>(),
                std::make_shared<LevelDrawerGLTraits::DrawObjectTableType>(),
                std::make_shared<LevelDrawerGLTraits::DrawObjectTableType>() },
            m_renderLoader{std::move(inRenderLoader)},
            m_gameRequester{std::move(inGameRequester)},
            m_neededForDrawing{neededForDrawing},
            m_bgColor{0.0f, 0.0f, 0.0f, 1.0f}
    {}
}