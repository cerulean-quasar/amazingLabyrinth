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
#ifndef AMAZING_LABYRINTH_MAZE_GL_HPP
#define AMAZING_LABYRINTH_MAZE_GL_HPP

#include <map>
#include "graphicsGL.hpp"
#include "mazeGraphics.hpp"
#include "levels/basic/level.hpp"
#include "levels/finisher/types.hpp"
#include "levelTracker/levelTracker.hpp"
#include "renderLoader/renderLoaderGL.hpp"
#include "levelDrawer/levelDrawerGL.hpp"

class GraphicsGL : public Graphics {
public:
    GraphicsGL(std::shared_ptr<WindowType> window,
               bool shadowsEnabled,
               std::shared_ptr<GameRequester> inGameRequester,
               float rotationAngle)
            : Graphics{std::move(inGameRequester), rotationAngle},
              m_surface{std::make_shared<graphicsGL::Surface>(std::move(window))},
              m_surfaceDetails{std::make_shared<graphicsGL::SurfaceDetails>(
                      graphicsGL::SurfaceDetails{m_surface->width(), m_surface->height(),
                                                 /*m_surface->glVersion() == graphicsGL::Surface::GL_GRAPHICS_VERSION_3*/ false})},
              m_renderLoader{std::make_shared<RenderLoaderGL>()},
              m_levelDrawer{std::make_shared<levelDrawer::LevelDrawerGL>(
                      levelDrawer::NeededForDrawingGL{},
                      m_surfaceDetails, m_renderLoader,
                      shadowsEnabled ? shadowsChainingRenderDetailsName : objectNoShadowsRenderDetailsName,
                      m_gameRequester)}
    {
        initPipeline(shadowsEnabled);

        m_levelSequence = std::make_shared<LevelSequence>(
                m_gameRequester, m_levelDrawer, static_cast<uint32_t>(m_surface->width()),
                static_cast<uint32_t >(m_surface->height()));
    }

    void initThread() override { m_surface->initThread(); }

    void cleanupThread() override { m_surface->cleanupThread(); }

    bool updateData(bool alwaysUpdateDynObjs) override { return m_levelSequence->updateData(alwaysUpdateDynObjs); }

    void drawFrame() override {
        m_levelDrawer->draw(levelDrawer::DrawArgumentGL{m_surface->width(), m_surface->height()});

        eglSwapBuffers(m_surface->display(), m_surface->surface());
    }

    bool isVulkanImplementation() override { return false; }

    void recreateSwapChain(uint32_t width, uint32_t height) override;

    std::vector<std::string> getBugList() {
        if (m_surfaceDetails->useIntTexture) {
            return std::vector<std::string>();
        } else {
            return std::vector<std::string>{"OpenGL implementation does not support integer surfaces."};
        }
    }

    GraphicsDescription graphicsDescription() override {
        return GraphicsDescription{
            "OpenGL ES",
            reinterpret_cast<char const *>(glGetString(GL_VERSION)),
            std::string(reinterpret_cast<char const *>(glGetString(GL_VENDOR))) + " " +
                std::string(reinterpret_cast<char const*>(glGetString(GL_RENDERER))),
            getBugList()};
    }

    ~GraphicsGL() override {
        destroyResources();
    }
private:
    std::shared_ptr<graphicsGL::Surface> m_surface;
    std::shared_ptr<graphicsGL::SurfaceDetails> m_surfaceDetails;
    std::shared_ptr<RenderLoaderGL> m_renderLoader;
    std::shared_ptr<levelDrawer::LevelDrawerGL> m_levelDrawer;

    void initPipeline(bool enableShadows, bool testFramebuffer = true);

    void destroyResources() {
        m_renderLoader->clearRenderDetails();
    }
};

#endif // AMAZING_LABYRINTH_MAZE_GL_HPP