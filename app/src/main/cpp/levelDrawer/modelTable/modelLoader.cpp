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

#include <streambuf>

#include <json.hpp>

#include "../../common.hpp"
#include "modelLoader.hpp"
namespace std {
    template<>
    struct hash<glm::vec3> {
        size_t operator()(glm::vec3 vector) const {
            return ((hash<float>()(vector.x) ^
                     (hash<float>()(vector.y) << 1)) >> 1) ^
                   (hash<float>()(vector.z) << 1);
        }
    };

    template<>
    struct hash<glm::vec2> {
        size_t operator()(glm::vec2 vector) const {
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
}

namespace levelDrawer {
    bool Vertex::operator==(const Vertex &other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord &&
               normal == other.normal;
    }

    bool compareLessVec3(glm::vec3 const &vec1, glm::vec3 const &vec2) {
        if (vec1.x != vec2.x) {
            return vec1.x < vec2.x;
        } else if (vec1.y != vec2.y) {
            return vec1.y < vec2.y;
        } else if (vec1.z != vec2.z) {
            return vec1.z < vec2.z;
        }

        return false;
    }

    class LoadModelSaxClass {
    public:
        LoadModelSaxClass()
                : m_state{state::receivingKey} {}

        void getVertices(
                std::pair<std::vector<Vertex>, std::vector<uint32_t>> &verticesWithFaceNormals,
                std::pair<std::vector<Vertex>, std::vector<uint32_t>> *verticesWithVertexNormals) {
            size_t size = 0;
            for (auto const &indices : m_indices) {
                size += indices.size();
            }
            std::unordered_map<Vertex, uint32_t> uniqueVertices{};
            uniqueVertices.reserve(size);
            std::unordered_map<Vertex, uint32_t> uniqueVerticesWithVertexNormals{};
            uniqueVerticesWithVertexNormals.reserve(size);
            for (auto const &indices : m_indices) {
                std::unordered_map<glm::vec3, glm::vec3> vertexNormals;
                vertexNormals.reserve(indices.size());
                if (verticesWithVertexNormals) {
                    for (size_t i = 0; i < indices.size() / 3; i++) {
                        glm::vec3 pos = {
                                m_vertices[3 * indices[i * 3 + 0] + 0],
                                m_vertices[3 * indices[i * 3 + 0] + 1],
                                m_vertices[3 * indices[i * 3 + 0] + 2]
                        };

                        glm::vec3 normal = {m_normals[3 * indices[i * 3 + 1] + 0],
                                            m_normals[3 * indices[i * 3 + 1] + 1],
                                            m_normals[3 * indices[i * 3 + 1] + 2]};

                        auto it = vertexNormals.find(pos);
                        if (it == vertexNormals.end()) {
                            vertexNormals.insert(std::make_pair(pos, normal));
                        } else {
                            it->second += normal;
                        }
                    }
                }

                for (size_t i = 0; i < indices.size() / 3; i++) {
                    Vertex vertex = {};
                    vertex.pos = {
                            m_vertices[3 * indices[i * 3 + 0] + 0],
                            m_vertices[3 * indices[i * 3 + 0] + 1],
                            m_vertices[3 * indices[i * 3 + 0] + 2]
                    };

                    if (m_texCoords.size() > 0) {
                        vertex.texCoord = {
                                m_texCoords[2 * indices[i * 3 + 2] + 0],
                                1.0f - m_texCoords[2 * indices[i * 3 + 2] + 1]
                        };
                    } else {
                        vertex.texCoord = {0.0f, 0.0f};
                    }

                    vertex.color = {0.2f, 0.2f, 0.2f};

                    if (verticesWithVertexNormals) {
                        Vertex vertexWithVertexNormal = vertex;
                        auto it = vertexNormals.find(vertexWithVertexNormal.pos);
                        if (it == vertexNormals.end()) {
                            throw std::runtime_error("Vertex normal not found when loading model");
                        }
                        vertexWithVertexNormal.normal = it->second;

                        /*
                        auto itUniqueVerticesWithVertexNormals = uniqueVerticesWithVertexNormals.find(vertexWithVertexNormal);
                        if (itUniqueVerticesWithVertexNormals == uniqueVerticesWithVertexNormals.end()) {
                            auto itPair = uniqueVerticesWithVertexNormals.emplace(vertexWithVertexNormal,
                                    static_cast<uint32_t>(verticesWithVertexNormals->first.size()));
                            itUniqueVerticesWithVertexNormals = itPair.first;
                            verticesWithVertexNormals->first.push_back(vertexWithVertexNormal);
                        }*/
                        auto itPair = uniqueVerticesWithVertexNormals.emplace(
                                vertexWithVertexNormal,
                                static_cast<uint32_t>(verticesWithVertexNormals->first.size()));

                        if (itPair.second) {
                            verticesWithVertexNormals->first.push_back(vertexWithVertexNormal);
                        }
                        verticesWithVertexNormals->second.push_back(itPair.first->second);
                    }

                    vertex.normal = {m_normals[3 * indices[i * 3 + 1] + 0],
                                     m_normals[3 * indices[i * 3 + 1] + 1],
                                     m_normals[3 * indices[i * 3 + 1] + 2]};

                    auto itPair2 = uniqueVertices.emplace(vertex,
                                                          static_cast<uint32_t>(verticesWithFaceNormals.first.size()));
                    if (itPair2.second) {
                        verticesWithFaceNormals.first.push_back(vertex);
                    }
                    verticesWithFaceNormals.second.push_back(itPair2.first->second);
                }
            }
        }

        // called when null is parsed
        bool null() { return true; }

        // called when a boolean is parsed; value is passed
        bool boolean(bool) { return true; }

        // called when a signed or unsigned integer number is parsed; value is passed
        bool number_integer(nlohmann::json::number_integer_t val) {
            // if the integer is less than 0, that is an unexpected value and we don't know what
            // to do with it, error out.
            if (val < 0) {
                return false;
            }
            return processUnsignedInteger(static_cast<uint32_t>(val));
        }

        bool number_unsigned(nlohmann::json::number_unsigned_t val) {
            return processUnsignedInteger(static_cast<uint32_t>(val));
        }

        // called when a floating-point number is parsed; value and original string is passed
        bool number_float(nlohmann::json::number_float_t val, nlohmann::json::string_t const &) {
            switch (m_state) {
                case state::receivingVertices:
                    m_vertices.push_back(val);
                    break;
                case state::receivingNormals:
                    m_normals.push_back(val);
                    break;
                case state::receivingTexCoords:
                    m_texCoords.push_back(val);
                    break;
                default:
                    break;
            }
            return true;
        }

        // called when a string is parsed; value is passed and can be safely moved away
        bool string(nlohmann::json::string_t &) { return true; }

        // called when an object or array begins or ends, resp. The number of elements is passed (or -1 if not known)
        bool start_object(std::size_t) { return true; }

        bool end_object() { return true; }

        bool start_array(std::size_t arrsize) {
            if (m_state == state::receivingIndices) {
                if (m_substate == substate::receivingArrayStart) {
                    m_substate = substate::receivingArrayStart2;
                    m_indices.reserve(arrsize);
                } else if (m_substate == substate::receivingArrayStart2) {
                    m_substate = substate::receivingArray;
                    m_indices.emplace_back();
                    m_indices[m_indices.size() - 1].reserve(arrsize);
                }
            } else if (m_state == state::receivingVertices) {
                m_vertices.reserve(arrsize);
            } else if (m_state == state::receivingNormals) {
                m_normals.reserve(arrsize);
            } else if (m_state == state::receivingTexCoords) {
                m_normals.reserve(arrsize);
            }
            return true;
        }

        bool end_array() {
            if (m_state == state::receivingIndices) {
                if (m_substate == substate::receivingArray) {
                    m_substate = substate::receivingArrayStart2;
                } else if (m_substate == substate::receivingArrayStart2) {
                    m_substate = substate::receivingArrayStart;
                }
            }

            return true;
        }

        // called when an object key is parsed; value is passed and can be safely moved away
        bool key(nlohmann::json::string_t &val) {
            if (val == KeyVertices) {
                m_state = state::receivingVertices;
            } else if (val == KeyNormals) {
                m_state = state::receivingNormals;
            } else if (val == KeyTexCoords) {
                m_state = state::receivingTexCoords;
            } else if (val == KeyIndices) {
                m_state = state::receivingIndices;
                m_substate = substate::receivingArrayStart;
            }

            return true;
        }

        // called when a parse error occurs; byte position, the last token, and an exception is passed
        bool parse_error(std::size_t /*position*/, const std::string & /*last_token*/,
                         const nlohmann::json::exception &ex) {
            throw std::runtime_error(ex.what());
        }

    private:
        bool processUnsignedInteger(uint32_t val) {
            if (m_state == state::receivingIndices && !m_indices.empty()) {
                m_indices[m_indices.size() - 1].push_back(val);
            }

            return true;
        }

        enum state {
            receivingKey = 0,
            receivingNormals = 1,
            receivingVertices = 2,
            receivingTexCoords = 3,
            receivingIndices = 4
        };

        enum substate {
            receivingArrayStart = 0,
            receivingArrayStart2 = 1,
            receivingArray = 2
        };

        state m_state;
        substate m_substate;
        std::vector<std::vector<uint32_t>> m_indices;
        std::vector<float> m_normals;
        std::vector<float> m_vertices;
        std::vector<float> m_texCoords;
    };

    std::pair<ModelVertices, ModelVertices> ModelDescriptionPath::getDataWithVertexNormalsAlso(
            const std::shared_ptr<GameRequester> &gameRequester)
    {
        ModelVertices vertices;
        ModelVertices verticesWithVertexNormals;
        loadModel(gameRequester->getAssetStream(m_path), vertices, &verticesWithVertexNormals);
        return std::make_pair(std::move(vertices), std::move(verticesWithVertexNormals));
    }

    ModelVertices
    ModelDescriptionPath::getData(std::shared_ptr<GameRequester> const &gameRequester) {
        ModelVertices vertices;
        loadModel(gameRequester->getAssetStream(m_path), vertices);
        return vertices;
    }

    bool ModelDescriptionPath::loadModel(
            std::unique_ptr<std::streambuf> const &modelStreamBuf,
            ModelVertices &verticesWithFaceNormals,
            ModelVertices *verticesWithVertexNormals) {
        std::istream assetIstream(modelStreamBuf.get());

        try {
            LoadModelSaxClass sax;

            nlohmann::json j = nlohmann::json::sax_parse(assetIstream, &sax,
                                                         nlohmann::json::input_format_t::cbor);
            sax.getVertices(verticesWithFaceNormals, verticesWithVertexNormals);
        } catch (...) {
            return false;
        }

        return true;
    }

    ModelVertices ModelDescriptionQuad::getData(std::shared_ptr<GameRequester> const &) {
        ModelVertices vertices{};
        Vertex vertex = {};
        vertex.color = {0.2f, 0.2f, 0.2f};
        vertex.normal = {0.0f, 0.0f, 1.0f};

        vertex.pos = glm::vec3{-1.0f, 1.0f, 0.0f} + m_center;
        vertex.texCoord = {0.0f, 0.0f};
        vertices.first.push_back(vertex);

        vertex.pos = glm::vec3{-1.0f, -1.0f, 0.0f} + m_center;
        vertex.texCoord = {0.0f, 1.0f};
        vertices.first.push_back(vertex);

        vertex.pos = glm::vec3{1.0f, -1.0f, 0.0f} + m_center;
        vertex.texCoord = {1.0f, 1.0f};
        vertices.first.push_back(vertex);

        vertex.pos = glm::vec3{1.0f, 1.0f, 0.0f} + m_center;
        vertex.texCoord = {1.0f, 0.0f};
        vertices.first.push_back(vertex);

        vertices.second.push_back(0);
        vertices.second.push_back(1);
        vertices.second.push_back(2);

        vertices.second.push_back(0);
        vertices.second.push_back(2);
        vertices.second.push_back(3);

        return vertices;
    }

// creates a cube with each side length 2.0f.
    ModelVertices ModelDescriptionCube::getData(std::shared_ptr<GameRequester> const &) {
        ModelVertices vertices{};
        Vertex vertex = {};
        vertex.color = {0.2f, 0.2f, 0.2f};

        std::array<glm::vec3, 8> positions = {
                glm::vec3{-1.0f, 1.0f, 1.0f} + m_center,
                glm::vec3{-1.0f, -1.0f, 1.0f} + m_center,
                glm::vec3{1.0f, -1.0f, 1.0f} + m_center,
                glm::vec3{1.0f, 1.0f, 1.0f} + m_center,

                glm::vec3{-1.0f, 1.0f, -1.0f} + m_center,
                glm::vec3{-1.0f, -1.0f, -1.0f} + m_center,
                glm::vec3{1.0f, -1.0f, -1.0f} + m_center,
                glm::vec3{1.0f, 1.0f, -1.0f} + m_center
        };

        // the top, z = 1.0
        size_t i = vertices.first.size();
        vertex.normal = {0.0f, 0.0f, 1.0f};

        vertex.pos = positions[0];
        vertex.texCoord = {0.0f, 0.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[1];
        vertex.texCoord = {0.0f, 1.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[2];
        vertex.texCoord = {1.0f / 2.0f, 1.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[3];
        vertex.texCoord = {1.0f / 2.0f, 0.0f};
        vertices.first.push_back(vertex);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 1);
        vertices.second.push_back(i + 2);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 2);
        vertices.second.push_back(i + 3);

        // the bottom, z = -1
        i = vertices.first.size();
        vertex.normal = {0.0f, 0.0f, -1.0f};

        vertex.pos = positions[7];
        vertex.texCoord = {0.0f, 1.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[6];
        vertex.texCoord = {0.0f, 2.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[5];
        vertex.texCoord = {1.0f / 2.0f, 2.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[4];
        vertex.texCoord = {1.0f / 2.0f, 1.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 1);
        vertices.second.push_back(i + 2);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 2);
        vertices.second.push_back(i + 3);

        // the side, y = -1
        i = vertices.first.size();
        vertex.normal = {0.0f, -1.0f, 0.0f};

        vertex.pos = positions[1];
        vertex.texCoord = {0.0f, 2.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[5];
        vertex.texCoord = {0.0f, 1.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[6];
        vertex.texCoord = {1.0f / 2.0f, 1.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[2];
        vertex.texCoord = {1.0f / 2.0f, 2.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 1);
        vertices.second.push_back(i + 2);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 2);
        vertices.second.push_back(i + 3);

        // the side, y = 1
        i = vertices.first.size();
        vertex.normal = {0.0f, 1.0f, 0.0f};

        vertex.pos = positions[3];
        vertex.texCoord = {1.0f / 2.0f, 0.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[7];
        vertex.texCoord = {1.0f / 2.0f, 1.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[4];
        vertex.texCoord = {1.0f, 1.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[0];
        vertex.texCoord = {1.0f, 0.0f};
        vertices.first.push_back(vertex);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 1);
        vertices.second.push_back(i + 2);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 2);
        vertices.second.push_back(i + 3);

        // the side, x = -1
        i = vertices.first.size();
        vertex.normal = {-1.0f, 0.0f, 0.0f};

        vertex.pos = positions[0];
        vertex.texCoord = {1.0f / 2.0f, 1.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[4];
        vertex.texCoord = {1.0f / 2.0f, 2.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[5];
        vertex.texCoord = {1.0f, 2.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[1];
        vertex.texCoord = {1.0f, 1.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 1);
        vertices.second.push_back(i + 2);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 2);
        vertices.second.push_back(i + 3);

        // the side, x = 1
        i = vertices.first.size();
        vertex.normal = {1.0f, 0.0f, 0.0f};

        vertex.pos = positions[2];
        vertex.texCoord = {1.0f / 2.0f, 2.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[6];
        vertex.texCoord = {1.0f / 2.0f, 1.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[7];
        vertex.texCoord = {1.0f, 1.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[3];
        vertex.texCoord = {1.0f, 2.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 1);
        vertices.second.push_back(i + 2);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 2);
        vertices.second.push_back(i + 3);

        return vertices;
    }
}
