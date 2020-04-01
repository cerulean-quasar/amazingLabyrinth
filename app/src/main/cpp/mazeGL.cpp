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

#include <cstring>
#include <istream>

#include <glm/glm.hpp>

#include "graphicsGL.hpp"
#include "mazeGL.hpp"
#include "mathGraphics.hpp"

void checkGraphicsError() {
    GLenum rc = glGetError();
    if (rc != GL_NO_ERROR) {
        std::string c;
        switch (rc) {
            case GL_INVALID_ENUM:
                c = "GL_INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                c = "GL_INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                c = "GL_INVALID_OPERATION";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                c = "GL_INVALID_FRAMEBUFFER_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                c = "GL_OUT_OF_MEMORY";
                break;
            default:
                c = "Unknown return code.";
        }
        throw std::runtime_error(std::string("A graphics error occurred: ") + c);
    }
}
std::string const SHADER_VERT_FILE("shaders/shaderGL.vert");
std::string const SHADER_FRAG_FILE("shaders/shaderGL.frag");
std::string const DEPTH_VERT_FILE("shaders/depthShaderGL.vert");
std::string const DEPTH_FRAG_FILE("shaders/depthShaderGL.frag");
std::string const DEPTH_AND_NORMAL_VERT_FILE("shaders/depthAndNormalGL.vert");
std::string const DEPTH_AND_NORMAL_FRAG_FILE("shaders/depthAndNormalGL.frag");

void DrawObjectDataGL::createDrawObjectData(std::shared_ptr<DrawObject> const &drawObj) {
    // the index buffer
    glGenBuffers(1, &(m_indexBuffer));
    checkGraphicsError();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    checkGraphicsError();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (uint32_t) * drawObj->indices.size(),
                 drawObj->indices.data(), GL_STATIC_DRAW);
    checkGraphicsError();

    // the vertex buffer
    glGenBuffers(1, &(m_vertexBuffer));
    checkGraphicsError();
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    checkGraphicsError();
    glBufferData(GL_ARRAY_BUFFER, sizeof (Vertex) * drawObj->vertices.size(),
                 drawObj->vertices.data(), GL_STATIC_DRAW);
    checkGraphicsError();
}

void TextureDataGL::createTexture(std::shared_ptr<TextureDescription> const &textureDescription) {

    glGenTextures(1, &m_handle);
    checkGraphicsError();
    glBindTexture(GL_TEXTURE_2D, m_handle);
    checkGraphicsError();

    // when sampling outside of the texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    checkGraphicsError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    checkGraphicsError();

    // when the texture is scaled up or down
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, /*GL_LINEAR_MIPMAP_LINEAR*/
                    GL_NEAREST);
    checkGraphicsError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, /*GL_LINEAR*/ GL_NEAREST);
    checkGraphicsError();

    uint32_t texHeight;
    uint32_t texWidth;
    uint32_t texChannels;
    std::vector<char> pixels = textureDescription->getData(texWidth, texHeight, texChannels);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixels.data());
    checkGraphicsError();

    glGenerateMipmap(GL_TEXTURE_2D);
    checkGraphicsError();
    glBindTexture(GL_TEXTURE_2D, 0);
    checkGraphicsError();
}

glm::mat4 LevelSequenceGL::getPerspectiveMatrixForLevel(uint32_t surfaceWidth, uint32_t surfaceHeight) {
    /* perspective matrix: takes the perspective projection, the aspect ratio, near and far
     * view planes.
     */
    return getPerspectiveMatrix(m_perspectiveViewAngle,
                                      surfaceWidth / static_cast<float>(surfaceHeight),
                                      m_perspectiveNearPlane, m_perspectiveFarPlane,
                                      false, false);
}

void LevelSequenceGL::updatePerspectiveMatrix(uint32_t surfaceWidth, uint32_t surfaceHeight) {
    m_proj = getPerspectiveMatrix(m_perspectiveViewAngle,
                                  surfaceWidth / static_cast<float>(surfaceHeight),
                                  m_perspectiveNearPlane, m_perspectiveFarPlane,
                                  false, false);

}

std::shared_ptr<TextureData> LevelSequenceGL::createTexture(std::shared_ptr<TextureDescription> const &textureDescription) {
    return std::make_shared<TextureDataGL>(textureDescription);
}

std::shared_ptr<DrawObjectData> LevelSequenceGL::createObject(std::shared_ptr<DrawObject> const &obj, TextureMap &textures) {
    return std::make_shared<DrawObjectDataGL>(obj);
}

void LevelSequenceGL::updateLevelData(DrawObjectTable &objsData, TextureMap &textures) {
    if (m_texturesChanged) {
        addTextures(textures);
    }

    for (auto &&objData : objsData) {
        DrawObjectDataGL *data = dynamic_cast<DrawObjectDataGL *> (objData.second.get());
        if (data == nullptr) {
            // a completely new entry
            objData.second = createObject(objData.first, textures);
        }
    }
}

void GraphicsGL::initPipeline() {
    glViewport(0, 0, m_surface.width(), m_surface.height());

    programID = loadShaders(SHADER_VERT_FILE, SHADER_FRAG_FILE);
    depthProgramID = loadShaders(DEPTH_VERT_FILE, DEPTH_FRAG_FILE);
    m_depthAndNormalProgramID = loadShaders(DEPTH_AND_NORMAL_VERT_FILE, DEPTH_AND_NORMAL_FRAG_FILE);

    // for shadow mapping.
    m_framebufferShadowMap = std::make_shared<Framebuffer>(m_surface.width(), m_surface.height());
}

Framebuffer::Framebuffer(uint32_t width, uint32_t height, uint32_t nbrColorAttachments)
    : m_depthMapFBO(GL_INVALID_VALUE),
      m_depthMap(GL_INVALID_VALUE),
      m_colorImage{}
{
    m_colorImage.resize(nbrColorAttachments, GL_INVALID_VALUE);
    for (uint32_t i = 0; i < nbrColorAttachments; i++) {
    }

    glGenTextures(1, &m_depthMap);
    checkGraphicsError();
    glBindTexture(GL_TEXTURE_2D, m_depthMap);
    checkGraphicsError();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    checkGraphicsError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    checkGraphicsError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    checkGraphicsError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    checkGraphicsError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    checkGraphicsError();
    glBindTexture(GL_TEXTURE_2D, 0);
    checkGraphicsError();

    glGenFramebuffers(1, &m_depthMapFBO);
    checkGraphicsError();

    glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
    checkGraphicsError();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthMap, 0);
    checkGraphicsError();
    for (uint32_t i = 0; i < nbrColorAttachments; i++) {
        auto activeTextureIndicator = GL_TEXTURE0;
        auto attachmentIndicator = GL_COLOR_ATTACHMENT0;
        switch (i) {
            case 0:
                activeTextureIndicator = GL_TEXTURE0;
                attachmentIndicator = GL_COLOR_ATTACHMENT0;
                break;
            case 1:
                activeTextureIndicator = GL_TEXTURE1;
                attachmentIndicator = GL_COLOR_ATTACHMENT1;
                break;
            case 2:
                activeTextureIndicator = GL_TEXTURE2;
                attachmentIndicator = GL_COLOR_ATTACHMENT2;
                break;
            case 3:
                activeTextureIndicator = GL_TEXTURE3;
                attachmentIndicator = GL_COLOR_ATTACHMENT3;
                break;
            case 4:
                activeTextureIndicator = GL_TEXTURE4;
                attachmentIndicator = GL_COLOR_ATTACHMENT4;
                break;
            case 5:
                activeTextureIndicator = GL_TEXTURE5;
                attachmentIndicator = GL_COLOR_ATTACHMENT5;
                break;
            case 6:
                activeTextureIndicator = GL_TEXTURE6;
                attachmentIndicator = GL_COLOR_ATTACHMENT6;
                break;
            case 7:
                activeTextureIndicator = GL_TEXTURE7;
                attachmentIndicator = GL_COLOR_ATTACHMENT7;
                break;
            case 8:
                activeTextureIndicator = GL_TEXTURE8;
                attachmentIndicator = GL_COLOR_ATTACHMENT8;
                break;
            case 9:
                activeTextureIndicator = GL_TEXTURE9;
                attachmentIndicator = GL_COLOR_ATTACHMENT9;
                break;
            case 10:
                activeTextureIndicator = GL_TEXTURE10;
                attachmentIndicator = GL_COLOR_ATTACHMENT10;
                break;
            case 11:
                activeTextureIndicator = GL_TEXTURE11;
                attachmentIndicator = GL_COLOR_ATTACHMENT11;
                break;
            case 12:
                activeTextureIndicator = GL_TEXTURE12;
                attachmentIndicator = GL_COLOR_ATTACHMENT12;
                break;
            case 13:
                activeTextureIndicator = GL_TEXTURE3;
                attachmentIndicator = GL_COLOR_ATTACHMENT13;
                break;
            case 14:
                activeTextureIndicator = GL_TEXTURE14;
                attachmentIndicator = GL_COLOR_ATTACHMENT14;
                break;
            case 15:
                activeTextureIndicator = GL_TEXTURE15;
                attachmentIndicator = GL_COLOR_ATTACHMENT15;
                break;
            default:
                throw std::runtime_error("Too many color attachments requested.  Not supported.");
        }
        glGenTextures(1, &m_colorImage[i]);
        checkGraphicsError();
        glActiveTexture(activeTextureIndicator);
        checkGraphicsError();
        glBindTexture(GL_TEXTURE_2D, m_colorImage[i]);
        checkGraphicsError();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        checkGraphicsError();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        checkGraphicsError();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        checkGraphicsError();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        checkGraphicsError();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        checkGraphicsError();
        glBindTexture(GL_TEXTURE_2D, 0);
        checkGraphicsError();

        glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentIndicator, GL_TEXTURE_2D, m_colorImage[i], 0);
        checkGraphicsError();
    }

    GLenum rc = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (rc != GL_FRAMEBUFFER_COMPLETE) {
        std::string c;
        switch (rc) {
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                c = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                c = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
                c = "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS";
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                c = "GL_FRAMEBUFFER_UNSUPPORTED";
                break;
            default:
                c = "Unknown return code.";
        }
        throw std::runtime_error(std::string("Framebuffer is not complete, returned: ") + c);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkGraphicsError();
}

void GraphicsGL::createDepthTexture() {
    if (m_levelSequence->levelDynObjsData().empty() && m_levelSequence->levelStaticObjsData().empty()) {
        // nothing to draw
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferShadowMap->fbo());
    checkGraphicsError();

    // set the shader to use
    glUseProgram(depthProgramID);
    checkGraphicsError();
    glCullFace(GL_FRONT);
    checkGraphicsError();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    checkGraphicsError();

    glm::mat4 proj = m_levelSequence->projectionMatrix();
    glm::mat4 view = m_levelSequence->viewLightSource();

    GLint MatrixID;
    MatrixID = glGetUniformLocation(depthProgramID, "view");
    checkGraphicsError();
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &view[0][0]);
    checkGraphicsError();
    MatrixID = glGetUniformLocation(depthProgramID, "proj");
    checkGraphicsError();
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &proj[0][0]);
    checkGraphicsError();

    for (auto &&obj : m_levelSequence->levelStaticObjsData()) {
        for (auto &&model : obj.first->modelMatrices) {
            DrawObjectDataGL *data = dynamic_cast<DrawObjectDataGL*> (obj.second.get());
            drawObject(depthProgramID, false, data->vertexBuffer(), data->indexBuffer(),
                       obj.first->indices.size(), model);
        }
    }

    for (auto &&obj : m_levelSequence->levelDynObjsData()) {
        for (auto &&model : obj.first->modelMatrices) {
            DrawObjectDataGL *data = dynamic_cast<DrawObjectDataGL*> (obj.second.get());
            drawObject(depthProgramID, false, data->vertexBuffer(), data->indexBuffer(),
                       obj.first->indices.size(), model);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkGraphicsError();
}

// some levels use this function to get a depth texture and surface normals
std::shared_ptr<TextureData> GraphicsGL::getDepthTexture(
        DrawObjectTable const &objsData,
        float width,
        float height,
        uint32_t rowSize,
        std::vector<float> &depthMap, /* output */
        std::vector<glm::vec3> &normalMap) /* output */
{
    uint32_t surfaceWidth = rowSize;
    uint32_t surfaceHeight = (m_surface.height()*surfaceWidth)/m_surface.width();
    Framebuffer fb(surfaceWidth, surfaceHeight, 2);
    glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo());
    checkGraphicsError();

    // set the viewport
    glViewport(0, 0, surfaceWidth, surfaceHeight);
    checkGraphicsError();

    // set the shader to use
    glUseProgram(m_depthAndNormalProgramID);
    checkGraphicsError();

    glCullFace(GL_BACK);
    checkGraphicsError();
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    checkGraphicsError();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    checkGraphicsError();

    glm::mat4 proj = getOrthoMatrix(-width/2.0f, width/2.0f, -height/2.0f, height/2.0f,
            m_depthTextureNearPlane, m_depthTextureFarPlane, false, false);
    glm::mat4 view = m_levelSequence->viewMatrix();

    GLint MatrixID;
    MatrixID = glGetUniformLocation(depthProgramID, "view");
    checkGraphicsError();
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &view[0][0]);
    checkGraphicsError();
    MatrixID = glGetUniformLocation(depthProgramID, "proj");
    checkGraphicsError();
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &proj[0][0]);
    checkGraphicsError();

    for (auto const &obj : objsData) {
        auto data = std::make_shared<DrawObjectDataGL>(obj.first);
        for (auto const &model : obj.first->modelMatrices) {
            drawObject(depthProgramID, true, data->vertexBuffer(), data->indexBuffer(),
                       obj.first->indices.size(), model);
        }
    }

    // set the viewport back for the rendering to the screen
    glViewport(0, 0, m_surface.width(), m_surface.height());
    checkGraphicsError();

    /* depth texture */
    /* width * height * 4 color values each a char in size. */
    std::vector<unsigned char> data(static_cast<size_t>(surfaceWidth * surfaceHeight * 4));
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    checkGraphicsError();
    glReadPixels(0, 0, surfaceWidth, surfaceHeight, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    checkGraphicsError();
    bitmapToDepthMap<unsigned char>(data, proj, view, surfaceWidth, surfaceHeight, 4, false, false, depthMap);

    /* normals texture */
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    checkGraphicsError();
    glReadPixels(0, 0, surfaceWidth, surfaceHeight, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    checkGraphicsError();
    bitmapToNormals<unsigned char>(data, surfaceWidth, surfaceHeight, 4, false, normalMap);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkGraphicsError();

    return std::make_shared<TextureDataGL>(fb.acquireColorImage(1));
}

void GraphicsGL::drawFrame() {
    createDepthTexture();

    // set the shader to use
    glUseProgram(programID);
    checkGraphicsError();
    glCullFace(GL_BACK);
    checkGraphicsError();

    // The clear background color
    glm::vec4 bgColor = m_levelSequence->backgroundColor();
    glClearColor(bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    checkGraphicsError();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    checkGraphicsError();

    glm::mat4 proj = m_levelSequence->projectionMatrix();
    glm::mat4 view = m_levelSequence->viewMatrix();

    glm::mat4 lightSpaceMatrix = m_levelSequence->viewLightSource();
    GLint MatrixID;
    MatrixID = glGetUniformLocation(programID, "view");
    checkGraphicsError();
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &view[0][0]);
    checkGraphicsError();
    MatrixID = glGetUniformLocation(programID, "proj");
    checkGraphicsError();
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &proj[0][0]);
    checkGraphicsError();
    MatrixID = glGetUniformLocation(programID, "lightSpaceMatrix");
    checkGraphicsError();
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
    checkGraphicsError();

    GLint lightPosID = glGetUniformLocation(programID, "lightPos");
    checkGraphicsError();
    glm::vec3 lightPos = m_levelSequence->lightingSource();
    glUniform3fv(lightPosID, 1, &lightPos[0]);
    checkGraphicsError();

    GLint textureID = glGetUniformLocation(programID, "texShadowMap");
    checkGraphicsError();
    glActiveTexture(GL_TEXTURE0);
    checkGraphicsError();
    glBindTexture(GL_TEXTURE_2D, m_framebufferShadowMap->depthImage());
    checkGraphicsError();
    glUniform1i(textureID, 0);
    checkGraphicsError();
    drawObjects(m_levelSequence->levelStaticObjsData(), m_levelSequence->levelTextures());
    drawObjects(m_levelSequence->levelDynObjsData(), m_levelSequence->levelTextures());
    drawObjects(m_levelSequence->starterStaticObjsData(), m_levelSequence->starterTextures());
    drawObjects(m_levelSequence->starterDynObjsData(), m_levelSequence->starterTextures());
    if (m_levelSequence->needFinisherObjs()) {
        drawObjects(m_levelSequence->finisherObjsData(), m_levelSequence->finisherTextures());
    }
    eglSwapBuffers(m_surface.display(), m_surface.surface());
}

void GraphicsGL::drawObjects(DrawObjectTable const &objsData, TextureMap const &textures) {
    for (auto const& obj : objsData) {
        DrawObjectDataGL *data = static_cast<DrawObjectDataGL*> (obj.second.get());
        auto it = textures.find(obj.first->texture);
        if (it == textures.end()) {
            throw (std::runtime_error("Could not find texture!"));
        }
        TextureDataGL const *texture = static_cast<TextureDataGL const *> (it->second.get());
        for (auto const &model : obj.first->modelMatrices) {
            drawObject(programID, true, data->vertexBuffer(), data->indexBuffer(),
                       obj.first->indices.size(), texture->handle(), model);
        }
    }
}

void GraphicsGL::drawObject(GLuint programID, bool needsNormal, GLuint vertex, GLuint index,
                            unsigned long nbrIndices, GLuint texture, glm::mat4 const &modelMatrix) {
    GLint textureID = glGetUniformLocation(programID, "texSampler");
    checkGraphicsError();
    glActiveTexture(GL_TEXTURE1);
    checkGraphicsError();
    glBindTexture(GL_TEXTURE_2D, texture);
    checkGraphicsError();
    glUniform1i(textureID, 1);
    checkGraphicsError();

    drawObject(programID, needsNormal, vertex, index, nbrIndices, modelMatrix);
}

void GraphicsGL::drawObject(GLuint programID, bool needsNormal, GLuint vertex, GLuint index,
                            unsigned long nbrIndices, glm::mat4 const &modelMatrix) {
    // Send our transformation to the currently bound shader, in the "MVP"
    // uniform. This is done in the main loop since each model will have a
    // different MVP matrix (At least for the M part)
    GLint MatrixID = glGetUniformLocation(programID, "model");
    checkGraphicsError();
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &modelMatrix[0][0]);
    checkGraphicsError();

    if (needsNormal) {
        GLint normalMatrixID = glGetUniformLocation(programID, "normalMatrix");
        checkGraphicsError();
        glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));
        glUniformMatrix4fv(normalMatrixID, 1, GL_FALSE, &normalMatrix[0][0]);
        checkGraphicsError();
    }

    glBindBuffer(GL_ARRAY_BUFFER, vertex);
    checkGraphicsError();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index);
    checkGraphicsError();

    // 1st attribute buffer : colors
    // color is only needed when doing the final render, not the depth texture render
    GLint colorID = glGetAttribLocation(programID, "inColor");
    if (colorID != -1) {
        glVertexAttribPointer(
                colorID,                          // The position of the attribute in the shader.
                3,                                // size
                GL_FLOAT,                         // type
                GL_FALSE,                         // normalized?
                sizeof(Vertex),                   // stride
                (void *) (offsetof(Vertex, color))// array buffer offset
        );
        checkGraphicsError();
        glEnableVertexAttribArray(colorID);
        checkGraphicsError();
    }

    // attribute: position
    GLint position = glGetAttribLocation(programID, "inPosition");
    glVertexAttribPointer(
            position,                        // The position of the attribute in the shader.
            3,                               // size
            GL_FLOAT,                        // type
            GL_FALSE,                        // normalized?
            sizeof(Vertex),                  // stride
            (void *) (offsetof(Vertex, pos)) // array buffer offset
    );
    checkGraphicsError();
    glEnableVertexAttribArray(position);
    checkGraphicsError();

    // Send in the texture coordinates
    // only required for the final rendering.
    GLint texCoordID = glGetAttribLocation(programID, "inTexCoord");
    if (texCoordID != -1) {
        glVertexAttribPointer(
                texCoordID,                       // The position of the attribute in the shader
                2,                                // size
                GL_FLOAT,                         // type
                GL_FALSE,                         // normalized?
                sizeof(Vertex),                  // stride
                (void *) offsetof (Vertex, texCoord)  // array buffer offset
        );
        checkGraphicsError();
        glEnableVertexAttribArray(texCoordID);
        checkGraphicsError();
    }

    GLint normCoordID = glGetAttribLocation(programID, "inNormal");
    if (normCoordID != -1) {
        checkGraphicsError();
        glVertexAttribPointer(
                normCoordID,                      // The position of the attribute in the shader
                3,                                // size
                GL_FLOAT,                         // type
                GL_FALSE,                         // normalized?
                sizeof(Vertex),                  // stride
                (void *) offsetof (Vertex, normal)  // array buffer offset
        );
        checkGraphicsError();
        glEnableVertexAttribArray(normCoordID);
        checkGraphicsError();
    }

    // Draw the triangles !
    //glDrawArrays(GL_TRIANGLES, 0, dice[0].die->vertices.size() /* total number of vertices*/);
    glDrawElements(GL_TRIANGLES, nbrIndices, GL_UNSIGNED_INT, 0);
    checkGraphicsError();

    glDisableVertexAttribArray(position);
    checkGraphicsError();

    if (colorID != -1) {
        glDisableVertexAttribArray(colorID);
        checkGraphicsError();
    }

    if (texCoordID != -1) {
        glDisableVertexAttribArray(texCoordID);
        checkGraphicsError();
    }

    if (normCoordID != -1) {
        glDisableVertexAttribArray(normCoordID);
        checkGraphicsError();
    }
}

GLuint GraphicsGL::loadShaders(std::string const &vertexShaderFile, std::string const &fragmentShaderFile) {
    GLint Result = GL_TRUE;
    GLint InfoLogLength = 0;

    std::vector<char> vertexShader = readFile(m_gameRequester, vertexShaderFile);
    std::vector<char> fragmentShader = readFile(m_gameRequester, fragmentShaderFile);

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Compile Vertex Shader
    char const * VertexSourcePointer = vertexShader.data();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (Result == GL_FALSE) {
        if (InfoLogLength > 0) {
            std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
            glGetShaderInfoLog(VertexShaderID, InfoLogLength, &InfoLogLength,
                               VertexShaderErrorMessage.data());
            if (VertexShaderErrorMessage[0] == '\0') {
                throw std::runtime_error("vertex shader compile error");
            } else {
                throw std::runtime_error(std::string("vertex shader compile error: ") + VertexShaderErrorMessage.data());
            }
        } else {
            throw std::runtime_error("vertex shader compile error");
        }
    }

    // Compile Fragment Shader
    char const * FragmentSourcePointer = fragmentShader.data();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (Result == GL_FALSE) {
        if (InfoLogLength > 0) {
            std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
            glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL,
                               FragmentShaderErrorMessage.data());
            if (FragmentShaderErrorMessage[0] == '\0') {
                throw std::runtime_error("Fragment shader compile error.");
            } else {
                throw std::runtime_error(std::string("Fragment shader compile error: ") + FragmentShaderErrorMessage.data());
            }
        } else {
            throw std::runtime_error("Fragment shader compile error.");
        }
    }

    // Link the program
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (Result == GL_FALSE) {
        if (InfoLogLength > 0) {
            std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
            glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, ProgramErrorMessage.data());
            throw std::runtime_error(ProgramErrorMessage.data());
        } else {
            throw std::runtime_error("Linker error.");
        }
    }

    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

void GraphicsGL::recreateSwapChain(uint32_t width, uint32_t height) {
    // do nothing
}
