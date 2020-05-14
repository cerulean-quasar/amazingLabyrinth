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

#include <vector>
#include <map>
#include <string>

#include <glm/glm.hpp>

char constexpr const *KeyVertices = "V";
char constexpr const *KeyTexCoords = "TX";
char constexpr const *KeyNormals = "N";
char constexpr const *KeyIndices = "I";

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
    glm::vec3 normal;

    bool operator==(const Vertex& other) const;
};

// creates a quad with each side length 2.0f.
void getQuad(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);

bool loadModel(
        std::unique_ptr<std::streambuf> const &modelStreamBuf,
        std::pair<std::vector<Vertex>, std::vector<uint32_t>> &verticesWithFaceNormals,
        std::pair<std::vector<Vertex>, std::vector<uint32_t>> *verticesWithVertexNormals = nullptr);

