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

        void createSurface();
        void destroyWindow();
    };
} /* namespace graphicsGL */

#endif