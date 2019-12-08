/**
 * Copyright 2019 Cerulean Quasar. All Rights Reserved.
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
#ifndef AMAZING_LABYRINTH_GRAPHICS_HPP
#define AMAZING_LABYRINTH_GRAPHICS_HPP
#include <set>
#include <vector>
#include <map>
#include <string>

#include <glm/glm.hpp>

class GameRequester;

constexpr float screenMaxX = 0.8f;
constexpr float screenMaxY = 1.0f;

static std::string const MODEL_WALL("models/wall.obj");
static std::string const MODEL_BALL("models/ball.obj");

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
    glm::vec3 normal;

    bool operator==(const Vertex& other) const;
};

namespace std {
    template<> struct hash<glm::vec3> {
        size_t operator()(glm::vec3 vector) const {
            return ((hash<float>()(vector.x) ^
                     (hash<float>()(vector.y) << 1)) >> 1) ^
                   (hash<float>()(vector.z) << 1);
        }
    };

    template<> struct hash<glm::vec2> {
        size_t operator()(glm::vec2 vector) const {
            return (hash<float>()(vector.x) ^ (hash<float>()(vector.y) << 1));
        }
    };

    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                     (hash<glm::vec2>()(vertex.texCoord) << 1)) >> 1) ^ (hash<glm::vec3>()(vertex.normal) << 1);
        }
    };
}

class TextureDescriptionPtrLess;

class TextureDescription {
    friend TextureDescriptionPtrLess;
protected:
    std::shared_ptr<GameRequester> m_gameRequester;

    virtual bool compare(TextureDescription *) = 0;
public:
    explicit TextureDescription(std::shared_ptr<GameRequester> inGameRequester)
        : m_gameRequester{std::move(inGameRequester)}
    {}
    virtual std::vector<char> getData(uint32_t &texWidth, uint32_t &texHeight, uint32_t &texChannels) = 0;
    virtual ~TextureDescription() = default;
};

// TODO: can remove, testing
class TextureDescriptionDummy : public TextureDescription {
private:
protected:
    virtual bool compare(TextureDescription *other) {
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

class TextureDescriptionPath : public TextureDescription {
private:
    std::string imagePath;
protected:
    virtual bool compare(TextureDescription *other) {
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
    virtual std::vector<char> getData(uint32_t &texWidth, uint32_t &texHeight, uint32_t &texChannels);
};

class TextureDescriptionText : public TextureDescription {
private:
    std::string m_textString;
protected:
    bool compare(TextureDescription *other) {
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
    virtual std::vector<char> getData(uint32_t &texWidth, uint32_t &texHeight, uint32_t &texChannels);
};

class TextureData {
public:
    virtual ~TextureData() = default;
};

class TextureDescriptionPtrLess {
public:
    bool operator() (std::shared_ptr<TextureDescription> const &p1,
                     std::shared_ptr<TextureDescription> const &p2) const {
        TextureDescription &p1ref = *p1;
        TextureDescription &p2ref = *p2;
        std::type_info const &c1 = typeid(p1ref);
        std::type_info const &c2 = typeid(p2ref);
        if (c1 != c2) {
            return c1.before(c2);
        } else if (p1.get() == p2.get()) {
            return false;
        } else {
            return p1->compare(p2.get());
        }
    }
};

typedef std::map<std::shared_ptr<TextureDescription>, std::shared_ptr<TextureData>,
                 TextureDescriptionPtrLess > TextureMap;

struct DrawObject {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<glm::mat4> modelMatrices;
    std::shared_ptr<TextureDescription> texture;
};

struct DrawObjectData {
    virtual ~DrawObjectData() = default;
};

typedef std::pair<std::shared_ptr<DrawObject>, std::shared_ptr<DrawObjectData> > DrawObjectEntry;
typedef std::vector<DrawObjectEntry> DrawObjectTable;

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

// creates a quad with each side length 2.0f.
void getQuad(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);

void loadModel(std::unique_ptr<std::streambuf> const &modelStreamBuf, std::vector<Vertex> &vertices,
        std::vector<uint32_t> &indices);

int istreamRead(void *userData, char *data, int size);
void istreamSkip(void *userData, int n);
int istreamEof(void *userData);
#endif