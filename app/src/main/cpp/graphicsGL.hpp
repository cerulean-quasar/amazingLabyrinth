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
#ifndef AMAZING_LABYRINTH_GRAPHICS_GL_HPP
#define AMAZING_LABYRINTH_GRAPHICS_GL_HPP

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <memory>

#include "android.hpp"

namespace graphicsGL {
    class Surface {
    public:
        static int constexpr GL_GRAPHICS_VERSION_3 = 3;
        static int constexpr GL_GRAPHICS_VERSION_2 = 2;

        Surface(std::shared_ptr<WindowType> window)
                : m_window{std::move(window)},
                  m_context{EGL_NO_CONTEXT},
                  m_config{},
                  m_surface{EGL_NO_SURFACE},
                  m_display{EGL_NO_DISPLAY},
                  m_width{0},
                  m_height{0}
        {
            createSurface();
        }

        void initThread();
        void cleanupThread();

        inline uint32_t width() { return static_cast<uint32_t>(m_width); }
        inline uint32_t height() { return static_cast<uint32_t>(m_height); }
        inline EGLSurface surface() { return m_surface; }
        inline EGLDisplay display() { return m_display; }
        inline std::shared_ptr<WindowType> window() { return m_window; }
        inline int glVersion() { return m_glVersion; }

        ~Surface() {
            destroyWindow();
        }

    private:
        std::shared_ptr<WindowType> m_window;
        EGLContext m_context;
        EGLConfig m_config;
        EGLSurface m_surface;
        EGLDisplay m_display;

        int m_width;
        int m_height;
        int m_glVersion;

        void createSurface();
        void destroyWindow();
    };

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

            ColorImageFormat(ColorImageFormat const &other) = default;

            ColorImageFormat(ColorImageFormat &&other) = default;

            ColorImageFormat &operator=(ColorImageFormat const &other) = default;
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

    struct SurfaceDetails {
        uint32_t surfaceWidth;
        uint32_t surfaceHeight;
        bool useIntTexture;
    };
} /* namespace graphicsGL */

#endif