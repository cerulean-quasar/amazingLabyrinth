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

    int width;
    int height;
    if (!eglQuerySurface(display, surface, EGL_WIDTH, &width) ||
        !eglQuerySurface(display, surface, EGL_HEIGHT, &height)) {
        destroyWindow();
        throw std::runtime_error("Could not get width and height of surface");
    }

    // The clear background color is transparent black.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it is closer to the camera than the former one.
    glDepthFunc(GL_LESS);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    glViewport(0, 0, width, height);
}

void GraphicsGL::initPipeline() {
    programID = loadShaders(SHADER_VERT_FILE, SHADER_FRAG_FILE);
    depthProgramID = loadShaders(DEPTH_VERT_FILE, DEPTH_FRAG_FILE);

    int32_t w = ANativeWindow_getWidth(window);
    int32_t h = ANativeWindow_getHeight(window);
    maze.loadModels();
    maze.setView();
    maze.updatePerspectiveMatrix(w, h);
    maze.generate();
    maze.generateModelMatrices();

    // the vertex buffer for a maze wall segment (to be drawn many times to get the full maze).
    glGenBuffers(1, &vertexBufferMazeWall);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferMazeWall);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * maze.getVertices().size(),
                 maze.getVertices().data(), GL_STATIC_DRAW);

    // the index buffer for a maze wall segment
    glGenBuffers(1, &indexBufferMazeWall);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferMazeWall);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * maze.getIndices().size(),
                 maze.getIndices().data(), GL_STATIC_DRAW);

    // the vertex buffer for the maze floor
    glGenBuffers(1, &vertexBufferMazeFloor);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferMazeFloor);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * maze.getFloorVertices().size(),
                 maze.getFloorVertices().data(), GL_STATIC_DRAW);

    // the index buffer for the maze floor
    glGenBuffers(1, &indexBufferMazeFloor);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferMazeFloor);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * maze.getFloorIndices().size(),
                 maze.getFloorIndices().data(), GL_STATIC_DRAW);

    // the vertex buffer for the ball
    glGenBuffers(1, &vertexBufferBall);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferBall);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * maze.getBallVertices().size(),
                 maze.getBallVertices().data(), GL_STATIC_DRAW);

    // the index buffer for the ball
    glGenBuffers(1, &indexBufferBall);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferBall);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * maze.getBallIndices().size(),
                 maze.getBallIndices().data(), GL_STATIC_DRAW);

    // the vertex buffer for the hole
    glGenBuffers(1, &vertexBufferHole);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferHole);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * maze.getHoleVertices().size(),
                 maze.getHoleVertices().data(), GL_STATIC_DRAW);

    // the index buffer for the hole
    glGenBuffers(1, &indexBufferHole);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferHole);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * maze.getHoleIndices().size(),
                 maze.getHoleIndices().data(), GL_STATIC_DRAW);

    loadTexture(TEXTURE_PATH_WALLS, textureWall);
    loadTexture(TEXTURE_PATH_FLOOR, textureFloor);
    loadTexture(TEXTURE_PATH_BALL, textureBall);
    loadTexture(TEXTURE_PATH_HOLE, textureHole);

    // for shadow mapping.
    glGenFramebuffers(1, &depthMapFBO);

    // needed because OpenGLES 2.0 does not have glReadBuffer or glDrawBuffer, so we need a color attachment.
    glGenTextures(1, &colorImage);
    glBindTexture(GL_TEXTURE_2D, colorImage);
    glActiveTexture(GL_TEXTURE0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glActiveTexture(GL_TEXTURE0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
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

void GraphicsGL::loadTexture(std::string const &texturePath, GLuint &texture) {
    // load the textures
    glGenTextures(1, &texture);
    //glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // when sampling outside of the texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // when the texture is scaled up or down
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, /*GL_LINEAR_MIPMAP_LINEAR*/GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, /*GL_LINEAR*/ GL_NEAREST);

    int texHeight;
    int texWidth;
    int texChannels;

    AssetStreambuf imageStreambuf(assetWrapper->getAsset(texturePath));
    std::istream imageStream(&imageStreambuf);

    stbi_io_callbacks clbk;
    clbk.read = istreamRead;
    clbk.skip = istreamSkip;
    clbk.eof = istreamEof;

    stbi_uc *pixels = stbi_load_from_callbacks(&clbk, &imageStream, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    /* free the CPU memory for the image */
    stbi_image_free(pixels);

    glGenerateMipmap(GL_TEXTURE_2D);
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
    glDeleteBuffers(1, &vertexBufferMazeWall);
    glDeleteBuffers(1, &indexBufferMazeWall);
    glDeleteTextures(1, &textureWall);

    glDeleteBuffers(1, &vertexBufferMazeFloor);
    glDeleteBuffers(1, &indexBufferMazeFloor);
    glDeleteTextures(1, &textureFloor);

    glDeleteBuffers(1, &vertexBufferBall);
    glDeleteBuffers(1, &indexBufferBall);
    glDeleteTextures(1, &textureBall);

    glDeleteBuffers(1, &vertexBufferHole);
    glDeleteBuffers(1, &indexBufferHole);
    glDeleteTextures(1, &textureHole);

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

    glm::mat4 proj = maze.getProjectionMatrix();
    glm::mat4 view = maze.getViewLightSource();
    GLint MatrixID;
    MatrixID = glGetUniformLocation(depthProgramID, "view");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &view[0][0]);
    MatrixID = glGetUniformLocation(depthProgramID, "proj");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &proj[0][0]);

    //drawObject(depthProgramID, false, vertexBufferMazeFloor, indexBufferMazeFloor, maze.getFloorIndices().size(), maze.getFloorModelMatrix());
    drawObject(depthProgramID, false, vertexBufferBall, indexBufferBall, maze.getBallIndices().size(), maze.getBallModelMatrix());
    //drawObject(depthProgramID, false, vertexBufferHole, indexBufferHole, maze.getHoleIndices().size(), maze.getHoleModelMatrix());
    for (auto&& model : maze.getModelMatricesMaze()) {
        drawObject(depthProgramID, false, vertexBufferMazeWall, indexBufferMazeWall, maze.getIndices().size(), model);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GraphicsGL::drawFrame() {
    createDepthTexture();

    // set the shader to use
    glUseProgram(programID);
    glCullFace(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 proj = maze.getProjectionMatrix();
    glm::mat4 view = maze.getViewMatrix();
    glm::mat4 lightSpaceMatrix = maze.getProjectionMatrix() * maze.getViewLightSource();
    GLint MatrixID;
    MatrixID = glGetUniformLocation(programID, "view");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &view[0][0]);
    MatrixID = glGetUniformLocation(programID, "proj");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &proj[0][0]);
    MatrixID = glGetUniformLocation(programID, "lightSpaceMatrix");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &lightSpaceMatrix[0][0]);

    GLint lightPosID = glGetUniformLocation(programID, "lightPos");
    glm::vec3 lightPos = maze.getLightingSource();
    glUniform3fv(lightPosID, 1, &lightPos[0]);

    GLint textureID = glGetUniformLocation(programID, "texShadowMap");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glUniform1i(textureID, 0);

    drawObject(programID, true, vertexBufferMazeFloor, indexBufferMazeFloor, maze.getFloorIndices().size(), textureFloor, maze.getFloorModelMatrix());
    drawObject(programID, true, vertexBufferBall, indexBufferBall, maze.getBallIndices().size(), textureBall, maze.getBallModelMatrix());
    drawObject(programID, true, vertexBufferHole, indexBufferHole, maze.getHoleIndices().size(), textureHole, maze.getHoleModelMatrix());
    for (auto&& model : maze.getModelMatricesMaze()) {
        drawObject(programID, true, vertexBufferMazeWall, indexBufferMazeWall, maze.getIndices().size(), textureWall, model);
    }
    eglSwapBuffers(display, surface);
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
    maze.updateAcceleration(x,y,z);
}

bool GraphicsGL::updateData() {
    return maze.updateData();
}