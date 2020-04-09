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

class Framebuffer {
public:
    struct ColorImageFormat {
        GLint internalFormat;
        GLenum format;
        GLenum type;
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
    glm::mat4 getPerspectiveMatrixForLevel(uint32_t surfaceWidth, uint32_t surfaceHeight) override;

    LevelSequenceGL(std::shared_ptr<GameRequester> inRequester, uint32_t width, uint32_t height)
            : LevelSequence{inRequester, width, height, true}
    {
    }

protected:
    void updatePerspectiveMatrix(uint32_t surfaceWidth, uint32_t surfaceHeight) override;
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
              m_linearDepthProgramID{},
              m_normalProgramID{},
              m_useIntTexture{true},
              m_framebufferShadowMap{}
    {
        m_levelSequence = std::make_shared<LevelSequenceGL>(m_gameRequester, static_cast<uint32_t>(m_surface.width()),
                        static_cast<uint32_t >(m_surface.height()));
        initPipeline();

        if (!testDepthTexture(false)) {
            m_useIntTexture = false;
            if (!testDepthTexture(false)) {
                throw std::runtime_error(
                        "This version of OpenGL has bugs making it impossible to get the depth texture and normal map.");
            }
        }
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
            float height,
            uint32_t nbrSampleForWidth,
            float farthestDepth,
            float nearestDepth,
            std::vector<float> &depthValues,
            std::vector<glm::vec3> &normalMap);

    virtual ~GraphicsGL() {
        glDeleteShader(programID);
        glDeleteShader(depthProgramID);
        glDeleteShader(m_linearDepthProgramID);
        glDeleteShader(m_normalProgramID);
    }
private:
    graphicsGL::Surface m_surface;
    GLuint programID;
    GLuint depthProgramID;
    GLuint m_linearDepthProgramID;
    GLuint m_normalProgramID;
    bool m_useIntTexture;

    std::shared_ptr<Framebuffer> m_framebufferShadowMap;

    GLuint loadShaders(std::string const &vertexShaderFile, std::string const &fragmentShaderFile);
    void initPipeline();
    void drawObjects(DrawObjectTable const &objsData, TextureMap const &textures);
    void drawObject(GLuint programID, bool needsNormal, GLuint vertex, GLuint index,
                    unsigned long nbrIndices, GLuint texture, glm::mat4 const &modelMatrix);
    void createDepthTexture();
    void drawObject(GLuint programID, bool needsNormal, GLuint vertex, GLuint index,
                    unsigned long nbrIndices, glm::mat4 const &modelMatrix);

    template <typename data_type>
    std::shared_ptr<TextureData> getDepthTextureTemplate(
            DrawObjectTable const &objsData,
            Framebuffer::ColorImageFormat colorImageFormat,
            float width,
            float height,
            uint32_t rowSize,
            float farthestDepth,
            float nearestDepth,
            std::vector<float> &depthMap, /* output */
            std::vector<glm::vec3> &normalMap); /* output */
};

#endif // AMAZING_LABYRINTH_MAZE_GL_HPP