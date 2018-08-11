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

#include <native_window.h>
#include <string.h>
#include <jni.h>
#include <istream>

#include <stb_image.h>

#include "maze.hpp"
#include "graphicsGL.hpp"
#include "android.hpp"

std::string const SHADER_VERT_FILE("shaders/shaderGL.vert");
std::string const SHADER_FRAG_FILE("shaders/shaderGL.frag");
std::string const DEPTH_VERT_FILE("shaders/depthShaderGL.vert");
std::string const DEPTH_FRAG_FILE("shaders/depthShaderGL.frag");


void GraphicsGL::init(WindowType *inWindow) {
    initWindow(inWindow);
    initPipeline();
}

void GraphicsGL::initWindow(WindowType *inWindow){
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
        EGL_NONE };

    window = inWindow;

    // Initialize EGL
    if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
        destroyWindow();
        throw std::runtime_error("Could not open display");
    }

    EGLint majorVersion;
    EGLint minorVersion;
    if (!eglInitialize(display, &majorVersion, &minorVersion)) {
        destroyWindow();
        throw std::runtime_error("Could not initialize display");
    }

    EGLint nbr_config;
    if (!eglChooseConfig(display, attribute_list, &config, 1, &nbr_config)) {
        destroyWindow();
        throw std::runtime_error("Could not get config");
    }

    if (nbr_config == 0) {
        destroyWindow();
        throw std::runtime_error("Got 0 configs.");
    }

    int32_t format;
    if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)) {
        destroyWindow();
        throw std::runtime_error("Could not get display format");
    }
    ANativeWindow_setBuffersGeometry(window, 0, 0, format);

    if ((surface = eglCreateWindowSurface(display, config, window, nullptr)) == EGL_NO_SURFACE) {
        destroyWindow();
        throw std::runtime_error("Could not create surface");
    }

    EGLint contextAttributes[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    if ((context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttributes)) == EGL_NO_CONTEXT) {
        destroyWindow();
        throw std::runtime_error("Could not create context");
    }

    if (!eglMakeCurrent(display, surface, surface, context)) {
        destroyWindow();
        throw std::runtime_error("Could not set the surface to current");
    }

    if (!eglQuerySurface(display, surface, EGL_WIDTH, &width) ||
        !eglQuerySurface(display, surface, EGL_HEIGHT, &height)) {
        destroyWindow();
        throw std::runtime_error("Could not get width and height of surface");
    }

    levelTracker.setParameters(width, height);
    levelStarter = levelTracker.getLevelStarter();
    maze = levelTracker.getLevel();
    float x, y;
    maze->getLevelFinisherCenter(x, y);
    levelFinisher = levelTracker.getLevelFinisher(x, y);

    // The clear background color
    glm::vec4 bgColor = maze->getBackgroundColor();
    glClearColor(bgColor.r, bgColor.g, bgColor.b, bgColor.a);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it is closer to the camera than the former one.
    glDepthFunc(GL_LESS);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, width, height);
}

void GraphicsGL::initPipeline() {
    programID = loadShaders(SHADER_VERT_FILE, SHADER_FRAG_FILE);
    depthProgramID = loadShaders(DEPTH_VERT_FILE, DEPTH_FRAG_FILE);

    initializeLevelData(maze.get(), staticObjsData, dynObjsData, levelTextures);
    initializeLevelData(levelStarter.get(), levelStarterStaticObjsData, levelStarterDynObjsData, levelStarterTextures);

    // for shadow mapping.
    glGenFramebuffers(1, &depthMapFBO);

    // needed because OpenGLES 2.0 does not have glReadBuffer or glDrawBuffer, so we need a color attachment.
    glGenTextures(1, &colorImage);
    glBindTexture(GL_TEXTURE_2D, colorImage);
    glActiveTexture(GL_TEXTURE0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glActiveTexture(GL_TEXTURE0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorImage, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
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

    // needed because we are going to switch to another thread now
    if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
        destroyWindow();
        throw std::runtime_error("Could not unset the surface to current");
    }
}

void GraphicsGL::addObjects(DrawObjectTable &objsData) {
    for (auto &&obj : objsData) {
        addObject(obj);
    }
}

void GraphicsGL::addObject(DrawObjectEntry &objData) {
    std::shared_ptr<DrawObjectDataGL> data(new DrawObjectDataGL());

    // the index buffer
    glGenBuffers(1, &(data->indexBuffer));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (uint32_t) * objData.first->indices.size(),
                 objData.first->indices.data(), GL_STATIC_DRAW);

    // the vertex buffer
    glGenBuffers(1, &(data->vertexBuffer));
    glBindBuffer(GL_ARRAY_BUFFER, data->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof (Vertex) * objData.first->vertices.size(),
                 objData.first->vertices.data(), GL_STATIC_DRAW);

    objData.second = data;
}

void GraphicsGL::loadTextures(TextureMap &textures) {
    for (TextureMap::iterator it = textures.begin(); it != textures.end(); it++) {
        if (it->second.get() == nullptr) {
            std::shared_ptr<TextureDataGL> textureData(new TextureDataGL());
            // load the textures
            glGenTextures(1, &textureData->handle);
            glBindTexture(GL_TEXTURE_2D, textureData->handle);

            // when sampling outside of the texture
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // when the texture is scaled up or down
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, /*GL_LINEAR_MIPMAP_LINEAR*/
                            GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, /*GL_LINEAR*/ GL_NEAREST);

            uint32_t texHeight;
            uint32_t texWidth;
            uint32_t texChannels;
            std::vector<char> pixels = it->first->getData(texWidth, texHeight, texChannels);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, pixels.data());

            glGenerateMipmap(GL_TEXTURE_2D);
            it->second = textureData;
        }
    }
}

void GraphicsGL::initThread() {
    if (!eglMakeCurrent(display, surface, surface, context)) {
        throw std::runtime_error("Could not set the surface to current");
    }
}

void GraphicsGL::cleanupThread() {
    if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
        throw std::runtime_error("Could not unset the surface to current");
    }
}

void GraphicsGL::cleanup() {
    staticObjsData.clear();
    dynObjsData.clear();
    levelfinisherObjsData.clear();
    levelStarterStaticObjsData.clear();
    levelStarterDynObjsData.clear();

    glDeleteFramebuffers(1, &depthMapFBO);
    glDeleteTextures(1, &colorImage);
    glDeleteTextures(1, &depthMap);

    destroyWindow();
}

void GraphicsGL::createDepthTexture() {
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

    // set the shader to use
    glUseProgram(depthProgramID);
    glCullFace(GL_FRONT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 proj = maze->getProjectionMatrix();
    glm::mat4 view = maze->getViewLightSource();
    GLint MatrixID;
    MatrixID = glGetUniformLocation(depthProgramID, "view");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &view[0][0]);
    MatrixID = glGetUniformLocation(depthProgramID, "proj");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &proj[0][0]);

    for (auto &&obj : staticObjsData) {
        for (auto &&model : obj.first->modelMatrices) {
            DrawObjectDataGL *data = dynamic_cast<DrawObjectDataGL*> (obj.second.get());
            drawObject(depthProgramID, false, data->vertexBuffer, data->indexBuffer,
                       obj.first->indices.size(), model);
        }
    }

    for (auto &&obj : dynObjsData) {
        for (auto &&model : obj.first->modelMatrices) {
            DrawObjectDataGL *data = dynamic_cast<DrawObjectDataGL*> (obj.second.get());
            drawObject(depthProgramID, false, data->vertexBuffer, data->indexBuffer,
                       obj.first->indices.size(), model);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GraphicsGL::drawFrame() {
    createDepthTexture();

    // set the shader to use
    glUseProgram(programID);
    glCullFace(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 proj = maze->getProjectionMatrix();
    glm::mat4 view = maze->getViewMatrix();
    glm::mat4 lightSpaceMatrix = maze->getProjectionMatrix() * maze->getViewLightSource();
    GLint MatrixID;
    MatrixID = glGetUniformLocation(programID, "view");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &view[0][0]);
    MatrixID = glGetUniformLocation(programID, "proj");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &proj[0][0]);
    MatrixID = glGetUniformLocation(programID, "lightSpaceMatrix");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &lightSpaceMatrix[0][0]);

    GLint lightPosID = glGetUniformLocation(programID, "lightPos");
    glm::vec3 lightPos = maze->getLightingSource();
    glUniform3fv(lightPosID, 1, &lightPos[0]);

    GLint textureID = glGetUniformLocation(programID, "texShadowMap");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glUniform1i(textureID, 0);
    drawObjects(staticObjsData, levelTextures);
    drawObjects(dynObjsData, levelTextures);
    if (levelStarter.get() != nullptr) {
        drawObjects(levelStarterStaticObjsData, levelStarterTextures);
        drawObjects(levelStarterDynObjsData, levelStarterTextures);
    }
    if (maze->isFinished() || levelFinisher->isUnveiling()) {
        drawObjects(levelfinisherObjsData, levelFinisherTextures);
    }
    eglSwapBuffers(display, surface);
}

void GraphicsGL::drawObjects(DrawObjectTable const &objsData, TextureMap const &textures) {
    for (auto&& obj : objsData) {
        DrawObjectDataGL *data = static_cast<DrawObjectDataGL*> (obj.second.get());
        TextureMap::const_iterator it = textures.find(obj.first->texture);
        if (it == textures.end()) {
            throw (std::runtime_error("Could not find texture!"));
        }
        TextureDataGL const *texture = static_cast<TextureDataGL const *> (it->second.get());
        for (auto &&model : obj.first->modelMatrices) {
            drawObject(programID, true, data->vertexBuffer, data->indexBuffer,
                       obj.first->indices.size(), texture->handle, model);
        }
    }
}
void GraphicsGL::drawObject(GLuint programID, bool needsNormal, GLuint vertex, GLuint index,
                            unsigned long nbrIndices, GLuint texture, glm::mat4 const &modelMatrix) {
    GLint textureID = glGetUniformLocation(programID, "texSampler");
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(textureID, 1);

    drawObject(programID, needsNormal, vertex, index, nbrIndices, modelMatrix);
}

void GraphicsGL::drawObject(GLuint programID, bool needsNormal, GLuint vertex, GLuint index,
                            unsigned long nbrIndices, glm::mat4 const &modelMatrix) {
    // Send our transformation to the currently bound shader, in the "MVP"
    // uniform. This is done in the main loop since each model will have a
    // different MVP matrix (At least for the M part)
    GLint MatrixID = glGetUniformLocation(programID, "model");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &modelMatrix[0][0]);

    if (needsNormal) {
        GLint normalMatrixID = glGetUniformLocation(programID, "normalMatrix");
        glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));
        glUniformMatrix4fv(normalMatrixID, 1, GL_FALSE, &normalMatrix[0][0]);
    }

    // 1st attribute buffer : colors
    GLint colorID = glGetAttribLocation(programID, "inColor");
    glBindBuffer(GL_ARRAY_BUFFER, vertex);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index);
    glVertexAttribPointer(
            colorID,                          // The position of the attribute in the shader.
            3,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            sizeof(Vertex),                   // stride
            (void *) (offsetof(Vertex, color))// array buffer offset
    );
    glEnableVertexAttribArray(colorID);

    // attribute buffer : vertices for die
    GLint position = glGetAttribLocation(programID, "inPosition");
    glVertexAttribPointer(
            position,                        // The position of the attribute in the shader.
            3,                               // size
            GL_FLOAT,                        // type
            GL_FALSE,                        // normalized?
            sizeof(Vertex),                  // stride
            (void *) (offsetof(Vertex, pos)) // array buffer offset
    );
    glEnableVertexAttribArray(position);

    // Send in the texture coordinates
    GLint texCoordID = glGetAttribLocation(programID, "inTexCoord");
    glVertexAttribPointer(
            texCoordID,                       // The position of the attribute in the shader
            2,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            sizeof (Vertex),                  // stride
            (void*) offsetof (Vertex, texCoord)  // array buffer offset
    );
    glEnableVertexAttribArray(texCoordID);

    GLint normCoordID = glGetAttribLocation(programID, "inNormal");
    glVertexAttribPointer(
            normCoordID,                      // The position of the attribute in the shader
            3,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            sizeof (Vertex),                  // stride
            (void*) offsetof (Vertex, normal)  // array buffer offset
    );
    glEnableVertexAttribArray(normCoordID);

    // Draw the triangles !
    //glDrawArrays(GL_TRIANGLES, 0, dice[0].die->vertices.size() /* total number of vertices*/);
    glDrawElements(GL_TRIANGLES, nbrIndices, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(position);
    glDisableVertexAttribArray(colorID);
    glDisableVertexAttribArray(texCoordID);
    glDisableVertexAttribArray(normCoordID);

}

GLuint GraphicsGL::loadShaders(std::string const &vertexShaderFile, std::string const &fragmentShaderFile) {
    GLint Result = GL_TRUE;
    GLint InfoLogLength = 0;

    std::vector<char> vertexShader = readFile(vertexShaderFile);
    std::vector<char> fragmentShader = readFile(fragmentShaderFile);

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Compile Vertex Shader
    char const * VertexSourcePointer = vertexShader.data();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (Result == GL_FALSE) {
        if (InfoLogLength > 0) {
            std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
            glGetShaderInfoLog(VertexShaderID, InfoLogLength, &InfoLogLength,
                               VertexShaderErrorMessage.data());
            if (VertexShaderErrorMessage[0] == '\0') {
                throw std::runtime_error("vertex shader compile error");
            } else {
                throw std::runtime_error(std::string("vertex shader compile error: ") + VertexShaderErrorMessage.data());
            }
        } else {
            throw std::runtime_error("vertex shader compile error");
        }
    }

    // Compile Fragment Shader
    char const * FragmentSourcePointer = fragmentShader.data();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (Result == GL_FALSE) {
        if (InfoLogLength > 0) {
            std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
            glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL,
                               FragmentShaderErrorMessage.data());
            if (FragmentShaderErrorMessage[0] == '\0') {
                throw std::runtime_error("Fragment shader compile error.");
            } else {
                throw std::runtime_error(std::string("Fragment shader compile error: ") + FragmentShaderErrorMessage.data());
            }
        } else {
            throw std::runtime_error("Fragment shader compile error.");
        }
    }

    // Link the program
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (Result == GL_FALSE) {
        if (InfoLogLength > 0) {
            std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
            glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, ProgramErrorMessage.data());
            throw std::runtime_error(ProgramErrorMessage.data());
        } else {
            throw std::runtime_error("Linker error.");
        }
    }

    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

void GraphicsGL::destroyWindow() {
    if (display != EGL_NO_DISPLAY) {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (context != EGL_NO_CONTEXT) {
            eglDestroyContext(display, context);
        }
        if (surface != EGL_NO_SURFACE) {
            eglDestroySurface(display, surface);
        }
        eglTerminate(display);
    }
    display = EGL_NO_DISPLAY;
    context = EGL_NO_CONTEXT;
    surface = EGL_NO_SURFACE;

    /* release the java window object */
    ANativeWindow_release(window);
}

void GraphicsGL::recreateSwapChain() {
    // do nothing
}

void GraphicsGL::updateAcceleration(float x, float y, float z) {
    if (levelStarter.get() != nullptr) {
        levelStarter->updateAcceleration(x, y, z);
    } else {
        maze->updateAcceleration(x, y, z);
    }
}

bool GraphicsGL::updateData() {
    if (maze->isFinished() || levelFinisher->isUnveiling()) {
        if (levelFinisher->isUnveiling()) {
            if (levelFinisher->isDone()) {
                float x, y;
                maze->getLevelFinisherCenter(x, y);
                levelFinisher = levelTracker.getLevelFinisher(x, y);
                levelfinisherObjsData.clear();
                levelStarter->start();
                return false;
            }
        } else {
            if (levelFinisher->isDone()) {
                levelTracker.gotoNextLevel();
                maze = levelTracker.getLevel();
                levelStarter = levelTracker.getLevelStarter();

                // The clear background color
                glm::vec4 bgColor = maze->getBackgroundColor();
                glClearColor(bgColor.r, bgColor.g, bgColor.b, bgColor.a);

                initializeLevelData(maze.get(), staticObjsData, dynObjsData, levelTextures);
                initializeLevelData(levelStarter.get(), levelStarterStaticObjsData,
                                    levelStarterDynObjsData, levelStarterTextures);
                levelFinisher->unveilNewLevel();
                return false;
            }
        }

        bool texturesChanged;
        bool drawingNecessary = levelFinisher->updateDrawObjects(levelfinisherObjsData,
                                                                 levelFinisherTextures,
                                                                 texturesChanged);
        if (!drawingNecessary) {
            return false;
        }

        if (texturesChanged) {
            loadTextures(levelFinisherTextures);
        }

        for (auto &&obj : levelfinisherObjsData) {
            if (obj.second.get() == nullptr) {
                addObject(obj);
            }
        }
    } else if (levelStarter.get() != nullptr) {
        updateLevelData(levelStarter.get(), levelStarterDynObjsData, levelStarterTextures);
        if (levelStarter->isFinished()) {
            levelStarter.reset();
            levelStarterDynObjsData.clear();
            levelStarterStaticObjsData.clear();
            levelStarterTextures.clear();
            maze->start();
            return false;
        }
    } else {
        updateLevelData(maze.get(), dynObjsData, levelTextures);
    }

    return true;
}

void GraphicsGL::initializeLevelData(Level *level, DrawObjectTable &staticObjsData,
                                     DrawObjectTable &dynObjsData, TextureMap &textures) {
    dynObjsData.clear();
    staticObjsData.clear();
    textures.clear();
    level->updateStaticDrawObjects(staticObjsData, textures);
    bool texturesChanged;
    level->updateDynamicDrawObjects(dynObjsData, textures, texturesChanged);
    loadTextures(textures);
    addObjects(staticObjsData);
    addObjects(dynObjsData);
}

bool GraphicsGL::updateLevelData(Level *level, DrawObjectTable &objsData, TextureMap &textures) {
    bool drawingNecessary = level->updateData();

    if (!drawingNecessary) {
        return false;
    }

    bool texturesChanged;
    level->updateDynamicDrawObjects(objsData, textures, texturesChanged);
    if (texturesChanged) {
        loadTextures(textures);
    }

    return texturesChanged;
}