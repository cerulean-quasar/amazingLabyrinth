#include <string>
#include <istream>
#include "graphics.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

std::string const TEXTURE_PATH_WALLS("textures/wall.png");
std::string const TEXTURE_PATH_FLOOR("textures/floor.png");
std::string const TEXTURE_PATH_BALL("textures/ball.png");
std::string const TEXTURE_PATH_HOLE("textures/hole.png");
unsigned int const MAZE_COLS = 5;
unsigned int const MAZE_ROWS = 5;

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

