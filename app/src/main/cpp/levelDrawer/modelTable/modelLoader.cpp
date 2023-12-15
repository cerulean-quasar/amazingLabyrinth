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

#include <streambuf>
#include <boost/endian/conversion.hpp>
#include <boost/noncopyable.hpp>
#include <json.hpp>

#include "../../common.hpp"
#include "modelLoader.hpp"

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

    /* read in the modelcbor file format.
     *
     * The modelcbor format is our own format.  All arrays of vertex attributes (vertices,
     * face normals, vertex normals, colors, and texture coordinates) which are normally represented
     * by a glm::vec are instead flattened such that the array is an array of floats whose elements
     * are defined by: e_(n/a)_(n%a) = array[a*indexn+n%a], where a is the number of components in
     * the glm::vec, n is the nth entry into the index array and e_(n/a)_(n%a) is the n%a component
     * of the n/a glm::vec for the n/a Vertex that we are building.
     *
     * Only unique attributes are stored in the attribute arrays.  The index array contains
     * indices to each of the vertex arrays' elements for a particular vertex.  The indices into
     * the different vertex attribute arrays are interlaced in the following order: position,
     * normal, texture coordinates, color.  There are two index arrays, one for vertices with
     * vertex normals, and one for vertices with face normals.
     *
     * The color and the texture coordinate attributes may be both or either missing.  If they
     * are, then the index array is adjusted to omit indices to the missing attributes.
     */
    class LoadModelSaxClass {
    public:
        LoadModelSaxClass()
                : m_state{state::receivingKey},
                  m_color{0.2f, 0.2f, 0.2f},
                  m_substate{receivingArrayStart}{}

        explicit LoadModelSaxClass(glm::vec3 color)
                : m_state{state::receivingKey},
                m_color{color},
                m_substate{receivingArrayStart}{}

        bool usingDefaultColor() {
            return m_colors.empty();
        }

        void getVertices(
                ModelVertices *verticesWithFaceNormals,
                ModelVertices *verticesWithVertexNormals) {

            if (m_vertices.empty() ||
                m_faceNormals.empty() ||
                m_vertexNormals.empty()) {
                /* These vertex attributes should always be present */
                throw std::runtime_error("Not all the mandatory vertex attributes present in the model file");
            }

            /* only certain attributes are present in the indices.  Identify which ones and which
             * position they will take.
             */
            size_t inumber = 3;
            size_t iposition = 0;
            size_t ifaceNormals = 1;
            size_t ivertexNormals = 2;
            size_t itexcoord = 0;
            size_t icolor = 0;
            if (!m_texCoords.empty()) {
                itexcoord = inumber++;
            }

            if (!m_colors.empty()) {
                icolor = inumber++;
            }

            size_t size = 0;
            for (auto const &indices : m_indices) {
                size += indices.size();
            }

            std::unordered_map<Vertex, uint32_t> uniqueVerticesWithFaceNormals{};
            uniqueVerticesWithFaceNormals.reserve(size / inumber);
            std::unordered_map<Vertex, uint32_t> uniqueVerticesWithVertexNormals{};
            uniqueVerticesWithVertexNormals.reserve(size / inumber);
            for (auto const &indices : m_indices) {
                for (size_t i = 0; i < indices.size() / inumber; i++) {
                    if (verticesWithFaceNormals) {
                        createModelVertex(verticesWithFaceNormals, uniqueVerticesWithFaceNormals,
                                          m_faceNormals, indices, i, inumber, iposition,
                                          ifaceNormals, itexcoord, icolor);
                    }
                    if (verticesWithVertexNormals) {
                        createModelVertex(verticesWithVertexNormals,
                                          uniqueVerticesWithVertexNormals, m_vertexNormals, indices,
                                          i, inumber, iposition, ivertexNormals, itexcoord, icolor);
                    }
                }
            }
        }

        // called when null is parsed
        static bool null() { return true; }

        // called when a boolean is parsed; value is passed
        bool boolean(bool) {
            return true;
        }

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
                    m_vertices.push_back(static_cast<float>(val));
                    break;
                case state::receivingFaceNormals:
                    m_faceNormals.push_back(static_cast<float>(val));
                    break;
                case state::receivingVertexNormals:
                    m_vertexNormals.push_back(static_cast<float>(val));
                    break;
                case state::receivingTexCoords:
                    m_texCoords.push_back(static_cast<float>(val));
                    break;
                case state::receivingColors:
                    m_colors.push_back(static_cast<float>(val));
                    break;
                default:
                    break;
            }
            return true;
        }

        // called when a string is parsed; value is passed and can be safely moved away
        static bool string(nlohmann::json::string_t &) { return true; }

        // called when an object or array begins or ends, resp. The number of elements is passed (or -1 if not known)
        static bool start_object(std::size_t) { return true; }

        static bool end_object() { return true; }

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
            } else if (m_state == state::receivingFaceNormals) {
                m_faceNormals.reserve(arrsize);
            } else if (m_state == state::receivingVertexNormals) {
                m_vertexNormals.reserve(arrsize);
            } else if (m_state == state::receivingTexCoords) {
                m_texCoords.reserve(arrsize);
            } else if (m_state == state::receivingColors) {
                m_colors.reserve(arrsize);
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
            } else if (val == KeyFaceNormals) {
                m_state = state::receivingFaceNormals;
            } else if (val == KeyVertexNormals) {
                m_state = state::receivingVertexNormals;
            } else if (val == KeyTexCoords) {
                m_state = state::receivingTexCoords;
            } else if (val == KeyColors) {
                m_state = state::receivingColors;
            } else if (val == KeyIndices) {
                m_state = state::receivingIndices;
                m_substate = substate::receivingArrayStart;
            }

            return true;
        }

        // called when a parse error occurs; byte position, the last token, and an exception is passed
        static bool parse_error(std::size_t /*position*/, const std::string & /*last_token*/,
                         const nlohmann::json::exception &ex) {
            throw std::runtime_error(ex.what());
        }

    private:
        void createModelVertex(
                ModelVertices *verticesWithNormals,
                std::unordered_map<Vertex, uint32_t> &uniqueVertices,
                std::vector<float> const &normals,
                std::vector<uint32_t> const &indices,
                uint32_t icurrent,
                uint32_t inumber,
                uint32_t iposition,
                uint32_t inormal,
                uint32_t itexcoord,
                uint32_t icolor)
        {
            Vertex vertex = {};
            vertex.pos = {
                    m_vertices[3 * indices[icurrent * inumber + iposition] + 0],
                    m_vertices[3 * indices[icurrent * inumber + iposition] + 1],
                    m_vertices[3 * indices[icurrent * inumber + iposition] + 2]
            };

            if (m_texCoords.empty()) {
                vertex.texCoord = {0.0f, 0.0f};
            } else {
                vertex.texCoord = {
                        m_texCoords[2 * indices[icurrent * inumber + itexcoord] + 0],
                        1.0f - m_texCoords[2 * indices[icurrent * inumber + itexcoord] + 1]
                };
            }

            if (m_colors.empty()) {
                vertex.color = m_color;
            } else {
                /* drop the alpha for now */
                vertex.color = {
                        m_colors[4 * indices[icurrent * inumber + icolor] + 0],
                        m_colors[4 * indices[icurrent * inumber + icolor] + 1],
                        m_colors[4 * indices[icurrent * inumber + icolor] + 2]
                };
            }

            vertex.normal = {normals[3 * indices[icurrent * inumber + inormal] + 0],
                             normals[3 * indices[icurrent * inumber + inormal] + 1],
                             normals[3 * indices[icurrent * inumber + inormal] + 2]};

            auto itPair2 = uniqueVertices.emplace(vertex,
                                                  static_cast<uint32_t>(verticesWithNormals->first.size()));
            if (itPair2.second) {
                verticesWithNormals->first.push_back(vertex);
            }
            verticesWithNormals->second.push_back(itPair2.first->second);
        }

        bool processUnsignedInteger(uint32_t val) {
            if (m_state == state::receivingIndices && !m_indices.empty()) {
                m_indices[m_indices.size() - 1].push_back(val);
            }

            return true;
        }

        enum state {
            receivingKey = 0,
            receivingFaceNormals = 2,
            receivingVertexNormals = 3,
            receivingVertices = 4,
            receivingTexCoords = 5,
            receivingColors = 6,
            receivingIndices = 7
        };

        enum substate {
            receivingArrayStart = 0,
            receivingArrayStart2 = 1,
            receivingArray = 2
        };

        state m_state;
        glm::vec3 m_color;
        substate m_substate;
        std::vector<std::vector<uint32_t>> m_indices;
        std::vector<float> m_faceNormals;
        std::vector<float> m_vertexNormals;
        std::vector<float> m_vertices;
        std::vector<float> m_texCoords;
        std::vector<float> m_colors;
    };

    std::pair<ModelVertices, ModelVertices> ModelDescriptionPath::getData(
            const std::shared_ptr<GameRequester> &gameRequester)
    {
        std::pair<ModelVertices, ModelVertices> vertices;
        ModelVertices *verticesWithFaceNormals = nullptr;
        ModelVertices *verticesWithVertexNormals = nullptr;
        if (m_normalsToLoad & LOAD_FACE_NORMALS) {
            verticesWithFaceNormals = &vertices.first;
        }
        if (m_normalsToLoad & LOAD_VERTEX_NORMALS) {
            verticesWithVertexNormals = &vertices.second;
        }
        loadModel(gameRequester->getAssetStream(m_path), verticesWithFaceNormals, verticesWithVertexNormals);
        return vertices;
    }

    bool ModelDescriptionPath::loadModel(
            std::unique_ptr<std::streambuf> const &modelStreamBuf,
            ModelVertices *verticesWithFaceNormals,
            ModelVertices *verticesWithVertexNormals) {

        try {
            std::istream assetIstream(modelStreamBuf.get());

            LoadModelSaxClass sax{m_color};

            nlohmann::json j = nlohmann::json::sax_parse(assetIstream, &sax,
                                                         nlohmann::json::input_format_t::cbor);
            sax.getVertices(verticesWithFaceNormals, verticesWithVertexNormals);
            m_usingDefaultColor = sax.usingDefaultColor();
        } catch (...) {
            return false;
        }

        return true;
    }

    std::pair<ModelVertices, ModelVertices> ModelDescriptionQuad::getData(std::shared_ptr<GameRequester> const &) {
        if (normalsToLoad() & LOAD_VERTEX_NORMALS) {
            throw std::runtime_error("Requesting vertex normals does not apply for this object.");
        }
        ModelVertices vertices{};
        Vertex vertex = {};
        vertex.color = m_color;
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

        return std::make_pair(std::move(vertices), ModelVertices());
    }

// creates a cube with each side length 2.0f.
    std::pair<ModelVertices, ModelVertices> ModelDescriptionCube::getData(std::shared_ptr<GameRequester> const &) {
        if (normalsToLoad() & LOAD_VERTEX_NORMALS) {
            throw std::runtime_error("Requesting vertex normals does not apply for this object.");
        }
        ModelVertices vertices{};
        Vertex vertex = {};
        vertex.color = m_color;

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

        return std::make_pair(std::move(vertices), ModelVertices());
    }
}
