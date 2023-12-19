/**
 * Copyright 2023 Cerulean Quasar. All Rights Reserved.
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

#include <memory>
#include <string>
#include <vector>

#include <GLES3/gl3.h>

#include "../common.hpp"

#include "renderDetailsGL.hpp"

namespace renderDetails {
    void RenderDetailsGL::drawVertices(
            GLuint programID,
            std::shared_ptr<levelDrawer::ModelDataGL> const &modelData,
            bool useVertexNormals)
    {
        GLuint vertexBuffer = useVertexNormals ?
                modelData->vertexBufferWithVertexNormals() :
                modelData->vertexBuffer();
        GLuint indexBuffer = useVertexNormals ?
                modelData->indexBufferWithVertexNormals() :
                modelData->indexBuffer();
        uint32_t nbrIndices = useVertexNormals ?
                modelData->numberIndicesWithVertexNormals() :
                modelData->numberIndices();

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        checkGraphicsError();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        checkGraphicsError();

        // 1st attribute buffer : colors
        // color is only needed when doing the final render, not the depth texture render
        GLint colorID = glGetAttribLocation(programID, "inColor");
        if (colorID != -1) {
            glVertexAttribPointer(
                    colorID,                          // The position of the attribute in the shader.
                    3,                                // size
                    GL_FLOAT,                         // type
                    GL_FALSE,                         // normalized?
                    sizeof(levelDrawer::Vertex),                   // stride
                    (void *) (offsetof(levelDrawer::Vertex, color))// array buffer offset
            );
            checkGraphicsError();
            glEnableVertexAttribArray(colorID);
            checkGraphicsError();
        }

        // attribute: position
        GLint position = glGetAttribLocation(programID, "inPosition");
        glVertexAttribPointer(
                position,                        // The position of the attribute in the shader.
                3,                               // size
                GL_FLOAT,                        // type
                GL_FALSE,                        // normalized?
                sizeof(levelDrawer::Vertex),                  // stride
                (void *) (offsetof(levelDrawer::Vertex, pos)) // array buffer offset
        );
        checkGraphicsError();
        glEnableVertexAttribArray(position);
        checkGraphicsError();

        // Send in the texture coordinates
        // only required for the final rendering.
        GLint texCoordID = glGetAttribLocation(programID, "inTexCoord");
        if (texCoordID != -1) {
            glVertexAttribPointer(
                    texCoordID,                       // The position of the attribute in the shader
                    2,                                // size
                    GL_FLOAT,                         // type
                    GL_FALSE,                         // normalized?
                    sizeof(levelDrawer::Vertex),                  // stride
                    (void *) offsetof (levelDrawer::Vertex, texCoord)  // array buffer offset
            );
            checkGraphicsError();
            glEnableVertexAttribArray(texCoordID);
            checkGraphicsError();
        }

        GLint normCoordID = glGetAttribLocation(programID, "inNormal");
        if (normCoordID != -1) {
            checkGraphicsError();
            glVertexAttribPointer(
                    normCoordID,                      // The position of the attribute in the shader
                    3,                                // size
                    GL_FLOAT,                         // type
                    GL_FALSE,                         // normalized?
                    sizeof(levelDrawer::Vertex),                  // stride
                    (void *) offsetof (levelDrawer::Vertex, normal)  // array buffer offset
            );
            checkGraphicsError();
            glEnableVertexAttribArray(normCoordID);
            checkGraphicsError();
        }

        // Draw the triangles !
        glDrawElements(GL_TRIANGLES, nbrIndices, GL_UNSIGNED_INT, 0);
        checkGraphicsError();

        glDisableVertexAttribArray(position);
        checkGraphicsError();

        if (colorID != -1) {
            glDisableVertexAttribArray(colorID);
            checkGraphicsError();
        }

        if (texCoordID != -1) {
            glDisableVertexAttribArray(texCoordID);
            checkGraphicsError();
        }

        if (normCoordID != -1) {
            glDisableVertexAttribArray(normCoordID);
            checkGraphicsError();
        }
    }
}