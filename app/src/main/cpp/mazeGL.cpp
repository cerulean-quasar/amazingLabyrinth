/**
 * Copyright 2022 Cerulean Quasar. All Rights Reserved.
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

void GraphicsGL::initPipeline(bool enableShadows, bool testFramebuffer) {
    glViewport(0, 0, m_surface->width(), m_surface->height());

    if (testFramebuffer) {
        if (!m_surfaceDetails->useIntTexture || !testDepthTexture(levelDrawer::Adaptor(levelDrawer::LEVEL, m_levelDrawer))) {
            m_surfaceDetails->useIntTexture = false;
            m_levelDrawer = std::make_shared<levelDrawer::LevelDrawerGL>(
                    levelDrawer::NeededForDrawingGL{}, m_surfaceDetails, m_renderLoader,
                    enableShadows ? shadowsChainingRenderDetailsName : objectNoShadowsRenderDetailsName,
                    m_gameRequester);
            if (!testDepthTexture(levelDrawer::Adaptor(levelDrawer::LEVEL, m_levelDrawer))) {
                throw std::runtime_error(
                        "This version of OpenGL has bugs making it impossible to get the depth texture and normal map.");
            }
        }
    }
}

void GraphicsGL::recreateSwapChain(uint32_t width, uint32_t height) {
    // needs to be called before clean up level data
    bool levelStarterNeeded = m_levelSequence->levelStarterRequired();

    if (m_surface->width() == width && m_surface->height() == height) {
        return;
    }

    m_levelSequence->cleanupLevelData();
    destroyResources();

    auto window = m_surface->window();
    m_surface.reset();

    m_surface = std::make_shared<graphicsGL::Surface>(window);

    m_surfaceDetails->surfaceWidth = width;
    m_surfaceDetails->surfaceHeight = height;

    initPipeline(false);

    m_levelSequence->notifySurfaceChanged(m_surface->width(), m_surface->height(),
            levelStarterNeeded);
}
