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

#include "textureTableGL.hpp"

void TextureDataGL::createTexture(std::shared_ptr<TextureDescription> const &textureDescription) {

    glGenTextures(1, &m_handle);
    checkGraphicsError();
    glBindTexture(GL_TEXTURE_2D, m_handle);
    checkGraphicsError();

    // when sampling outside of the texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    checkGraphicsError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    checkGraphicsError();

    // when the texture is scaled up or down
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, /*GL_LINEAR_MIPMAP_LINEAR*/
                    GL_NEAREST);
    checkGraphicsError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, /*GL_LINEAR*/ GL_NEAREST);
    checkGraphicsError();

    uint32_t texHeight{};
    uint32_t texWidth{};
    uint32_t texChannels{};
    std::vector<char> pixels = textureDescription->getData(texWidth, texHeight, texChannels);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixels.data());
    checkGraphicsError();

    glGenerateMipmap(GL_TEXTURE_2D);
    checkGraphicsError();
    glBindTexture(GL_TEXTURE_2D, 0);
    checkGraphicsError();
}
