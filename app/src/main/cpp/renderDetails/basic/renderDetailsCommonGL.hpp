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
#ifndef AMAZING_LABYRINTH_RENDER_DETAILS_COMMON_GL_HPP
#define AMAZING_LABYRINTH_RENDER_DETAILS_COMMON_GL_HPP
#include <memory>
#include <string>

#include <GLES3/gl3.h>

#include "../../common.hpp"

namespace renderDetails {
    class Framebuffer {
    public:
        struct ColorImageFormat {
            GLint internalFormat;
            GLenum format;
            GLenum type;

            ColorImageFormat(
                    GLint internalFormat_,
                    GLenum format_,
                    GLenum type_) {
                internalFormat = internalFormat_;
                format = format_;
                type = type_;
            }

            ColorImageFormat(ColorImageFormat const &other) {
                internalFormat = other.internalFormat;
                format = other.format;
                type = other.type;
            }

            ColorImageFormat(ColorImageFormat &&other) {
                internalFormat = other.internalFormat;
                format = other.format;
                type = other.type;
            }
        };
        Framebuffer(uint32_t width, uint32_t height, std::vector<ColorImageFormat> colorImageFormats);
        ~Framebuffer() {
            if (m_depthMapFBO != GL_INVALID_VALUE) {
                glDeleteFramebuffers(1, &m_depthMapFBO);
            }

            if (m_depthMap != GL_INVALID_VALUE) {
                glDeleteTextures(1, &m_depthMap);
            }

            for (auto &colorImage : m_colorImage) {
                if (colorImage != GL_INVALID_VALUE) {
                    glDeleteTextures(1, &colorImage);
                    colorImage = GL_INVALID_VALUE;
                }
            }
        }

        GLuint fbo() { return m_depthMapFBO; }
        GLuint depthImage() { return m_depthMap; }
        GLuint colorImage(uint32_t attachmentNbr = 0) { return m_colorImage[attachmentNbr]; }

        GLuint acquireDepthImage() {
            GLuint depthMap_ = m_depthMap;
            m_depthMap = GL_INVALID_VALUE;
            return depthMap_;
        }

        GLuint acquireColorImage(uint32_t attachmentNbr = 0) {
            GLuint colorImage = m_colorImage[attachmentNbr];
            m_colorImage[attachmentNbr] = GL_INVALID_VALUE;
            return colorImage;
        }
    private:
        GLuint m_depthMapFBO;
        GLuint m_depthMap;
        std::vector<GLuint> m_colorImage;
    };

    GLuint loadShaders(std::shared_ptr<GameRequester> const &gameRequester,
                       std::string const &vertexShaderFile, std::string const &fragmentShaderFile);
}

#endif // AMAZING_LABYRINTH_RENDER_DETAILS_COMMON_GL_HPP