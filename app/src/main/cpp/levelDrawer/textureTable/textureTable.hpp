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

namespace levelDrawer {
    template<typename TextureDataType>
    class TextureTable {
    public:
        std::shared_ptr<TextureDataType>
        addTexture(std::shared_ptr<GameRequester> const &gameRequester,
                   std::shared_ptr<TextureDescription> const &textureDescription) override {
            auto item = m_textureIndexMap.emplace(textureDescription, 0);
            if (!item.second) {
                item.first->second = getTextureData(gameRequester, textureDescription);
            }
            return item.first->second;
        }

        TextureTable() = default;

        virtual ~TextureTable() = default;

    protected:
        virtual std::shared_ptr<TextureDataType>
        getTextureData(std::shared_ptr<GameRequester> const &gameRequester,
                       std::shared_ptr<TextureDescription> const &textureDescription) = 0;

        std::map<std::shared_ptr<TextureDescription>, std::shared_ptr<TextureDataType>, BaseClassPtrLess<TextureDescription>> m_textureIndexMap;
    };
}
#endif // AMAZING_LABYRINTH_TEXTURE_TABLE_HPP
