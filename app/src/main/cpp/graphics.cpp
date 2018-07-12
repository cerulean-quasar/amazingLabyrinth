#include <string>
#include <istream>
#include "graphics.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

unsigned int const MAZE_COLS = 5;
unsigned int const MAZE_ROWS = 5;

void getQuad(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices) {
    Vertex vertex = {};
    vertex.color = {0.2f, 0.2f, 0.2f };
    vertex.normal = {0.0f, 0.0f, 1.0f };

    vertex.pos = { -1.0f, 1.0f, 0.0f };
    vertex.texCoord = {0.0f, 0.0f };
    vertices.push_back(vertex);

    vertex.pos = { -1.0f, -1.0f, 0.0f };
    vertex.texCoord = {1.0f, 0.0f };
    vertices.push_back(vertex);

    vertex.pos = { 1.0f, -1.0f, 0.0f };
    vertex.texCoord = {1.0f, 1.0f };
    vertices.push_back(vertex);

    vertex.pos = { 1.0f, 1.0f, 0.0f };
    vertex.texCoord = {0.0f, 1.0f };
    vertices.push_back(vertex);

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);

    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(3);

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

