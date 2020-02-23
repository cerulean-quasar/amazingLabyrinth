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

#define GLM_FORCE_RADIANS
#include <glm/gtx/transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "common.hpp"
#include "graphics.hpp"


bool Vertex::operator==(const Vertex& other) const {
    return pos == other.pos && color == other.color && texCoord == other.texCoord && normal == other.normal;
}

// creates a quad with each side length 2.0f.
void getQuad(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices) {
    Vertex vertex = {};
    vertex.color = {0.2f, 0.2f, 0.2f };
    vertex.normal = {0.0f, 0.0f, 1.0f };

    vertex.pos = { -1.0f, 1.0f, 0.0f };
    vertex.texCoord = {0.0f, 0.0f };
    vertices.push_back(vertex);

    vertex.pos = { -1.0f, -1.0f, 0.0f };
    vertex.texCoord = {0.0f, 1.0f };
    vertices.push_back(vertex);

    vertex.pos = { 1.0f, -1.0f, 0.0f };
    vertex.texCoord = {1.0f, 1.0f };
    vertices.push_back(vertex);

    vertex.pos = { 1.0f, 1.0f, 0.0f };
    vertex.texCoord = {1.0f, 0.0f };
    vertices.push_back(vertex);

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);

    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(3);

}

void loadModel(
        std::unique_ptr<std::streambuf> const &modelStreamBuf,
        std::vector<Vertex> &vertices,
        std::vector<uint32_t> &indices) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    std::istream assetIstream(modelStreamBuf.get());

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, &assetIstream)) {
        throw std::runtime_error(err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices = {};
    for (const auto& shape : shapes) {
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

            vertex.normal = { attrib.normals[3 * index.normal_index +0],
                              attrib.normals[3 * index.normal_index +1],
                              attrib.normals[3 * index.normal_index +2] };

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
}

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
    texChannels = static_cast<uint32_t> (c);

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
