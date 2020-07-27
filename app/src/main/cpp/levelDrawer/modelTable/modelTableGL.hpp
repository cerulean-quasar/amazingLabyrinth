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
            std::pair<ModelVertices, ModelVertices> vertices;
            if (modelDescription->shouldLoadVertexNormals()) {
                vertices = modelDescription->getDataWithVertexNormalsAlso(gameRequester);
            } else {
                vertices.first = modelDescription->getData(gameRequester);
            }
            m_numberIndices = vertices.first.second.size();

            // the index buffer
            glGenBuffers(1, &(m_indexBuffer));
            checkGraphicsError();
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
            checkGraphicsError();
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * vertices.first.second.size(),
                         vertices.first.second.data(), GL_STATIC_DRAW);
            checkGraphicsError();

            // the vertex buffer
            glGenBuffers(1, &(m_vertexBuffer));
            checkGraphicsError();
            glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
            checkGraphicsError();
            glBufferData(GL_ARRAY_BUFFER, sizeof (Vertex) * vertices.first.first.size(),
                         vertices.first.first.data(), GL_STATIC_DRAW);
            checkGraphicsError();

            // the vertex normals (if requested)
            m_numberIndicesWithVertexNormals = vertices.second.second.size();
            if (m_numberIndicesWithVertexNormals > 0) {
                // the index buffer
                glGenBuffers(1, &(m_indexBufferWithVertexNormals));
                checkGraphicsError();
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferWithVertexNormals);
                checkGraphicsError();
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (uint32_t) * vertices.second.second.size(),
                             vertices.second.second.data(), GL_STATIC_DRAW);
                checkGraphicsError();

                // the vertex buffer
                glGenBuffers(1, &(m_vertexBufferWithVertexNormals));
                checkGraphicsError();
                glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferWithVertexNormals);
                checkGraphicsError();
                glBufferData(GL_ARRAY_BUFFER, sizeof (Vertex) * vertices.second.first.size(),
                             vertices.second.first.data(), GL_STATIC_DRAW);
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
