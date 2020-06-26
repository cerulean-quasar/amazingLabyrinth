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

class TextureDescription {
    friend BaseClassPtrLess<TextureDescription>;
protected:
    std::shared_ptr<GameRequester> m_gameRequester;

    virtual bool compareLess(TextureDescription *) = 0;
public:
    explicit TextureDescription(std::shared_ptr<GameRequester> inGameRequester)
        : m_gameRequester{std::move(inGameRequester)}
    {}
    virtual std::vector<char> getData(uint32_t &texWidth, uint32_t &texHeight, uint32_t &texChannels) = 0;
    virtual ~TextureDescription() = default;
};

// TODO: can remove, testing
/*
class TextureDescriptionDummy : public TextureDescription {
private:
protected:
    virtual bool compareLess(TextureDescription *other) {
        return false;
    }
public:
    TextureDescriptionDummy(std::shared_ptr<GameRequester> inGameRequester)
        : TextureDescription(std::move(inGameRequester))
        {}
    virtual std::vector<char> getData(uint32_t &texWidth, uint32_t &texHeight, uint32_t &texChannels) {
        return std::vector<char>();
    }
};
 */

class TextureDescriptionPath : public TextureDescription {
private:
    std::string imagePath;
protected:
    virtual bool compareLess(TextureDescription *other) {
        auto otherPath = dynamic_cast<TextureDescriptionPath*>(other);
        return imagePath < otherPath->imagePath;
    }
public:
    TextureDescriptionPath(
            std::shared_ptr<GameRequester> inGameRequester,
            std::string const &inImagePath)
        : TextureDescription{std::move(inGameRequester)},
          imagePath{inImagePath}
    {}
    std::vector<char> getData(uint32_t &texWidth, uint32_t &texHeight, uint32_t &texChannels) override;
};

class TextureDescriptionText : public TextureDescription {
private:
    std::string m_textString;
protected:
    bool compareLess(TextureDescription *other) {
        auto otherPath = dynamic_cast<TextureDescriptionText*>(other);
        return m_textString < otherPath->m_textString;
    }
public:
    TextureDescriptionText(
            std::shared_ptr<GameRequester> inGameRequester,
            std::string const &inTextString)
        : TextureDescription{std::move(inGameRequester)},
          m_textString{inTextString}
    {
    }
    std::vector<char> getData(uint32_t &texWidth, uint32_t &texHeight, uint32_t &texChannels) override;
};


int istreamRead(void *userData, char *data, int size);
void istreamSkip(void *userData, int n);
int istreamEof(void *userData);
#endif // AMAZING_LABYRINTH_TEXTURE_LOADER_HPP