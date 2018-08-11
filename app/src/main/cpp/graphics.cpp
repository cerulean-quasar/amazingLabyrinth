#include <string>
#include <istream>
#include <array>
#include <unordered_map>
#include "graphics.hpp"
#include "vulkanWrapper.hpp"
#include "native-lib.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

unsigned int const MAZE_COLS = 5;
unsigned int const MAZE_ROWS = 5;


VkVertexInputBindingDescription Vertex::getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription = {};

    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);

    /* move to the next data entry after each vertex.  VK_VERTEX_INPUT_RATE_INSTANCE
     * moves to the next data entry after each instance, but we are not using instanced
     * rendering
     */
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 4> Vertex::getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};

    /* position */
    attributeDescriptions[0].binding = 0; /* binding description to use */
    attributeDescriptions[0].location = 0; /* matches the location in the vertex shader */
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    /* color */
    attributeDescriptions[1].binding = 0; /* binding description to use */
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    /* texture coordinate */
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    /* normal vector */
    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex, normal);
    return attributeDescriptions;
}

bool Vertex::operator==(const Vertex& other) const {
    return pos == other.pos && color == other.color && texCoord == other.texCoord && normal == other.normal;
}

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

void loadModel(std::string const & modelFile, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    AssetStreambuf assetStreambuf(assetWrapper->getAsset(modelFile));
    std::istream assetIstream(&assetStreambuf);

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

            vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

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
    AssetStreambuf imageStreambuf(assetWrapper->getAsset(imagePath));
    std::istream imageStream(&imageStreambuf);

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
    return getTextImage(textString, texWidth, texHeight, texChannels);
}