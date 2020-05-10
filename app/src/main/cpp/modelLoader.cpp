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

bool loadModel(
    std::unique_ptr<std::streambuf> const &modelStreamBuf,
    std::pair<std::vector<Vertex>, std::vector<uint32_t>> &verticesWithFaceNormals,
    std::pair<std::vector<Vertex>, std::vector<uint32_t>> *verticesWithVertexNormals)
{
    size_t constexpr block_size = 1024;

    std::istream assetIstream(modelStreamBuf.get());
    std::vector<uint8_t> vec;
    vec.resize(block_size);
    std::vector<uint8_t> data;
    while (assetIstream) {
        assetIstream.read(reinterpret_cast<char *>(vec.data()), vec.size());
        int readlen = assetIstream.gcount();
        if (readlen > 0) {
            data.resize(data.size() + readlen);
            memcpy(data.data() + data.size() - readlen, vec.data(), static_cast<size_t>(readlen));
        }
    }

    try {
        nlohmann::json j = nlohmann::json::from_cbor(data);
        if (verticesWithVertexNormals) {
            (*verticesWithVertexNormals).first = j[KeyVerticesWithVertexNormals].get<std::vector<Vertex>>();
            (*verticesWithVertexNormals).second = j[KeyIndicesForVertexNormals].get<std::vector<uint32_t>>();
        }

        verticesWithFaceNormals.first = j[KeyVerticesWithFaceNormals].get<std::vector<Vertex>>();
        verticesWithFaceNormals.second = j[KeyIndicesForFaceNormals].get<std::vector<uint32_t>>();
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