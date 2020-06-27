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
#ifndef AMAZING_LABYRINTH_DRAW_OBJECT_GL_HPP
#define AMAZING_LABYRINTH_DRAW_OBJECT_GL_HPP

#include <momory>
#include <glm/glm.h>
#include "drawObject.hpp"

class DrawObjectDataGL : public DrawObjectData {
public:
    void update(glm::mat4 const &modelMatrix) override {
        m_modelMatrix = modelMatrix;
    }

    DrawObjectDataGL(glm::mat4 const &inModelMatrix)
        : m_modelMatrix{inModelMatrix}
    {}

    ~DrawObjectDataGL() override = default;
private:
    glm::mat4 m_modelMatrix;
};

#endif // AMAZING_LABYRINTH_DRAW_OBJECT_GL_HPP