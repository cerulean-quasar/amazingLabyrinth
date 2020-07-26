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
#include "../graphicsGL.hpp"
#include "levelDrawerGL.hpp"
#include "levelDrawerGraphics.hpp"
#include "../renderDetails/renderDetailsGL.hpp"

namespace levelDrawer {
    template <>
    void LevelDrawerGraphics<LevelDrawerGLTraits>::draw(
            LevelDrawerGLTraits::DrawArgumentType const &info)
    {
        auto rulesList = getDrawRules();

        // add the pre main draw commands to the command buffer.
        for (auto const &rule : rulesList) {
            rule.renderDetails->preMainDraw(0, rule.commonObjectData,
                    m_drawObjectTableList, rule.indicesPerLevelType);
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

        // add the commands to the command buffer for the main draw.
        for (auto index : std::vector<ObjectType>{LEVEL, STARTER, FINISHER}) {
            for (auto const &rule : rulesList) {
                if (rule.commonObjectData[index] == nullptr || rule.indicesPerLevelType[index].empty()) {
                    continue;
                }
                rule.renderDetails->draw(
                        0, rule.commonObjectData[index], m_drawObjectTableList[index],
                        rule.indicesPerLevelType[index]);
            }
        }
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
        auto rules = drawObjTable->getDrawRules();
        DrawObjectTableList drawObjTableList = {nullptr, drawObjTable, nullptr};
        CommonObjectDataList commonObjectDataList = {nullptr, rules[0].commonObjectData, nullptr};
        IndicesForDrawList indicesForDrawList = {std::vector<size_t>{}, rules[0].drawObjectIndices, std::vector<size_t>{}};
        rules[0].renderDetails->preMainDraw(
                0, commonObjectDataList, drawObjTableList, indicesForDrawList);

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
        rules[0].renderDetails->overrideClearColor(clearColor);
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        checkGraphicsError();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        checkGraphicsError();

        rules[0].renderDetails->draw(0, rules[0].commonObjectData, drawObjTable, rules[0].drawObjectIndices);

        glFinish();
        checkGraphicsError();

        if (m_neededForDrawing.useIntegerSurface) {
            /* width * height * 4 color values each a char in size. */
            std::vector<float> data(static_cast<size_t>(imageWidth * imageHeight * 4), 0.0f);
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            checkGraphicsError();
            glReadPixels(0, 0, imageWidth, imageHeight, colorImageFormat.format, colorImageFormat.type, data.data());
            checkGraphicsError();
            rules[0].renderDetails->postProcessImageBuffer(rules[0].commonObjectData, data, results);
        } else {
            /* width * height * 4 color values each a char in size. */
            std::vector<uint8_t> data(static_cast<size_t>(imageWidth * imageHeight * 4), 0.0f);
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            checkGraphicsError();
            glReadPixels(0, 0, imageWidth, imageHeight, colorImageFormat.format, colorImageFormat.type, data.data());
            checkGraphicsError();
            std::vector<float> input{};
            for (auto datum : data) {
                uint32_t datum32 = datum;
                float *pdatum = reinterpret_cast<float*>(&datum32);
                input.push_back(*pdatum);
            }
            rules[0].renderDetails->postProcessImageBuffer(rules[0].commonObjectData, input, results);
        }

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