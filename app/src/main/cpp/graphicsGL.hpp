/**
 * Copyright 2018 Cerulean Quasar. All Rights Reserved.
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
#include <map>
#include "graphics.hpp"
#include "level.hpp"
#include "levelFinish.hpp"
#include "levelTracker.hpp"

namespace graphicsGL {
    class Surface {
    public:
        Surface(WindowType *window)
                : m_window{window},
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

        inline int width() { return m_width; }
        inline int height() { return m_height; }
        inline EGLSurface surface() { return m_surface; }
        inline EGLDisplay display() { return m_display; }

        ~Surface() {
            destroyWindow();
        }

    private:
        WindowType *m_window;
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

class TextureDataGL : public TextureData {
public:
    TextureDataGL(std::shared_ptr<TextureDescription> const &textureDescription) {
        createTexture(textureDescription);
    }
    virtual ~TextureDataGL() {
        glDeleteTextures(1, &m_handle);
    }

    inline GLuint handle() const { return m_handle; }
private:
    GLuint m_handle;

    void createTexture(std::shared_ptr<TextureDescription> const &textureDescription);
};

class DrawObjectDataGL : public DrawObjectData {
public:
    DrawObjectDataGL(std::shared_ptr<DrawObject> const &drawObj) {
        createDrawObjectData(drawObj);
    }

    virtual ~DrawObjectDataGL() {
        glDeleteBuffers(1, &m_vertexBuffer);
        glDeleteBuffers(1, &m_indexBuffer);
    }

    inline GLuint vertexBuffer() const { return m_vertexBuffer; }
    inline GLuint indexBuffer() const { return m_indexBuffer; }

private:
    void createDrawObjectData(std::shared_ptr<DrawObject> const &drawObj);

    GLuint m_vertexBuffer;
    GLuint m_indexBuffer;
};

class LevelSequenceGL : public LevelSequence {
public:
    LevelSequenceGL(uint32_t level, uint32_t width, uint32_t height)
        :LevelSequence{width, height},
         m_levelTextures{},
         m_levelStarterTextures{},
         m_levelFinisherTextures{},
         m_level{},
         m_levelFinisher{},
         m_levelStarter{},
         m_levelTracker{level},
         m_levelStarterStaticObjsData{},
         m_levelStarterDynObjsData{},
         m_staticObjsData{},
         m_dynObjsData{},
         m_levelfinisherObjsData{}
    {
        m_levelTracker.setParameters(width, height);
        m_levelStarter = m_levelTracker.getLevelStarter();
        m_level = m_levelTracker.getLevel();
        float x, y;
        m_level->getLevelFinisherCenter(x, y);
        m_levelFinisher = m_levelTracker.getLevelFinisher(x, y);
    }

    void loadTextures(TextureMap &textures);
    void addObjects(DrawObjectTable &objsData);
    bool updateLevelData(Level *level, DrawObjectTable &objsData, TextureMap &textures);
    bool updateData();
    void updateAcceleration(float x, float y, float z);
    glm::vec4 backgroundColor() { return m_level->getBackgroundColor(); }
    void initializeLevels() {
        initializeLevelData(m_level, m_staticObjsData, m_dynObjsData, m_levelTextures);
        initializeLevelData(m_levelStarter, m_levelStarterStaticObjsData, m_levelStarterDynObjsData, m_levelStarterTextures);
    }
    bool needFinisherObjs() { return m_level->isFinished() || m_levelFinisher->isUnveiling(); }

    inline DrawObjectTable const &starterStaticObjsData() { return m_levelStarterStaticObjsData; }
    inline DrawObjectTable const &starterDynObjsData() { return m_levelStarterDynObjsData; }
    inline DrawObjectTable const &levelStaticObjsData() { return m_staticObjsData; }
    inline DrawObjectTable const &levelDynObjsData() { return m_dynObjsData; }
    inline DrawObjectTable const &finisherObjsData() { return m_levelfinisherObjsData; }

    inline TextureMap const &starterTextures() { return m_levelStarterTextures; }
    inline TextureMap const &levelTextures() { return m_levelTextures; }
    inline TextureMap const &finisherTextures() { return m_levelFinisherTextures; }
private:
    TextureMap m_levelTextures;
    TextureMap m_levelStarterTextures;
    TextureMap m_levelFinisherTextures;

    std::shared_ptr<Level> m_level;
    std::shared_ptr<LevelFinish> m_levelFinisher;
    std::shared_ptr<LevelStarter> m_levelStarter;
    LevelTracker m_levelTracker;

    DrawObjectTable m_levelStarterStaticObjsData;
    DrawObjectTable m_levelStarterDynObjsData;
    DrawObjectTable m_staticObjsData;
    DrawObjectTable m_dynObjsData;
    DrawObjectTable m_levelfinisherObjsData;

    void initializeLevelData(std::shared_ptr<Level> const &level, DrawObjectTable &staticObjsData,
                             DrawObjectTable &dynObjsData, TextureMap &textures);
};

class GraphicsGL : public Graphics {
public:
    GraphicsGL(WindowType *window, uint32_t level)
        :m_surface{window},
        programID{},
        depthProgramID{},
        depthMapFBO{},
        depthMap{},
        colorImage{},
        m_levelSequence{level, static_cast<uint32_t>(m_surface.width()),
                        static_cast<uint32_t >(m_surface.height())}
    {
        initPipeline();
    }

    virtual void initThread() { m_surface.initThread(); }

    virtual void cleanupThread() { m_surface.cleanupThread(); }

    virtual bool updateData() { return m_levelSequence.updateData(); }

    virtual void drawFrame();

    virtual void updateAcceleration(float x, float y, float z) {
        m_levelSequence.updateAcceleration(x, y, z);
    }

    virtual void destroyWindow() {}

    virtual void recreateSwapChain();

    virtual ~GraphicsGL() {
        glDeleteFramebuffers(1, &depthMapFBO);
        glDeleteTextures(1, &colorImage);
        glDeleteTextures(1, &depthMap);
        glDeleteShader(programID);
        glDeleteShader(depthProgramID);
    }
private:
    graphicsGL::Surface m_surface;
    GLuint programID;
    GLuint depthProgramID;

    GLuint depthMapFBO;
    GLuint depthMap;
    GLuint colorImage;

    LevelSequenceGL m_levelSequence;

    GLuint loadShaders(std::string const &vertexShaderFile, std::string const &fragmentShaderFile);
    void initPipeline();
    void drawObjects(DrawObjectTable const &objsData, TextureMap const &textures);
    void drawObject(GLuint programID, bool needsNormal, GLuint vertex, GLuint index,
                    unsigned long nbrIndices, GLuint texture, glm::mat4 const &modelMatrix);
    void createDepthTexture();
    void drawObject(GLuint programID, bool needsNormal, GLuint vertex, GLuint index,
                    unsigned long nbrIndices, glm::mat4 const &modelMatrix);
};

#endif