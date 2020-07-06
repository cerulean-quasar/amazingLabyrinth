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

#include "../../common.hpp"
#include "textureLoader.hpp"

namespace levelDrawer {
    int istreamRead(void *userData, char *data, int size) {
        std::istream *stream = static_cast<std::istream *>(userData);
        return stream->read(data, size).gcount();
    }

    void istreamSkip(void *userData, int n) {
        std::istream *stream = static_cast<std::istream *>(userData);
        stream->seekg(n, stream->cur);
    }

    int istreamEof(void *userData) {
        std::istream *stream = static_cast<std::istream *>(userData);
        return stream->eof();
    }

    std::vector<char>
    TextureDescriptionPath::getData(std::shared_ptr<GameRequester> const &gameRequester,
                                    uint32_t &texWidth, uint32_t &texHeight,
                                    uint32_t &texChannels) {
        std::unique_ptr<std::streambuf> assetbuf = gameRequester->getAssetStream(imagePath);
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

        std::vector<char> data{};
        unsigned int size = texWidth * texHeight * texChannels;
        data.resize(size);
        memcpy(data.data(), pixels, size);

        /* free the CPU memory for the image */
        stbi_image_free(pixels);

        return data;
    }

    std::vector<char>
    TextureDescriptionText::getData(std::shared_ptr<GameRequester> const &gameRequester,
                                    uint32_t &texWidth, uint32_t &texHeight,
                                    uint32_t &texChannels) {
        return gameRequester->getTextImage(m_textString, texWidth, texHeight, texChannels);
    }
}