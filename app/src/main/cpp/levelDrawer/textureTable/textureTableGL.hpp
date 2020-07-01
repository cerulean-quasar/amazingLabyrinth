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
#ifndef AMAZING_LABYRINTH_TEXTURE_TABLE_GL_HPP
#define AMAZING_LABYRINTH_TEXTURE_TABLE_GL_HPP

#include <map>
#include <memory>

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include "textureTable.hpp"

class TextureDataGL : public TextureData {
public:
    TextureDataGL(std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<TextureDescription> const &textureDescription)
    {
        createTexture(gameRequester, textureDescription);
    }

    ~TextureDataGL() override {
        glDeleteTextures(1, &m_handle);
    }

    inline GLuint handle() const { return m_handle; }
private:
    GLuint m_handle;

    void createTexture(std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<TextureDescription> const &textureDescription);
};

class TextureTableGL : public TextureTableGeneric<TextureDataGL> {
public:
    ~TextureTableGL() override = default;
protected:
    std::shared_ptr<TextureDataGL> getTextureData(std::shared_ptr<GameRequester> const &gameRequester,
                                                  std::shared_ptr<TextureDescription> const &textureDescription) override
    {
        return std::make_shared<TextureDataGL>(gameRequester, textureDescription);
    }

};

#endif // AMAZING_LABYRINTH_TEXTURE_TABLE_GL_HPP
