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
#ifndef AMAZING_LABYRINTH_MAZE_GL_HPP
#define AMAZING_LABYRINTH_MAZE_GL_HPP

#include <map>
#include "graphics.hpp"
#include "graphicsGL.hpp"
#include "mazeGraphics.hpp"
#include "level/level.hpp"
#include "level/levelFinish.hpp"
#include "level/levelTracker.hpp"

class TextureDataGL : public TextureData {
public:
    TextureDataGL(std::shared_ptr<TextureDescription> const &textureDescription) {
        createTexture(textureDescription);
        deleteWhenDone = true; // TODO: can remove, testing
    }

    // TODO: can remove, testing
    TextureDataGL(GLuint texture)
        : deleteWhenDone{false},
        m_handle{texture} {
    }

    virtual ~TextureDataGL() {
        if (deleteWhenDone) { // TODO: can remove, testing
            glDeleteTextures(1, &m_handle);
        }
    }

    inline GLuint handle() const { return m_handle; }
private:
    bool deleteWhenDone; // TODO: can remove, testing
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
    LevelSequenceGL(std::shared_ptr<GameRequester> inRequester, uint32_t width, uint32_t height)
            : LevelSequence{inRequester, width, height, true}
    {
        // Need to call these here because they call virtual functions.
        if (m_levelStarter != nullptr && ! m_levelStarter->isFinished()) {
            initializeLevelData(m_levelStarter, m_levelStarterStaticObjsData,
                                m_levelStarterDynObjsData, m_texturesLevelStarter);
        }
        initializeLevelData(m_level, m_staticObjsData, m_dynObjsData, m_texturesLevel);
    }

protected:
    std::shared_ptr<TextureData> createTexture(std::shared_ptr<TextureDescription> const &textureDescription) override;
    std::shared_ptr<DrawObjectData> createObject(std::shared_ptr<DrawObject> const &obj, TextureMap &textures) override;
    void updateLevelData(DrawObjectTable &objsData, TextureMap &textures) override;
};

class GraphicsGL : public Graphics {
public:
    GraphicsGL(std::shared_ptr<WindowType> window,
               GameRequesterCreator inRequesterCreator)
            : Graphics{inRequesterCreator},
              m_surface{std::move(window)},
              programID{},
              depthProgramID{},
              depthMapFBO{},
              depthMap{},
              colorImage{}
    {
        m_levelSequence = std::make_shared<LevelSequenceGL>(m_gameRequester, static_cast<uint32_t>(m_surface.width()),
                        static_cast<uint32_t >(m_surface.height()));
        initPipeline();
    }

    virtual void initThread() { m_surface.initThread(); }

    virtual void cleanupThread() { m_surface.cleanupThread(); }

    virtual bool updateData() { return m_levelSequence->updateData(); }

    virtual void drawFrame();

    virtual void destroyWindow() {}

    virtual void recreateSwapChain(uint32_t width, uint32_t height);

    virtual GraphicsDescription graphicsDescription() {
        return GraphicsDescription{"OpenGLES", "", ""};
    }

    virtual std::shared_ptr<TextureData> getDepthTexture(
            DrawObjectTable const &objsData,
            float width,
            float height);

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

    GLuint loadShaders(std::string const &vertexShaderFile, std::string const &fragmentShaderFile);
    void initPipeline();
    void drawObjects(DrawObjectTable const &objsData, TextureMap const &textures);
    void drawObject(GLuint programID, bool needsNormal, GLuint vertex, GLuint index,
                    unsigned long nbrIndices, GLuint texture, glm::mat4 const &modelMatrix);
    void createDepthTexture();
    void drawObject(GLuint programID, bool needsNormal, GLuint vertex, GLuint index,
                    unsigned long nbrIndices, glm::mat4 const &modelMatrix);
};

#endif // AMAZING_LABYRINTH_MAZE_GL_HPP