/**
 * Copyright 2023 Cerulean Quasar. All Rights Reserved.
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
#ifndef AMAZING_LABYRINTH_MODEL_LOADER_HPP
#define AMAZING_LABYRINTH_MODEL_LOADER_HPP
#include <vector>
#include <map>
#include <string>

#include <glm/glm.hpp>
#include "../common.hpp"

namespace levelDrawer {
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;
        glm::vec3 normal;

        bool operator==(const Vertex &other) const;
        Vertex(glm::vec3 inPos,
               glm::vec3 inColor,
               glm::vec2 inTexCoord,
               glm::vec3 inNormal)
                : pos(inPos),
                  color(inColor),
                  texCoord(inTexCoord),
                  normal(inNormal) {}
        Vertex() {
            pos = {0.0f, 0.0f, 0.0f};
            color = { 0.0f, 0.0f, 0.0f};
            texCoord = {0.0f, 0.0f};
            normal = {0.0f, 0.0f, 0.0f};
        }
    };
} // namespace levelDrawer

namespace std {
    template<>
    struct hash<glm::vec4> {
        size_t operator()(glm::vec4 const &vector) const {
            return ((((hash<float>()(vector.r) ^
                       (hash<float>()(vector.g) << 1)) >> 1) ^
                     (hash<float>()(vector.b) << 1)) >> 1) ^
                   (hash<float>()(vector.a) << 1);
        }
    };

    template<>
    struct hash<glm::vec3> {
        size_t operator()(glm::vec3 const &vector) const {
            return ((hash<float>()(vector.x) ^
                     (hash<float>()(vector.y) << 1)) >> 1) ^
                   (hash<float>()(vector.z) << 1);
        }
    };

    template<>
    struct hash<glm::vec2> {
        size_t operator()(glm::vec2 const &vector) const {
            return (hash<float>()(vector.x) ^ (hash<float>()(vector.y) << 1));
        }
    };

    template<>
    struct hash<levelDrawer::Vertex> {
        size_t operator()(levelDrawer::Vertex const &vertex) const {
            return ((((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1))
                    >> 1) ^
                     (hash<glm::vec2>()(vertex.texCoord) << 1)) >> 1) ^
                   (hash<glm::vec3>()(vertex.normal) << 1);
        }
    };
} // namespace std

class GameRequester;
namespace levelDrawer {
    char constexpr const *KeyVertices = "V";
    char constexpr const *KeyTexCoords = "TX";
    char constexpr const *KeyFaceNormals = "N";
    char constexpr const *KeyIndices = "I";
    char constexpr const *KeyColors = "C";
    char constexpr const *KeyVertexNormals = "NV";

    enum AttributeTypes {
        vertices,
        faceNormals,
        vertexNormals,
        texCoords,
        colors
    };

    using ModelVertices = std::pair<std::vector<Vertex>, std::vector<uint32_t>>;

    bool compareLessVec3(glm::vec3 const &vec1, glm::vec3 const &vec2);

    class ModelDescription {
        friend BaseClassPtrLess<ModelDescription>;
    public:
        static uint8_t const constexpr LOAD_VERTEX_NORMALS = 0x1;
        static uint8_t const constexpr LOAD_FACE_NORMALS = 0x2;
        static uint8_t const constexpr LOAD_BOTH = 0x3;

        virtual uint8_t normalsToLoad() { return LOAD_FACE_NORMALS; }
        virtual std::pair<ModelVertices, ModelVertices> getData(
                std::shared_ptr<GameRequester> const &) = 0;

    protected:
        // returns true if this < other.
        virtual bool compareLess(ModelDescription *) = 0;
    };

    class ModelDescriptionPath : public ModelDescription {
    public:
        uint8_t normalsToLoad() override { return m_normalsToLoad; }
        std::pair<ModelVertices, ModelVertices> getData(std::shared_ptr<GameRequester> const &gameRequester) override;

        ModelDescriptionPath(std::string path, glm::vec3 color = glm::vec3{0.2f, 0.2f, 0.2f}, uint8_t normalsToLoad = LOAD_FACE_NORMALS)
                : m_path{std::move(path)},
                m_usingDefaultColor{true},
                m_color{color},
                m_normalsToLoad{normalsToLoad}
        {}

    protected:
        // Always equal to other, never less than other
        bool compareLess(ModelDescription *other) override {
            auto otherPath = dynamic_cast<ModelDescriptionPath *>(other);
            if (otherPath == nullptr) {
                // should never happen
                throw std::runtime_error("Comparing incompatible pointers");
            }
            if (m_path == otherPath->m_path) {
                if (m_normalsToLoad == otherPath->m_normalsToLoad) {
                    if (m_usingDefaultColor && otherPath->m_usingDefaultColor) {
                        return compareLessVec3(m_color, otherPath->m_color);
                    } else {
                        return m_usingDefaultColor < otherPath->m_usingDefaultColor;
                    }
                } else {
                    return m_normalsToLoad < otherPath->m_normalsToLoad;
                }
            } else {
                return m_path < otherPath->m_path;
            }
        }

    private:
        std::string m_path;
        bool m_usingDefaultColor;
        glm::vec3 m_color;
        uint8_t m_normalsToLoad;

        bool loadModel(
                std::unique_ptr<std::streambuf> const &modelStreamBuf,
                ModelVertices *verticesWithFaceNormals,
                ModelVertices *verticesWithVertexNormals = nullptr);
    };

// creates a quad with each side length 2.0f and center at specified location.
    class ModelDescriptionQuad : public ModelDescription {
    public:
        std::pair<ModelVertices, ModelVertices> getData(std::shared_ptr<GameRequester> const &gameRequester) override;

        ModelDescriptionQuad()
                : m_center{0.0f, 0.0f, 0.0f},
                  m_color{0.2f, 0.2f, 0.2f} {}

        ModelDescriptionQuad(glm::vec3 const &center, glm::vec3 const &color)
                : m_center{center},
                  m_color{color} {}

    protected:
        bool compareLess(ModelDescription *other) override {
            auto otherQuad = dynamic_cast<ModelDescriptionQuad *>(other);
            if (otherQuad == nullptr) {
                // should never happen
                throw std::runtime_error("Quad: Comparing incompatible pointers");
            }

            bool ret = compareLessVec3(m_center, otherQuad->m_center);
            if (!ret && !compareLessVec3(otherQuad->m_center, m_center)) {
                return compareLessVec3(m_color, otherQuad->m_color);
            } else {
                return ret;
            }
        }

    private:
        glm::vec3 m_center;
        glm::vec3 m_color;
    };

    // creates a cube with each side length 2.0f and center at specified location
    class ModelDescriptionCube : public ModelDescription {
    public:
        std::pair<ModelVertices, ModelVertices> getData(std::shared_ptr<GameRequester> const &gameRequester) override;

        ModelDescriptionCube()
                : m_center{0.0f, 0.0f, 0.0f},
                m_color {0.2f, 0.2f, 0.2f} {}

        ModelDescriptionCube(glm::vec3 const &center, glm::vec3 const &color)
                : m_center{center},
                m_color{color} {}

    protected:
        bool compareLess(ModelDescription *other) override {
            auto otherCube = dynamic_cast<ModelDescriptionCube *>(other);
            if (otherCube == nullptr) {
                // should never happen
                throw std::runtime_error("Cube: Comparing incompatible pointers");
            }
            bool ret = compareLessVec3(m_center, otherCube->m_center);
            if (!ret && !compareLessVec3(otherCube->m_center, m_center)) {
                return compareLessVec3(m_color, otherCube->m_color);
            } else {
                return ret;
            }
        }

    private:
        glm::vec3 m_center;
        glm::vec3 m_color;
    };
}
#endif /* AMAZING_LABYRINTH_MODEL_LOADER_HPP */