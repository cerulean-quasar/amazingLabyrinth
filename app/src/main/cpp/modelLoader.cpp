#include <streambuf>

#include <json.hpp>

#include "modelLoader.hpp"

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

bool loadModel(
    std::unique_ptr<std::streambuf> const &modelStreamBuf,
    std::pair<std::vector<Vertex>, std::vector<uint32_t>> &verticesWithFaceNormals,
    std::pair<std::vector<Vertex>, std::vector<uint32_t>> *verticesWithVertexNormals)
{
    size_t constexpr block_size = 1024;

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