/**
 * Copyright 2024 Cerulean Quasar. All Rights Reserved.
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
#include <memory>
#include <vector>

#include "renderDetails/renderDetailsGL.hpp"
#include "renderDetailsGL.hpp"
#include "renderDetails/shadows/renderDetailsGL.hpp"
#include "graphicsGL.hpp"

namespace darkObject {

    renderDetails::ReferenceGL RenderDetailsGL::loadNew(
            renderDetails::Description const &description,
            std::vector<char const *> const &shaders,
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &,
            std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &parametersBase)
    {
        auto parameters =
                dynamic_cast<renderDetails::ParametersDarkObjectGL*>(parametersBase.get());
        if (parameters == nullptr) {
            throw std::runtime_error("Invalid render details parameter type.");
        }

        if (shaders.size() != 3) {
            throw std::runtime_error("Invalid number of shaders received in load attempt of Dark Object Render details.");
        }

        auto rd =
                std::make_shared<RenderDetailsGL>(
                    description, shaders[0],
                    shaders[1], shaders[2],
                    gameRequester, surfaceDetails->surfaceWidth,
                    surfaceDetails->surfaceHeight,
                    surfaceDetails->useIntTexture);

        auto cod = std::make_shared<CommonObjectDataGL>(*parameters,
                surfaceDetails->surfaceWidth / static_cast<float>(surfaceDetails->surfaceHeight));

        return createReference(std::move(rd), std::move(cod));
    }

    renderDetails::ReferenceGL RenderDetailsGL::loadExisting(
            std::shared_ptr<GameRequester> const &,
            std::shared_ptr<RenderLoaderGL> const &,
            std::shared_ptr<renderDetails::RenderDetailsGL> rdBase,
            std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &parametersBase)
    {
        auto rd = dynamic_cast<RenderDetailsGL*>(rdBase.get());
        if (rd == nullptr) {
            throw std::runtime_error("Invalid render details type.");
        }

        if (rd->m_surfaceWidth != surfaceDetails->surfaceWidth ||
            rd->m_surfaceHeight != surfaceDetails->surfaceHeight)
        {
            rd->m_surfaceWidth = surfaceDetails->surfaceWidth;
            rd->m_surfaceHeight = surfaceDetails->surfaceHeight;
        }

        auto parameters =
                dynamic_cast<renderDetails::ParametersDarkObjectGL*>(parametersBase.get());
        if (parameters == nullptr) {
            throw std::runtime_error("Invalid render details parameter type.");
        }

        auto cod = std::make_shared<CommonObjectDataGL>(*parameters,
                surfaceDetails->surfaceWidth / static_cast<float>(surfaceDetails->surfaceHeight));

        return createReference(std::move(rdBase), std::move(cod));
    }

    renderDetails::ReferenceGL RenderDetailsGL::createReference(
            std::shared_ptr<renderDetails::RenderDetailsGL> rd,
            std::shared_ptr<CommonObjectDataGL> cod)
    {
        renderDetails::ReferenceGL ref;
        ref.createDrawObjectData = renderDetails::ReferenceGL::CreateDrawObjectData(
            [] (
                    std::shared_ptr<renderDetails::DrawObjectDataGL> const &,
                    std::shared_ptr<levelDrawer::TextureData> const &,
                    glm::mat4 const &modelMatrix) -> std::shared_ptr<renderDetails::DrawObjectDataGL>
            {
                return std::make_shared<DrawObjectDataGL>(modelMatrix);
            });

        ref.getProjViewForLevel = renderDetails::ReferenceGL::GetProjViewForLevel(
                [cod] () -> std::pair<glm::mat4, glm::mat4> {
                    return cod->getProjViewForLevel();
                });

        ref.renderDetails = std::move(rd);
        ref.commonObjectData = std::move(cod);
        return std::move(ref);
    }

    void loadTexture(CommonObjectDataGL const *cod, GLuint programID, int activeTexture, char const *textureVarName, int imageID) {
        auto &fb = cod->darkFramebuffer(imageID);
        auto textureID = glGetUniformLocation(programID, textureVarName);
        checkGraphicsError();
        glActiveTexture(activeTexture);
        checkGraphicsError();
        glBindTexture(GL_TEXTURE_2D, fb->depthImage());
        checkGraphicsError();
        glUniform1i(textureID, imageID);
        checkGraphicsError();
    }

    void RenderDetailsGL::draw(
            uint32_t modelMatrixID,
            std::shared_ptr<renderDetails::CommonObjectDataBase> const &commonObjectData,
            std::shared_ptr<levelDrawer::DrawObjectTableGL> const &drawObjTable,
            std::set<levelDrawer::ZValueReference>::iterator beginZValRefs,
            std::set<levelDrawer::ZValueReference>::iterator endZValRefs)
    {
        glCullFace(GL_BACK);
        checkGraphicsError();

        GLuint textureProgramID = *m_textureProgram.get();
        GLuint colorProgramID = *m_colorProgram.get();
        GLuint programID = textureProgramID;
        GLint MatrixID = -1;
        GLint normalMatrixID = -1;
        GLint textureID = -1;

        auto cod = dynamic_cast<CommonObjectDataGL *>(commonObjectData.get());
        if (!cod) {
            throw std::runtime_error("Invalid common object data type");
        }

        for (auto it = beginZValRefs; it != endZValRefs; it++) {
            auto &drawObj = drawObjTable->drawObject(it->drawObjectReference);
            auto &modelData = drawObj->modelData();
            auto &textureData = drawObj->textureData();
            if (it == beginZValRefs ||
                (programID == textureProgramID && !textureData) ||
                    (programID == colorProgramID && textureData)) {
                programID = textureData ? textureProgramID : colorProgramID;
                glUseProgram(programID);
                checkGraphicsError();

                auto projView = cod->getProjViewForLevel();

                // the projection matrix * the view matrix
                MatrixID = glGetUniformLocation(programID, "projView");
                checkGraphicsError();
                glm::mat4 projTimesView = projView.first * projView.second;
                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &projTimesView[0][0]);
                checkGraphicsError();

                // the projection matrix * the view matrix from the light source point of view (Ball)
                auto projViewLightBall = projView.first * cod->getViewLightSource(0, CommonObjectDataGL::up);
                MatrixID = glGetUniformLocation(programID, "projViewLightBallUp");
                checkGraphicsError();
                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &projViewLightBall[0][0]);
                checkGraphicsError();

                projViewLightBall = projView.first * cod->getViewLightSource(0, CommonObjectDataGL::right);
                MatrixID = glGetUniformLocation(programID, "projViewLightBallRight");
                checkGraphicsError();
                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &projViewLightBall[0][0]);
                checkGraphicsError();

                projViewLightBall = projView.first * cod->getViewLightSource(0, CommonObjectDataGL::down);
                MatrixID = glGetUniformLocation(programID, "projViewLightBallDown");
                checkGraphicsError();
                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &projViewLightBall[0][0]);
                checkGraphicsError();

                projViewLightBall = projView.first * cod->getViewLightSource(0, CommonObjectDataGL::left);
                MatrixID = glGetUniformLocation(programID, "projViewLightBallLeft");
                checkGraphicsError();
                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &projViewLightBall[0][0]);
                checkGraphicsError();

                GLint lightBallPosID = glGetUniformLocation(programID, "lightPosBall");
                checkGraphicsError();
                glm::vec3 lightBallPos = cod->getLightSource(0);
                glUniform3fv(lightBallPosID, 1, &lightBallPos[0]);
                checkGraphicsError();

                // the projection matrix * the view matrix from the light source point of view (Hole)
                auto projViewLight = projView.first * cod->getViewLightSource(1, CommonObjectDataGL::up);
                MatrixID = glGetUniformLocation(programID, "projViewLightHoleUp");
                checkGraphicsError();
                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &projViewLight[0][0]);
                checkGraphicsError();

                projViewLight = projView.first * cod->getViewLightSource(1, CommonObjectDataGL::right);
                MatrixID = glGetUniformLocation(programID, "projViewLightHoleRight");
                checkGraphicsError();
                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &projViewLight[0][0]);
                checkGraphicsError();

                projViewLight = projView.first * cod->getViewLightSource(1, CommonObjectDataGL::down);
                MatrixID = glGetUniformLocation(programID, "projViewLightHoleDown");
                checkGraphicsError();
                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &projViewLight[0][0]);
                checkGraphicsError();

                projViewLight = projView.first * cod->getViewLightSource(1, CommonObjectDataGL::left);
                MatrixID = glGetUniformLocation(programID, "projViewLightHoleLeft");
                checkGraphicsError();
                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &projViewLight[0][0]);
                checkGraphicsError();

                GLint lightPosID = glGetUniformLocation(programID, "lightPosHole");
                checkGraphicsError();
                glm::vec3 lightPos = cod->getLightSource(1);
                glUniform3fv(lightPosID, 1, &lightPos[0]);
                checkGraphicsError();

                // Dark maps
                loadTexture(cod, programID, GL_TEXTURE0, "texDarkMap0", 0);
                loadTexture(cod, programID, GL_TEXTURE1, "texDarkMap1", 1);
                loadTexture(cod, programID, GL_TEXTURE2, "texDarkMap2", 2);
                loadTexture(cod, programID, GL_TEXTURE3, "texDarkMap3", 3);
                loadTexture(cod, programID, GL_TEXTURE4, "texDarkMap4", 4);
                loadTexture(cod, programID, GL_TEXTURE5, "texDarkMap5", 5);
                loadTexture(cod, programID, GL_TEXTURE6, "texDarkMap6", 6);
                loadTexture(cod, programID, GL_TEXTURE7, "texDarkMap7", 7);

                MatrixID = glGetUniformLocation(programID, "model");
                checkGraphicsError();

                normalMatrixID = glGetUniformLocation(programID, "normalMatrix");
                checkGraphicsError();

                if (textureData) {
                    textureID = glGetUniformLocation(programID, "texSampler");
                    checkGraphicsError();
                }
            }

            if (textureData) {
                glActiveTexture(GL_TEXTURE4);
                checkGraphicsError();
                glBindTexture(GL_TEXTURE_2D, textureData->handle());
                checkGraphicsError();
                glUniform1i(textureID, 4);
                checkGraphicsError();
            }

            auto objData = drawObj->objData(it->drawObjectDataReference.get());
            auto modelMatrix = objData->modelMatrix(modelMatrixID);

            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &modelMatrix[0][0]);
            checkGraphicsError();

            glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));
            glUniformMatrix4fv(normalMatrixID, 1, GL_FALSE, &normalMatrix[0][0]);
            checkGraphicsError();

            drawVertices(programID, modelData);
        }
    }

    RenderDetailsGL::RenderDetailsGL(
            renderDetails::Description inDescription,
            char const *vertexShaderFile,
            char const *textureFragShaderFile,
            char const *colorFragShaderFile,
            std::shared_ptr<GameRequester> const &inGameRequester,
            uint32_t inWidth, uint32_t inHeight, bool usesIntSurface)
        : renderDetails::RenderDetailsGL(std::move(inDescription), inWidth, inHeight, usesIntSurface),
        m_textureProgram{},
        m_colorProgram{}
    {
        auto vertexShader = renderDetails::getShader(
                inGameRequester, vertexShaderFile, GL_VERTEX_SHADER);
        auto textureShader = renderDetails::getShader(
                inGameRequester, textureFragShaderFile, GL_FRAGMENT_SHADER);
        auto colorShader = renderDetails::getShader(
                inGameRequester, colorFragShaderFile, GL_FRAGMENT_SHADER);

        m_textureProgram = renderDetails::getProgram(
                std::vector{vertexShader, std::move(textureShader)});
        m_colorProgram = renderDetails::getProgram(
                std::vector{vertexShader, std::move(colorShader)});
    }

    char constexpr const *SHADER_VERT_GL_FILE = "shaders/darkShaderGL.vert";
    char constexpr const *TEXTURE_SHADER_FRAG_GL_FILE = "shaders/darkTextureGL.frag";
    char constexpr const *COLOR_SHADER_FRAG_GL_FILE = "shaders/darkColorGL.frag";
    RegisterGL<renderDetails::RenderDetailsGL, RenderDetailsGL> registerGL(
            {renderDetails::DrawingStyle::dark, {renderDetails::Features::color, renderDetails::Features::texture}},
            std::vector<char const *>{SHADER_VERT_GL_FILE, TEXTURE_SHADER_FRAG_GL_FILE, COLOR_SHADER_FRAG_GL_FILE});
} // darkObject
