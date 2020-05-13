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

#include <string>
#include <istream>
#include <array>
#include <unordered_map>
#include <list>

#include <glm/glm.hpp>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#pragma clang diagnostic pop

#include "common.hpp"
#include "graphics.hpp"


#if 0
void loadModel(
        std::unique_ptr<std::streambuf> const &modelStreamBuf,
        std::vector<Vertex> &vertices,
        std::vector<uint32_t> &indices,
        std::pair<std::vector<Vertex>, std::vector<uint32_t>> *verticesWithVertexNormals)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    std::istream assetIstream(modelStreamBuf.get());

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, &assetIstream)) {
        throw std::runtime_error(err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices = {};
    std::unordered_map<Vertex, uint32_t> uniqueVerticesWithVertexNormals = {};
    for (const auto& shape : shapes) {
        std::unordered_map<glm::vec3, glm::vec3> vertexNormals;
        if (verticesWithVertexNormals) {
            for (const auto &index : shape.mesh.indices) {
                glm::vec3 pos = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                };

                glm::vec3 normal = {attrib.normals[3 * index.normal_index + 0],
                                    attrib.normals[3 * index.normal_index + 1],
                                    attrib.normals[3 * index.normal_index + 2]};

                auto it = vertexNormals.find(pos);
                if (it == vertexNormals.end()) {
                    vertexNormals.insert(std::make_pair(pos, normal));
                } else {
                    it->second += normal;
                }
            }

            for (auto &item: vertexNormals) {
                item.second = glm::normalize(item.second);
            }
        }

        for (const auto& index : shape.mesh.indices) {
            Vertex vertex = {};
            vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
            };

            if (attrib.texcoords.size() > 0) {
                vertex.texCoord = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
            } else {
                vertex.texCoord = {0.0f, 0.0f};
            }

            vertex.color = {0.2f, 0.2f, 0.2f};

            if (verticesWithVertexNormals) {
                Vertex vertexWithVertexNormal = vertex;
                auto it = vertexNormals.find(vertexWithVertexNormal.pos);
                if (it == vertexNormals.end()) {
                    throw std::runtime_error("Vertex normal not found when loading model");
                }
                vertexWithVertexNormal.normal = it->second;
                if (uniqueVerticesWithVertexNormals.count(vertexWithVertexNormal) == 0) {
                    uniqueVerticesWithVertexNormals[vertexWithVertexNormal] = static_cast<uint32_t>(verticesWithVertexNormals->first.size());
                    verticesWithVertexNormals->first.push_back(vertexWithVertexNormal);
                }

                verticesWithVertexNormals->second.push_back(uniqueVerticesWithVertexNormals[vertexWithVertexNormal]);
            }

            vertex.normal = {attrib.normals[3 * index.normal_index + 0],
                                attrib.normals[3 * index.normal_index + 1],
                                attrib.normals[3 * index.normal_index + 2]};

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
}
#endif

int istreamRead(void *userData, char *data, int size) {
    std::istream *stream = static_cast<std::istream*>(userData);
    return stream->read(data,size).gcount();
}

void istreamSkip(void *userData, int n) {
    std::istream *stream = static_cast<std::istream*>(userData);
    stream->seekg(n, stream->cur);
}

int istreamEof(void *userData) {
    std::istream *stream = static_cast<std::istream*>(userData);
    return stream->eof();
}

std::vector<char> TextureDescriptionPath::getData(uint32_t &texWidth, uint32_t &texHeight,
                                                  uint32_t &texChannels) {
    std::unique_ptr<std::streambuf> assetbuf = m_gameRequester->getAssetStream(imagePath);
    std::istream imageStream(assetbuf.get());

    int c, w, h;

    stbi_io_callbacks clbk;
    clbk.read = istreamRead;
    clbk.skip = istreamSkip;
    clbk.eof = istreamEof;

    stbi_uc *pixels = stbi_load_from_callbacks(&clbk, &imageStream, &w, &h, &c, STBI_rgb_alpha);
    texWidth = static_cast<uint32_t> (w);
    texHeight = static_cast<uint32_t> (h);
    texChannels = 4;

    std::vector<char> data;
    unsigned int size = texWidth*texHeight*texChannels;
    data.resize(size);
    memcpy(data.data(), pixels, size);

    /* free the CPU memory for the image */
    stbi_image_free(pixels);

    return data;
}

std::vector<char> TextureDescriptionText::getData(uint32_t &texWidth, uint32_t &texHeight,
                                                  uint32_t &texChannels) {
    return m_gameRequester->getTextImage(m_textString, texWidth, texHeight, texChannels);
}

std::vector<char> readFile(std::shared_ptr<FileRequester> const &requester, std::string const &filename) {
    std::unique_ptr<std::streambuf> assetStreambuf = requester->getAssetStream(filename);
    std::istream reader(assetStreambuf.get());
    std::vector<char> data;
    unsigned long const readSize = 1024;

    while (!reader.eof()) {
        unsigned long size = data.size();
        data.resize(size + readSize);
        long bytesRead = reader.read(data.data()+size, readSize).gcount();

        if (bytesRead != readSize) {
            data.resize(size + bytesRead);
        }
    }

    return data;
}
