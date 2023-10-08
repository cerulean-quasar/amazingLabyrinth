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

class GameRequester;
namespace levelDrawer {
    char constexpr const *KeyVertices = "V";
    char constexpr const *KeyTexCoords = "TX";
    char constexpr const *KeyNormals = "N";
    char constexpr const *KeyIndices = "I";

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

    using ModelVertices = std::pair<std::vector<Vertex>, std::vector<uint32_t>>;

    bool compareLessVec3(glm::vec3 const &vec1, glm::vec3 const &vec2);

    class ModelDescription {
        friend BaseClassPtrLess<ModelDescription>;
    public:
        virtual bool shouldLoadVertexNormals() { return false; }
        virtual std::pair<ModelVertices, ModelVertices> getDataWithVertexNormalsAlso(
                std::shared_ptr<GameRequester> const &)
        {
            throw std::runtime_error("Requesting vertex normals does not apply for this object.");
        }
        virtual ModelVertices getData(std::shared_ptr<GameRequester> const &gameRequester) = 0;

    protected:
        // returns true if this < other.
        virtual bool compareLess(ModelDescription *) = 0;
    };

    class ModelReader{
    public:
        virtual bool read(std::unique_ptr<std::streambuf> const &modelStreamBuf,
                          ModelVertices &verticesWithFaceNormals,
                          ModelVertices *verticesWithVertexNormals) = 0;


        ModelReader() = default;
        virtual ~ModelReader() = default;
    };

    class ModelDescriptionPath : public ModelDescription {
    public:
        bool shouldLoadVertexNormals() override { return m_loadVertexNormals; }
        std::pair<ModelVertices, ModelVertices> getDataWithVertexNormalsAlso(
                std::shared_ptr<GameRequester> const &gameRequester) override;

        ModelVertices getData(std::shared_ptr<GameRequester> const &gameRequester) override;

        ModelDescriptionPath(std::string path, glm::vec3 color = glm::vec3{0.2f, 0.2f, 0.2f}, bool loadVertexNormalsAlso = false)
                : m_path{std::move(path)},
                m_color{color},
                m_loadVertexNormals{loadVertexNormalsAlso}
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
                return compareLessVec3(m_color, otherPath->m_color);
            } else {
                return m_path < otherPath->m_path;
            }
        }

    private:
        std::string m_path;
        glm::vec3 m_color;
        bool m_loadVertexNormals;

        std::shared_ptr<ModelReader> getReader();

        static bool loadModel(
                std::unique_ptr<std::streambuf> const &modelStreamBuf,
                std::shared_ptr<ModelReader> &modelReader,
                ModelVertices &verticesWithFaceNormals,
                ModelVertices *verticesWithVertexNormals = nullptr);
    };

// creates a quad with each side length 2.0f and center at specified location.
    class ModelDescriptionQuad : public ModelDescription {
    public:
        ModelVertices getData(std::shared_ptr<GameRequester> const &gameRequester) override;

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
        ModelVertices getData(std::shared_ptr<GameRequester> const &gameRequester) override;

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