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
#ifndef AMAZING_LABYRINTH_TEXTURE_LOADER_HPP
#define AMAZING_LABYRINTH_TEXTURE_LOADER_HPP
#include <set>
#include <vector>
#include <map>
#include <string>
#include "../common.hpp"

class GameRequester;

namespace levelDrawer {
    class TextureData {
    public:
        virtual ~TextureData() = default;
    };

    class TextureDescription {
        friend BaseClassPtrLess<TextureDescription>;
    protected:
        virtual bool compareLess(TextureDescription *) = 0;

    public:
        virtual std::vector<char> getData(std::shared_ptr<GameRequester> const &gameRequester,
                                          uint32_t &texWidth, uint32_t &texHeight,
                                          uint32_t &texChannels) = 0;

        virtual ~TextureDescription() = default;
    };

    class TextureDescriptionPath : public TextureDescription {
    private:
        std::string imagePath;
    protected:
        bool compareLess(TextureDescription *other) override {
            auto otherPath = dynamic_cast<TextureDescriptionPath *>(other);
            return imagePath < otherPath->imagePath;
        }

    public:
        TextureDescriptionPath(std::string const &inImagePath)
                : TextureDescription{},
                  imagePath{inImagePath} {}

        std::vector<char> getData(std::shared_ptr<GameRequester> const &gameRequester,
                                  uint32_t &texWidth, uint32_t &texHeight,
                                  uint32_t &texChannels) override;
    };

    class TextureDescriptionText : public TextureDescription {
    private:
        std::string m_textString;
    protected:
        bool compareLess(TextureDescription *other) override {
            auto otherPath = dynamic_cast<TextureDescriptionText *>(other);
            return m_textString < otherPath->m_textString;
        }

    public:
        TextureDescriptionText(std::string const &inTextString)
                : TextureDescription{},
                  m_textString{inTextString} {
        }

        std::vector<char> getData(std::shared_ptr<GameRequester> const &gameRequester,
                                  uint32_t &texWidth, uint32_t &texHeight,
                                  uint32_t &texChannels) override;
    };
}

#endif // AMAZING_LABYRINTH_TEXTURE_LOADER_HPP