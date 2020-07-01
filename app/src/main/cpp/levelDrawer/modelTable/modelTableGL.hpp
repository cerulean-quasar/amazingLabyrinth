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

class ModelDataGL : public ModelData {
public:
    ModelDataGL(std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<ModelDescription> const &modelDescription)
    {
        ModelVertices vertices = modelDescription->getData(gameRequester);

        // the index buffer
        glGenBuffers(1, &(m_indexBuffer));
        checkGraphicsError();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
        checkGraphicsError();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (uint32_t) * vertices.second.size(),
                     vertices.second.data(), GL_STATIC_DRAW);
        checkGraphicsError();

        // the vertex buffer
        glGenBuffers(1, &(m_vertexBuffer));
        checkGraphicsError();
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        checkGraphicsError();
        glBufferData(GL_ARRAY_BUFFER, sizeof (Vertex) * vertices.first.size(),
                     vertices.first.data(), GL_STATIC_DRAW);
        checkGraphicsError();
    }

    ~ModelDataGL() override {
        glDeleteBuffers(1, &m_vertexBuffer);
        glDeleteBuffers(1, &m_indexBuffer);
    }

    inline GLuint vertexBuffer() const { return m_vertexBuffer; }
    inline GLuint indexBuffer() const { return m_indexBuffer; }

private:
    void DrawObjectDataGL::createDrawObjectData(std::shared_ptr<DrawObject> const &drawObj) {
    }

    GLuint m_vertexBuffer;
    GLuint m_indexBuffer;
};

class ModelTableGL : public ModelTableGeneric<ModelDataGL> {
public:
    ~ModelTableGL() override = default;
protected:
    std::shared_ptr<ModelData> getModelData(std::shared_ptr<GameRequester> const &gameRequester,
                                            std::shared_ptr<ModelDescription> const &modelDescription) override
    {
        return std::make_shared<ModelDataGL>(gameRequester,  modelDescription);
    }
};

#endif // AMAZING_LABYRINTH_MODEL_TABLE_GL_HPP
