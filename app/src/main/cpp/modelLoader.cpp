#include <streambuf>

#include <json.hpp>

#include "modelLoader.hpp"

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

char constexpr const *KeyX = "X";
char constexpr const *KeyY = "Y";
char constexpr const *KeyZ = "Z";

// serialize glm::vec types
namespace nlohmann {
    // glm::vec3
    template <>
    struct adl_serializer<glm::vec3> {
        static void to_json(json &j, glm::vec3 const &v) {
            j = json{std::vector<float>{v.x, v.y, v.z}};
        }

        static void from_json(const json &j, glm::vec3 &v) {
            std::vector<float> tmp = j.get<std::vector<float>>();
            if (tmp.size() < 3) {
                v.x = v.y = v.z = 0.0f;
                return;
            }
            v.x = tmp[0];
            v.y = tmp[1];
            v.z = tmp[2];
        }
    };

    template <>
    struct adl_serializer<glm::vec2> {
        // glm::vec2
        static void to_json(json &j, glm::vec2 const &v) {
            j = json{std::vector<float>{v.x, v.y}};
        }

        static void from_json(const json &j, glm::vec2 &v) {
            std::vector<float> tmp = j.get<std::vector<float>>();
            if (tmp.size() < 2) {
                v.x = v.y = 0.0f;
                return;
            }
            v.x = tmp[0];
            v.y = tmp[1];
        }
    };
}

bool Vertex::operator==(const Vertex& other) const {
    return pos == other.pos && color == other.color && texCoord == other.texCoord && normal == other.normal;
}

// serialize Vertex
char constexpr const *KeyColor = "C";
char constexpr const *KeyNormal = "N";
char constexpr const *KeyPosition = "p";
char constexpr const *KeyTextureCoordinate = "T";
void to_json(nlohmann::json &j, Vertex const &v) {
    j[KeyColor] = v.color;
    j[KeyNormal] = v.normal;
    j[KeyPosition] = v.pos;
    j[KeyTextureCoordinate] = v.texCoord;
}

void from_json(nlohmann::json const &j, Vertex &v) {
    v.color = j[KeyColor].get<glm::vec3>();
    v.normal = j[KeyNormal].get<glm::vec3>();
    v.pos = j[KeyPosition].get<glm::vec3>();
    v.texCoord = j[KeyTextureCoordinate].get<glm::vec2>();
}

class LoadModelSaxClass {
public:
    LoadModelSaxClass(bool receiveVertexNormals = false)
        : m_state{state::receivingKey},
        m_receiveVertexNormals{receiveVertexNormals},
        m_v {glm::vec3{0.0f, 0.0f, 0.0f},
             glm::vec3{0.2f, 0.2f, 0.2f},
             glm::vec2{0.0f, 0.0f},
             glm::vec3{0.0f, 0.0f, 0.0f}}
    {}

    void fillVerticesWithFaceNormals(std::pair<std::vector<Vertex>, std::vector<uint32_t>> &v) {
        v.first = std::move(m_verticesWithFaceNormals.first);
        v.second = std::move(m_verticesWithFaceNormals.second);
    }

    void fillVerticesWithVertexNormals(std::pair<std::vector<Vertex>, std::vector<uint32_t>> &v) {
        v.first = std::move(m_verticesWithVertexNormals.first);
        v.second = std::move(m_verticesWithVertexNormals.second);
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
        if (m_state == state::receivingFaceNormals || m_state == state::receivingVertexNormals) {
            switch (m_substate) {
            case substate::px:
                m_v.pos.x = val;
                m_substate = substate::py;
                break;
            case substate::py:
                m_v.pos.y = val;
                m_substate = substate::pz;
                break;
            case substate::pz:
                m_v.pos.z = val;
                m_substate = substate::nx;
                break;
            case substate::nx:
                m_v.normal.x = val;
                m_substate = substate::ny;
                break;
            case substate::ny:
                m_v.normal.y = val;
                m_substate = substate::nz;
                break;
            case substate::nz:
                m_v.normal.z = val;
                if (m_state == state::receivingFaceNormals) {
                    m_substate = substate::tx;
                } else {
                    m_v.texCoord = glm::vec2{0.0f, 0.0f};
                    m_verticesWithVertexNormals.first.push_back(m_v);
                    m_substate = substate::px;
                }
                break;
            case substate::tx:
                m_v.texCoord.x = val;
                m_substate = substate::ty;
                break;
            case substate::ty:
                m_v.texCoord.y = val;
                m_verticesWithFaceNormals.first.push_back(m_v);
                m_substate = substate::px;
                break;
            }
        }

        return true;
    }

    // called when a string is parsed; value is passed and can be safely moved away
    bool string(nlohmann::json::string_t &) { return true; }

    // called when an object or array begins or ends, resp. The number of elements is passed (or -1 if not known)
    bool start_object(std::size_t) { return true; }
    bool end_object() { return true; }
    bool start_array(std::size_t) { return true; }
    bool end_array() { return true; }
    // called when an object key is parsed; value is passed and can be safely moved away
    bool key(nlohmann::json::string_t& val) {
        if (val == KeyVerticesWithFaceNormals) {
            m_state = state::receivingFaceNormals;
            m_substate = substate::px;
        } else if (val == KeyVerticesWithVertexNormals) {
            m_state = state::receivingVertexNormals;
            m_substate = substate::px;
        } else if (val == KeyIndicesForFaceNormals) {
            m_state = state::receivingFaceNormalIndices;
        } else if (val == KeyIndicesForVertexNormals) {
            m_state = state::receivingVertexNormalIndices;
        }

        return true;
    }

    // called when a parse error occurs; byte position, the last token, and an exception is passed
    bool parse_error(std::size_t /*position*/, const std::string& /*last_token*/, const nlohmann::json::exception& ex) {
        throw std::runtime_error(ex.what());
    }
private:
    bool processUnsignedInteger(uint32_t val) {
        if (m_state == state::receivingFaceNormalIndices) {
            m_verticesWithFaceNormals.second.push_back(val);
        } else if (m_state == state::receivingVertexNormalIndices) {
            m_verticesWithVertexNormals.second.push_back(val);
        }

        // ignore integers received if not in the proper state: the file could contain vertex normals
        // but we don't need them.
        return true;
    }

    enum state {
        receivingKey = 0,
        receivingVertexNormals = 1,
        receivingFaceNormals = 2,
        receivingVertexNormalIndices = 3,
        receivingFaceNormalIndices = 4
    };

    enum substate {
        px, py, pz,
        nx, ny, nz,
        tx, ty
    };

    state m_state;
    bool m_receiveVertexNormals;
    std::pair<std::vector<Vertex>, std::vector<uint32_t>> m_verticesWithFaceNormals;
    std::pair<std::vector<Vertex>, std::vector<uint32_t>> m_verticesWithVertexNormals;
    substate m_substate;
    Vertex m_v;
};

class LoadModelSaxClass2 {
public:
    LoadModelSaxClass2()
            : m_state{state::receivingKey}
    {}

    void getVertices(
        std::pair<std::vector<Vertex>, std::vector<uint32_t>> &verticesWithFaceNormals,
        std::pair<std::vector<Vertex>, std::vector<uint32_t>> *verticesWithVertexNormals)
    {
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
                for (size_t i = 0; i < indices.size()/3; i++) {
                    glm::vec3 pos = {
                            m_vertices[3 * indices[i*3 + 0] + 0],
                            m_vertices[3 * indices[i*3 + 0] + 1],
                            m_vertices[3 * indices[i*3 + 0] + 2]
                    };

                    glm::vec3 normal = {m_normals[3 * indices[i*3+1] + 0],
                                        m_normals[3 * indices[i*3+1] + 1],
                                        m_normals[3 * indices[i*3+1] + 2]};

                    auto it = vertexNormals.find(pos);
                    if (it == vertexNormals.end()) {
                        vertexNormals.insert(std::make_pair(pos, normal));
                    } else {
                        it->second += normal;
                    }
                }

                /*for (auto &item: vertexNormals) {
                    item.second = glm::normalize(item.second);
                }*/
            }

            for (size_t i = 0; i < indices.size()/3; i++) {
                Vertex vertex = {};
                vertex.pos = {
                        m_vertices[3 * indices[i*3 + 0] + 0],
                        m_vertices[3 * indices[i*3 + 0] + 1],
                        m_vertices[3 * indices[i*3 + 0] + 2]
                };

                if (m_texCoords.size() > 0) {
                    vertex.texCoord = {
                            m_texCoords[2 * indices[i*3 + 2] + 0],
                            1.0f - m_texCoords[2 * indices[i*3 + 2] + 1]
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
                    auto itPair = uniqueVerticesWithVertexNormals.emplace(vertexWithVertexNormal,
                                                                          static_cast<uint32_t>(verticesWithVertexNormals->first.size()));

                    if (itPair.second) {
                        verticesWithVertexNormals->first.push_back(vertexWithVertexNormal);
                    }
                    verticesWithVertexNormals->second.push_back(itPair.first->second);
                }

                vertex.normal = {m_normals[3 * indices[i*3 + 1] + 0],
                                 m_normals[3 * indices[i*3 + 1] + 1],
                                 m_normals[3 * indices[i*3 + 1] + 2]};

                auto itPair2 = uniqueVertices.emplace(vertex, static_cast<uint32_t>(verticesWithFaceNormals.first.size()));
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
    bool key(nlohmann::json::string_t& val) {
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
    bool parse_error(std::size_t /*position*/, const std::string& /*last_token*/, const nlohmann::json::exception& ex) {
        throw std::runtime_error(ex.what());
    }
private:
    bool processUnsignedInteger(uint32_t val) {
        if (m_state == state::receivingIndices && !m_indices.empty()) {
            m_indices[m_indices.size()-1].push_back(val);
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

bool loadModel(
    std::unique_ptr<std::streambuf> const &modelStreamBuf,
    std::pair<std::vector<Vertex>, std::vector<uint32_t>> &verticesWithFaceNormals,
    std::pair<std::vector<Vertex>, std::vector<uint32_t>> *verticesWithVertexNormals)
{
    std::istream assetIstream(modelStreamBuf.get());

    try {
        LoadModelSaxClass sax(verticesWithVertexNormals != nullptr);

        nlohmann::json j = nlohmann::json::sax_parse(assetIstream, &sax, nlohmann::json::input_format_t::cbor);
        sax.fillVerticesWithFaceNormals(verticesWithFaceNormals);
        if (verticesWithVertexNormals) {
            sax.fillVerticesWithVertexNormals(*verticesWithVertexNormals);
        }
    } catch (...) {
        return false;
    }

    return true;
}

bool loadModel2(
        std::unique_ptr<std::streambuf> const &modelStreamBuf,
        std::pair<std::vector<Vertex>, std::vector<uint32_t>> &verticesWithFaceNormals,
        std::pair<std::vector<Vertex>, std::vector<uint32_t>> *verticesWithVertexNormals)
{
    std::istream assetIstream(modelStreamBuf.get());

    try {
        LoadModelSaxClass2 sax;

        nlohmann::json j = nlohmann::json::sax_parse(assetIstream, &sax, nlohmann::json::input_format_t::cbor);
        sax.getVertices(verticesWithFaceNormals, verticesWithVertexNormals);
    } catch (...) {
        return false;
    }

    return true;
}

// creates a quad with each side length 2.0f.
void getQuad(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices) {
    Vertex vertex = {};
    vertex.color = {0.2f, 0.2f, 0.2f };
    vertex.normal = {0.0f, 0.0f, 1.0f };

    vertex.pos = { -1.0f, 1.0f, 0.0f };
    vertex.texCoord = {0.0f, 0.0f };
    vertices.push_back(vertex);

    vertex.pos = { -1.0f, -1.0f, 0.0f };
    vertex.texCoord = {0.0f, 1.0f };
    vertices.push_back(vertex);

    vertex.pos = { 1.0f, -1.0f, 0.0f };
    vertex.texCoord = {1.0f, 1.0f };
    vertices.push_back(vertex);

    vertex.pos = { 1.0f, 1.0f, 0.0f };
    vertex.texCoord = {1.0f, 0.0f };
    vertices.push_back(vertex);

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);

    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(3);

}

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

void loadModelFromObj2(
        std::unique_ptr<std::streambuf> const &modelStreamBuf,
        std::pair<std::vector<Vertex>, std::vector<uint32_t>> &verticesWithFaceNormals,
        std::pair<std::vector<Vertex>, std::vector<uint32_t>> *verticesWithVertexNormals)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    std::istream assetIstream(modelStreamBuf.get());
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, &assetIstream)) {
        throw std::runtime_error(err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices = {};
    std::unordered_map<Vertex, uint32_t> uniqueVerticesWithVertexNormals = {};
    for (const auto& shape : shapes) {
        std::unordered_map<glm::vec3, glm::vec3> vertexNormals;
        if (verticesWithVertexNormals) {
            for (const auto &index : shape.mesh.indices) {
                glm::vec3 pos = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                };

                glm::vec3 normal = {attrib.normals[3 * index.normal_index + 0],
                                    attrib.normals[3 * index.normal_index + 1],
                                    attrib.normals[3 * index.normal_index + 2]};

                auto it = vertexNormals.find(pos);
                if (it == vertexNormals.end()) {
                    vertexNormals.insert(std::make_pair(pos, normal));
                } else {
                    it->second += normal;
                }
            }

            for (auto &item: vertexNormals) {
                item.second = glm::normalize(item.second);
            }
        }

        for (const auto& index : shape.mesh.indices) {
            Vertex vertex = {};
            vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
            };

            if (attrib.texcoords.size() > 0) {
                vertex.texCoord = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
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
                if (uniqueVerticesWithVertexNormals.count(vertexWithVertexNormal) == 0) {
                    uniqueVerticesWithVertexNormals[vertexWithVertexNormal] = static_cast<uint32_t>(verticesWithVertexNormals->first.size());
                    verticesWithVertexNormals->first.push_back(vertexWithVertexNormal);
                }

                verticesWithVertexNormals->second.push_back(uniqueVerticesWithVertexNormals[vertexWithVertexNormal]);
            }

            vertex.normal = {attrib.normals[3 * index.normal_index + 0],
                             attrib.normals[3 * index.normal_index + 1],
                             attrib.normals[3 * index.normal_index + 2]};

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(verticesWithFaceNormals.first.size());
                verticesWithFaceNormals.first.push_back(vertex);
            }

            verticesWithFaceNormals.second.push_back(uniqueVertices[vertex]);
        }
    }
}

void loadModelFromObj3(
        std::unique_ptr<std::streambuf> const &modelStreamBuf,
        std::pair<std::vector<Vertex>, std::vector<uint32_t>> &,
        std::pair<std::vector<Vertex>, std::vector<uint32_t>> *)
{
    std::istream assetIstream(modelStreamBuf.get());

    std::vector<char> data;
    data.resize(1024);
    while (assetIstream.good()) {
        assetIstream.read(data.data(), data.size());
    }
}