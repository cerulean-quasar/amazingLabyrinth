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

#include <memory>
#include <string>
#include <vector>

#include <GLES3/gl3.h>

#include "../../common.hpp"

#include "renderDetailsCommonGL.hpp"

namespace renderDetails {
    Framebuffer::Framebuffer(uint32_t width, uint32_t height, std::vector<Framebuffer::ColorImageFormat> colorImageFormats)
            : m_depthMapFBO(GL_INVALID_VALUE),
              m_depthMap(GL_INVALID_VALUE),
              m_colorImage{}
    {
        size_t nbrColorAttachments = colorImageFormats.size();
        m_colorImage.resize(nbrColorAttachments, GL_INVALID_VALUE);

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
            GLenum activeTextureIndicator;
            GLenum attachmentIndicator;
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
            glTexImage2D(GL_TEXTURE_2D, 0, colorImageFormats[i].internalFormat, width, height, 0, colorImageFormats[i].format,
                         colorImageFormats[i].type, nullptr);
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

    GLuint loadShaders(std::shared_ptr<GameRequester> const &gameRequester,
                       std::string const &vertexShaderFile, std::string const &fragmentShaderFile) {
        GLint Result = GL_TRUE;
        GLint InfoLogLength = 0;

        std::vector<char> vertexShader = readFile(gameRequester, vertexShaderFile);
        std::vector<char> fragmentShader = readFile(gameRequester, fragmentShaderFile);

        // Create the shaders
        GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

        // Compile Vertex Shader
        char const *VertexSourcePointer = vertexShader.data();
        glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
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
                    throw std::runtime_error(std::string("vertex shader compile error: ") +
                                             VertexShaderErrorMessage.data());
                }
            } else {
                throw std::runtime_error("vertex shader compile error");
            }
        }

        // Compile Fragment Shader
        char const *FragmentSourcePointer = fragmentShader.data();
        glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
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
                    throw std::runtime_error(std::string("Fragment shader compile error: ") +
                                             FragmentShaderErrorMessage.data());
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
}