/**
 * Copyright 2019 Cerulean Quasar. All Rights Reserved.
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

#include <string.h>
#include <istream>

#include <stb_image.h>

#include "level/level.hpp"
#include "graphicsGL.hpp"
#include "mazeGL.hpp"

std::string const SHADER_VERT_FILE("shaders/shaderGL.vert");
std::string const SHADER_FRAG_FILE("shaders/shaderGL.frag");
std::string const DEPTH_VERT_FILE("shaders/depthShaderGL.vert");
std::string const DEPTH_FRAG_FILE("shaders/depthShaderGL.frag");

void DrawObjectDataGL::createDrawObjectData(std::shared_ptr<DrawObject> const &drawObj) {
    // the index buffer
    glGenBuffers(1, &(m_indexBuffer));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (uint32_t) * drawObj->indices.size(),
                 drawObj->indices.data(), GL_STATIC_DRAW);

    // the vertex buffer
    glGenBuffers(1, &(m_vertexBuffer));
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof (Vertex) * drawObj->vertices.size(),
                 drawObj->vertices.data(), GL_STATIC_DRAW);
}

void TextureDataGL::createTexture(std::shared_ptr<TextureDescription> const &textureDescription) {

    glGenTextures(1, &m_handle);
    glBindTexture(GL_TEXTURE_2D, m_handle);

    // when sampling outside of the texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // when the texture is scaled up or down
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, /*GL_LINEAR_MIPMAP_LINEAR*/
                    GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, /*GL_LINEAR*/ GL_NEAREST);

    uint32_t texHeight;
    uint32_t texWidth;
    uint32_t texChannels;
    std::vector<char> pixels = textureDescription->getData(texWidth, texHeight, texChannels);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixels.data());

    glGenerateMipmap(GL_TEXTURE_2D);
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

    // The clear background color
    glm::vec4 bgColor = m_levelSequence->backgroundColor();
    glClearColor(bgColor.r, bgColor.g, bgColor.b, bgColor.a);

    programID = loadShaders(SHADER_VERT_FILE, SHADER_FRAG_FILE);
    depthProgramID = loadShaders(DEPTH_VERT_FILE, DEPTH_FRAG_FILE);

    // for shadow mapping.
    glGenFramebuffers(1, &depthMapFBO);

    // needed because OpenGLES 2.0 does not have glReadBuffer or glDrawBuffer, so we need a color attachment.
    glGenTextures(1, &colorImage);
    glBindTexture(GL_TEXTURE_2D, colorImage);
    glActiveTexture(GL_TEXTURE0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_surface.width(), m_surface.height(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glActiveTexture(GL_TEXTURE0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_surface.width(), m_surface.height(), 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorImage, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
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
}

void GraphicsGL::createDepthTexture() {
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

    // set the shader to use
    glUseProgram(depthProgramID);
    glCullFace(GL_FRONT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 proj = m_levelSequence->projectionMatrix();
    glm::mat4 view = m_levelSequence->viewMatrix();

    GLint MatrixID;
    MatrixID = glGetUniformLocation(depthProgramID, "view");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &view[0][0]);
    MatrixID = glGetUniformLocation(depthProgramID, "proj");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &proj[0][0]);

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
}

void GraphicsGL::drawFrame() {
    createDepthTexture();

    // set the shader to use
    glUseProgram(programID);
    glCullFace(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // The clear background color
    glm::vec4 bgColor = m_levelSequence->backgroundColor();
    glClearColor(bgColor.r, bgColor.g, bgColor.b, bgColor.a);

    glm::mat4 proj = m_levelSequence->projectionMatrix();
    glm::mat4 view = m_levelSequence->viewMatrix();

    glm::mat4 lightSpaceMatrix = proj * m_levelSequence->viewLightSource();
    GLint MatrixID;
    MatrixID = glGetUniformLocation(programID, "view");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &view[0][0]);
    MatrixID = glGetUniformLocation(programID, "proj");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &proj[0][0]);
    MatrixID = glGetUniformLocation(programID, "lightSpaceMatrix");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &lightSpaceMatrix[0][0]);

    GLint lightPosID = glGetUniformLocation(programID, "lightPos");
    glm::vec3 lightPos = m_levelSequence->lightingSource();
    glUniform3fv(lightPosID, 1, &lightPos[0]);

    GLint textureID = glGetUniformLocation(programID, "texShadowMap");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glUniform1i(textureID, 0);
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
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(textureID, 1);

    drawObject(programID, needsNormal, vertex, index, nbrIndices, modelMatrix);
}

void GraphicsGL::drawObject(GLuint programID, bool needsNormal, GLuint vertex, GLuint index,
                            unsigned long nbrIndices, glm::mat4 const &modelMatrix) {
    // Send our transformation to the currently bound shader, in the "MVP"
    // uniform. This is done in the main loop since each model will have a
    // different MVP matrix (At least for the M part)
    GLint MatrixID = glGetUniformLocation(programID, "model");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &modelMatrix[0][0]);

    if (needsNormal) {
        GLint normalMatrixID = glGetUniformLocation(programID, "normalMatrix");
        glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));
        glUniformMatrix4fv(normalMatrixID, 1, GL_FALSE, &normalMatrix[0][0]);
    }

    // 1st attribute buffer : colors
    GLint colorID = glGetAttribLocation(programID, "inColor");
    glBindBuffer(GL_ARRAY_BUFFER, vertex);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index);
    glVertexAttribPointer(
            colorID,                          // The position of the attribute in the shader.
            3,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            sizeof(Vertex),                   // stride
            (void *) (offsetof(Vertex, color))// array buffer offset
    );
    glEnableVertexAttribArray(colorID);

    // attribute buffer : vertices for die
    GLint position = glGetAttribLocation(programID, "inPosition");
    glVertexAttribPointer(
            position,                        // The position of the attribute in the shader.
            3,                               // size
            GL_FLOAT,                        // type
            GL_FALSE,                        // normalized?
            sizeof(Vertex),                  // stride
            (void *) (offsetof(Vertex, pos)) // array buffer offset
    );
    glEnableVertexAttribArray(position);

    // Send in the texture coordinates
    GLint texCoordID = glGetAttribLocation(programID, "inTexCoord");
    glVertexAttribPointer(
            texCoordID,                       // The position of the attribute in the shader
            2,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            sizeof (Vertex),                  // stride
            (void*) offsetof (Vertex, texCoord)  // array buffer offset
    );
    glEnableVertexAttribArray(texCoordID);

    GLint normCoordID = glGetAttribLocation(programID, "inNormal");
    glVertexAttribPointer(
            normCoordID,                      // The position of the attribute in the shader
            3,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            sizeof (Vertex),                  // stride
            (void*) offsetof (Vertex, normal)  // array buffer offset
    );
    glEnableVertexAttribArray(normCoordID);

    // Draw the triangles !
    //glDrawArrays(GL_TRIANGLES, 0, dice[0].die->vertices.size() /* total number of vertices*/);
    glDrawElements(GL_TRIANGLES, nbrIndices, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(position);
    glDisableVertexAttribArray(colorID);
    glDisableVertexAttribArray(texCoordID);
    glDisableVertexAttribArray(normCoordID);

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
