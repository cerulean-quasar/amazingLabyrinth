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
#ifndef AMAZING_LABYRINTH_TEXTURE_TABLE_HPP
#define AMAZING_LABYRINTH_TEXTURE_TABLE_HPP

#include <vector>
#include <map>
#include <memory>

#include "../../common.hpp"
#include "../common.hpp"
#include "textureLoader.hpp"

class TextureTable {
public:
    std::shared_ptr<TextureData> const &getTextureData(size_t index) {
        return m_textureData[index];
    }

    size_t addTexture(std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<TextureDescription> const &textureDescription)
    {
        auto item = m_textureIndexMap.emplace(textureDescription, 0);
        if (item.second) {
            return item.first->second;
        } else {
            m_textureDataVector.push_back(getTextureData(gameRequester, textureDescription));
            item.first->second = m_textureDataVector.size() - 1;
        }
    }

    TextureTable()
            : m_textureIndexMap{},
              m_textureData{}
    {}

    virtual ~TextureTable() = default;
protected:
    virtual std::shared_ptr<TextureData> getTextureData(std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<TextureDescription> const &textureDescription) = 0;

    std::map<std::shared_ptr<TextureDescription>, size_t, BaseClassPtrLess<TextureDescription>> m_textureIndexMap;
    std::vector<std::shared_ptr<TextureData>> m_textureDataVector;
};
#endif // AMAZING_LABYRINTH_TEXTURE_TABLE_HPP
