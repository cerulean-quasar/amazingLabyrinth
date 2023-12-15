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
#ifndef AMAZING_LABYRINTH_MODEL_TABLE_GL_HPP
#define AMAZING_LABYRINTH_MODEL_TABLE_GL_HPP

#include <vector>
#include <map>
#include <memory>

#include <GLES3/gl3.h>

#include "../../common.hpp"
#include "modelLoader.hpp"
#include "modelTable.hpp"

namespace levelDrawer {
    class ModelDataGL {
    public:
        ModelDataGL(std::shared_ptr <GameRequester> const &gameRequester,
                    std::shared_ptr <ModelDescription> const &modelDescription) {
            std::pair<ModelVertices, ModelVertices> vertices = modelDescription->getData(gameRequester);

            ModelVertices *firstVerticesToLoad = nullptr;
            ModelVertices *secondVerticesToLoad = nullptr;
            switch (modelDescription->normalsToLoad()) {
                case 0:
                    throw std::runtime_error("ModelDescription is not loading any vertices.");
                case levelDrawer::ModelDescription::LOAD_BOTH:
                    secondVerticesToLoad = &vertices.second;
                    /* continue on */
                case levelDrawer::ModelDescription::LOAD_FACE_NORMALS:
                    firstVerticesToLoad = &vertices.first;
                    break;
                case levelDrawer::ModelDescription::LOAD_VERTEX_NORMALS:
                    firstVerticesToLoad = &vertices.second;
                    break;
            }

            m_numberIndices = firstVerticesToLoad->second.size();

            /* If either the vertex normals or the face normals (not both) were requested, then these
             * would be the one that was requested.  If both were requested, then this would be the
             * face normals and the vertex normals would be loaded down below.
             */
            // the index buffer
            glGenBuffers(1, &(m_indexBuffer));
            checkGraphicsError();
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
            checkGraphicsError();
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (uint32_t) * firstVerticesToLoad->second.size(),
                         firstVerticesToLoad->second.data(), GL_STATIC_DRAW);
            checkGraphicsError();

            // the vertex buffer
            glGenBuffers(1, &(m_vertexBuffer));
            checkGraphicsError();
            glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
            checkGraphicsError();
            glBufferData(GL_ARRAY_BUFFER, sizeof (Vertex) * firstVerticesToLoad->first.size(),
                         firstVerticesToLoad->first.data(), GL_STATIC_DRAW);
            checkGraphicsError();

            // If both the vertex normals and the face normals were requested, then these would be
            // the vertex normals.
            if (secondVerticesToLoad) {
                m_numberIndicesWithVertexNormals = secondVerticesToLoad->second.size();

                // the index buffer
                glGenBuffers(1, &(m_indexBufferWithVertexNormals));
                checkGraphicsError();
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferWithVertexNormals);
                checkGraphicsError();
                glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                             sizeof (uint32_t) * secondVerticesToLoad->second.size(),
                             secondVerticesToLoad->second.data(), GL_STATIC_DRAW);
                checkGraphicsError();

                // the vertex buffer
                glGenBuffers(1, &(m_vertexBufferWithVertexNormals));
                checkGraphicsError();
                glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferWithVertexNormals);
                checkGraphicsError();
                glBufferData(GL_ARRAY_BUFFER, sizeof (Vertex) * secondVerticesToLoad->first.size(),
                             secondVerticesToLoad->first.data(), GL_STATIC_DRAW);
                checkGraphicsError();
            }
        }

        ~ModelDataGL() {
            glDeleteBuffers(1, &m_vertexBuffer);
            glDeleteBuffers(1, &m_indexBuffer);

            if (m_numberIndicesWithVertexNormals > 0) {
                glDeleteBuffers(1, &m_vertexBufferWithVertexNormals);
                glDeleteBuffers(1, &m_indexBufferWithVertexNormals);
            }
        }

        inline GLuint vertexBuffer() const { return m_vertexBuffer; }

        inline GLuint indexBuffer() const { return m_indexBuffer; }

        inline uint32_t numberIndices() const { return m_numberIndices; }

        inline GLuint vertexBufferWithVertexNormals() const {
            if (m_numberIndicesWithVertexNormals == 0) {
                throw std::runtime_error("Vertex normals not requested at model creation, but requested at model usage.");
            }
            return m_vertexBufferWithVertexNormals;
        }

        inline GLuint indexBufferWithVertexNormals() const {
            if (m_numberIndicesWithVertexNormals == 0) {
                throw std::runtime_error("Vertex normals not requested at model creation, but requested at model usage.");
            }
            return m_indexBufferWithVertexNormals;
        }

        inline uint32_t numberIndicesWithVertexNormals() const {
            return m_numberIndicesWithVertexNormals;
        }
    private:
        GLuint m_vertexBuffer;
        GLuint m_indexBuffer;
        uint32_t m_numberIndices;

        GLuint m_vertexBufferWithVertexNormals;
        GLuint m_indexBufferWithVertexNormals;
        uint32_t m_numberIndicesWithVertexNormals;
    };

    class ModelTableGL : public ModelTable<ModelDataGL> {
    public:
        ~ModelTableGL() override = default;

    protected:
        std::shared_ptr <ModelDataGL>
        getModelData(std::shared_ptr <GameRequester> const &gameRequester,
                     std::shared_ptr <ModelDescription> const &modelDescription) override {
            return std::make_shared<ModelDataGL>(gameRequester, modelDescription);
        }
    };
}

#endif // AMAZING_LABYRINTH_MODEL_TABLE_GL_HPP
