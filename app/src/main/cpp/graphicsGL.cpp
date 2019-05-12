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

#include <native_window.h>
#include <string.h>

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
        const EGLint attribute_list[] = {
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
                EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
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
        if (!eglChooseConfig(m_display, attribute_list, &m_config, 1, &nbr_config)) {
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

        EGLint contextAttributes[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
        if ((m_context = eglCreateContext(m_display, m_config, EGL_NO_CONTEXT, contextAttributes)) ==
            EGL_NO_CONTEXT) {
            destroyWindow();
            throw std::runtime_error("Could not create context");
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

} /* namespace graphicsGL */
