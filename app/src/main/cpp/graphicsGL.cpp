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

#include <android/native_window.h>
#include <cstring>
#include <array>

#include "graphicsGL.hpp"
#include "android.hpp"

namespace graphicsGL {
    void Surface::createSurface() {
        /*
        static EGLint const attribute_list[] = {
                EGL_RED_SIZE, 1,
                EGL_GREEN_SIZE, 1,
                EGL_BLUE_SIZE, 1,
                EGL_NONE
        };
         */

        std::array<EGLint, 21> attribute_list = {
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
                EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_ALPHA_SIZE, 8,
                EGL_DEPTH_SIZE, 24,
                EGL_NONE};

        // Initialize EGL
        if ((m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
            destroyWindow();
            throw std::runtime_error("Could not open display");
        }

        EGLint majorVersion;
        EGLint minorVersion;
        if (!eglInitialize(m_display, &majorVersion, &minorVersion)) {
            destroyWindow();
            throw std::runtime_error("Could not initialize display");
        }

        EGLint nbr_config;
        if (!eglChooseConfig(m_display, attribute_list.data(), &m_config, 1, &nbr_config)) {
            destroyWindow();
            throw std::runtime_error("Could not get config");
        }

        if (nbr_config == 0) {
            destroyWindow();
            throw std::runtime_error("Got 0 configs.");
        }

        int32_t format;
        if (!eglGetConfigAttrib(m_display, m_config, EGL_NATIVE_VISUAL_ID, &format)) {
            destroyWindow();
            throw std::runtime_error("Could not get display format");
        }
        ANativeWindow_setBuffersGeometry(m_window.get(), 0, 0, format);

        if ((m_surface = eglCreateWindowSurface(m_display, m_config, m_window.get(), nullptr)) ==
            EGL_NO_SURFACE) {
            destroyWindow();
            throw std::runtime_error("Could not create surface");
        }

        m_glVersion = GL_GRAPHICS_VERSION_3;
        std::array<EGLint, 3> contextAttributes = {EGL_CONTEXT_CLIENT_VERSION, m_glVersion, EGL_NONE};
        if ((m_context = eglCreateContext(
                m_display, m_config, EGL_NO_CONTEXT, contextAttributes.data())) ==
            EGL_NO_CONTEXT)
        {
            contextAttributes[1] = m_glVersion = GL_GRAPHICS_VERSION_2;
            if ((m_context = eglCreateContext(
                    m_display, m_config, EGL_NO_CONTEXT, contextAttributes.data())) ==
                EGL_NO_CONTEXT)
            {
                destroyWindow();
                throw std::runtime_error("Could not create context");
            }
        }

        if (!eglMakeCurrent(m_display, m_surface, m_surface, m_context)) {
            destroyWindow();
            throw std::runtime_error("Could not set the surface to current");
        }

        if (!eglQuerySurface(m_display, m_surface, EGL_WIDTH, &m_width) ||
            !eglQuerySurface(m_display, m_surface, EGL_HEIGHT, &m_height)) {
            destroyWindow();
            throw std::runtime_error("Could not get width and height of surface");
        }

        // Enable depth test
        glEnable(GL_DEPTH_TEST);
        // Accept fragment if it is closer to the camera than the former one.
        glDepthFunc(GL_LESS);

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        /*
        GLint range[2];
        GLint precision;
        glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_HIGH_FLOAT, range, &precision);
        glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_HIGH_INT, range, &precision);
        glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_MEDIUM_FLOAT, range, &precision);
        glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_MEDIUM_INT, range, &precision);
        checkGraphicsError();
        */
    }

    void Surface::destroyWindow() {
        if (m_display != EGL_NO_DISPLAY) {
            eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (m_context != EGL_NO_CONTEXT) {
                eglDestroyContext(m_display, m_context);
            }
            if (m_surface != EGL_NO_SURFACE) {
                eglDestroySurface(m_display, m_surface);
            }
            eglTerminate(m_display);
        }
        m_display = EGL_NO_DISPLAY;
        m_context = EGL_NO_CONTEXT;
        m_surface = EGL_NO_SURFACE;
    }

    void Surface::initThread() {
        if (!eglMakeCurrent(m_display, m_surface, m_surface, m_context)) {
            throw std::runtime_error("Could not set the surface to current");
        }
    }

    void Surface::cleanupThread() {
        if (!eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
            throw std::runtime_error("Could not unset the surface to current");
        }
    }

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

} /* namespace graphicsGL */
