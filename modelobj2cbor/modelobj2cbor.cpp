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

#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <iomanip>

#include <glm/glm.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <modelLoader.hpp>

void loadModelFromObj(
    std::ifstream &modelStream,
    std::vector<float> &vertices,
    std::vector<float> &faceNormals,
    std::vector<float> &vertexNormals,
    std::vector<float> &texcoords,
    std::vector<float> &,
    std::vector<std::vector<uint32_t>> &indices)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, &modelStream)) {
        throw std::runtime_error(err);
    }

    vertices = std::move(attrib.vertices);
    faceNormals = std::move(attrib.normals);
    texcoords = std::move(attrib.texcoords);

    for (auto const &shape : shapes) {
        /* Find the vertex normals */
        std::unordered_map<glm::vec3, glm::vec3> vertexNormalMap(shape.mesh.indices.size());
        for (auto const & index : shape.mesh.indices) {
            glm::vec3 pos = {
                    vertices[3*index.vertex_index + 0],
                    vertices[3*index.vertex_index + 1],
                    vertices[3*index.vertex_index + 2]
            };

            glm::vec3 normal = {
                    faceNormals[3*index.normal_index + 0],
                    faceNormals[3*index.normal_index + 1],
                    faceNormals[3*index.normal_index + 2]
            };

            auto it = vertexNormalMap.emplace(pos, normal);
            if (!it.second) {
                it.first->second += normal;
            }
        }

        /* build the indices */
        std::vector<uint32_t> ind;
        std::unordered_map<glm::vec3, uint32_t> uniqueVertexNormals;
        for (auto const &index : shape.mesh.indices) {
            ind.push_back(index.vertex_index);
            ind.push_back(index.normal_index);

            glm::vec3 pos = {
                    vertices[3*index.vertex_index + 0],
                    vertices[3*index.vertex_index + 1],
                    vertices[3*index.vertex_index + 2]};
            auto it = vertexNormalMap.find(pos);
            if (it == vertexNormalMap.end()) {
                throw std::runtime_error("Vertex normal not found when loading model");
            }

            auto itPair = uniqueVertexNormals.emplace(glm::normalize(it->second), vertexNormals.size() / 3);
            if (itPair.second) {
                vertexNormals.push_back(itPair.first->first.x);
                vertexNormals.push_back(itPair.first->first.y);
                vertexNormals.push_back(itPair.first->first.z);
            }

            ind.push_back(itPair.first->second);

            if (!texcoords.empty()) {
                ind.push_back(index.texcoord_index);
            }
        }
        indices.push_back(std::move(ind));
    }
}
