/**
 * Copyright 2023 Cerulean Quasar. All Rights Reserved.
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
#include <string>
#include <vector>

#include <GLES3/gl3.h>

#include "../common.hpp"

#include "renderDetailsGL.hpp"

namespace renderDetails {
    Shader getShader(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::string const &shaderFile,
            GLenum shaderType)
    {
        std::vector<char> shader = readFile(gameRequester, shaderFile);

        // Create the shaders
        GLuint shaderID = glCreateShader(shaderType);

        // Compile the Shader
        char const *sourcePointer = shader.data();
        GLint shaderLength = shader.size();
        glShaderSource(shaderID, 1, &sourcePointer, &shaderLength);
        glCompileShader(shaderID);

        // Check the Shader
        GLint Result = GL_TRUE;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &Result);
        if (Result == GL_FALSE) {
            GLint InfoLogLength = 0;
            glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
            if (InfoLogLength > 0) {
                std::vector<char> shaderErrorMessage(InfoLogLength + 1);
                glGetShaderInfoLog(shaderID, InfoLogLength, &InfoLogLength,
                                   shaderErrorMessage.data());
                glDeleteShader(shaderID);
                if (shaderErrorMessage[0] == '\0') {
                    throw std::runtime_error("shader: " + shaderFile + " compile error.");
                } else {
                    throw std::runtime_error(std::string("shader: ") + shaderFile + " compile error: " +
                                             shaderErrorMessage.data());
                }
            } else {
                throw std::runtime_error("shader: " + shaderFile + " compile error.");
            }
        }

        auto deleter = [](GLuint shaderID) -> void {
            if (shaderID != 0) {
                glDeleteShader(shaderID);
            }};

        return {shaderID, deleter};
    }

    Program getProgram(std::vector<Shader> const &shaders)
    {
        if (shaders.empty()) {
            throw std::runtime_error("A shader was incorrectly initialized when loading the GL program.");
        }

        // Create the program
        GLuint programID = glCreateProgram();

        for (auto const &shader : shaders) {
            glAttachShader(programID, *(shader.get()));
        }

        glLinkProgram(programID);

        // glLinkProgram doc pages state that once the link step is done, programs can be
        // detached, deleted, etc.
        for (auto const &shader : shaders) {
            glDetachShader(programID, *(shader.get()));
        }

        // Check the program
        GLint Result = GL_TRUE;
        glGetProgramiv(programID, GL_LINK_STATUS, &Result);

        if (Result == GL_FALSE) {
            GLint InfoLogLength = 0;
            glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &InfoLogLength);
            if (InfoLogLength > 0) {
                std::vector<char> ProgramErrorMessage(InfoLogLength + 1, 0);
                glGetProgramInfoLog(programID, InfoLogLength, nullptr, ProgramErrorMessage.data());
                glDeleteProgram(programID);
                throw std::runtime_error(ProgramErrorMessage.data());
            } else {
                glDeleteProgram(programID);
                throw std::runtime_error("glLinkProgram error.");
            }
        }

        auto deleter = [](GLuint programID) -> void {
            if (programID != 0) {
                glDeleteProgram(programID);
            }
        };

        return {programID, deleter};
    }

    void RenderDetailsGL::drawVertices(
            GLuint programID,
            std::shared_ptr<levelDrawer::ModelDataGL> const &modelData,
            bool useVertexNormals)
    {
        GLuint vertexBuffer = useVertexNormals ?
                modelData->vertexBufferWithVertexNormals() :
                modelData->vertexBuffer();
        GLuint indexBuffer = useVertexNormals ?
                modelData->indexBufferWithVertexNormals() :
                modelData->indexBuffer();
        uint32_t nbrIndices = useVertexNormals ?
                modelData->numberIndicesWithVertexNormals() :
                modelData->numberIndices();

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        checkGraphicsError();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
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
                    sizeof(levelDrawer::Vertex),                   // stride
                    (void *) (offsetof(levelDrawer::Vertex, color))// array buffer offset
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
                sizeof(levelDrawer::Vertex),                  // stride
                (void *) (offsetof(levelDrawer::Vertex, pos)) // array buffer offset
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
                    sizeof(levelDrawer::Vertex),                  // stride
                    (void *) offsetof (levelDrawer::Vertex, texCoord)  // array buffer offset
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
                    sizeof(levelDrawer::Vertex),                  // stride
                    (void *) offsetof (levelDrawer::Vertex, normal)  // array buffer offset
            );
            checkGraphicsError();
            glEnableVertexAttribArray(normCoordID);
            checkGraphicsError();
        }

        // Draw the triangles !
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
}