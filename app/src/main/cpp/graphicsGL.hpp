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

class GraphicsGL : public Graphics {
public:
    GraphicsGL(WindowType *window, uint32_t level)
        :window{},
        context{},
        config{},
        surface{},
        display{},
        programID{},
        depthProgramID{},
        levelTextures{},
        levelStarterTextures{},
        levelFinisherTextures{},
        width{},
        height{},
        depthMapFBO{},
        depthMap{},
        colorImage{},
        maze{},
        levelFinisher{},
        levelStarter{},
        levelTracker{level},
        levelStarterStaticObjsData{},
        levelStarterDynObjsData{},
        staticObjsData{},
        dynObjsData{},
        levelfinisherObjsData{}
    { }
    virtual void init(WindowType *window);
    virtual void initThread();

    virtual void cleanupThread();

    virtual void cleanup();

    virtual bool updateData();

    virtual void drawFrame();

    virtual void updateAcceleration(float x, float y, float z);

    virtual void destroyWindow();

    virtual void recreateSwapChain();

    virtual ~GraphicsGL() {}
private:
    WindowType *window;
    EGLContext context;
    EGLConfig config;
    EGLSurface surface;
    EGLDisplay display;
    GLuint programID;
    GLuint depthProgramID;

    class TextureDataGL : public TextureData {
    public:
        GLuint handle;
        virtual ~TextureDataGL() {
            glDeleteTextures(1, &handle);
        }
    };

    TextureMap levelTextures;
    TextureMap levelStarterTextures;
    TextureMap levelFinisherTextures;

    int width;
    int height;

    GLuint depthMapFBO;
    GLuint depthMap;
    GLuint colorImage;
    std::shared_ptr<Level> maze;
    std::shared_ptr<LevelFinish> levelFinisher;
    std::shared_ptr<LevelStarter> levelStarter;
    LevelTracker levelTracker;

    struct DrawObjectDataGL : public DrawObjectData {
        GLuint vertexBuffer;
        GLuint indexBuffer;

        virtual ~DrawObjectDataGL() {
            glDeleteBuffers(1, &vertexBuffer);
            glDeleteBuffers(1, &indexBuffer);
        }
    };

    DrawObjectTable levelStarterStaticObjsData;
    DrawObjectTable levelStarterDynObjsData;
    DrawObjectTable staticObjsData;
    DrawObjectTable dynObjsData;
    DrawObjectTable levelfinisherObjsData;

    GLuint loadShaders(std::string const &vertexShaderFile, std::string const &fragmentShaderFile);
    void initWindow(WindowType *window);
    void initPipeline();
    void drawObjects(DrawObjectTable const &objsData, TextureMap const &textures);
    void drawObject(GLuint programID, bool needsNormal, GLuint vertex, GLuint index,
                    unsigned long nbrIndices, GLuint texture, glm::mat4 const &modelMatrix);
    void loadTextures(TextureMap &textures);
    void addObjects(DrawObjectTable &objsData);
    void addObject(DrawObjectEntry &objData);
    bool updateLevelData(Level *level, DrawObjectTable &objsData, TextureMap &textures);
    void initializeLevelData(Level *level, DrawObjectTable &staticObjsData,
                             DrawObjectTable &dynObjsData, TextureMap &textures);
    void createDepthTexture();
    void drawObject(GLuint programID, bool needsNormal, GLuint vertex, GLuint index,
                    unsigned long nbrIndices, glm::mat4 const &modelMatrix);
};

#endif