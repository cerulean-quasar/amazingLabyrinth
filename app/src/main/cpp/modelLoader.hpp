#include <vector>
#include <map>
#include <string>

#include <glm/glm.hpp>

char constexpr const *KeyVerticesWithVertexNormals = "VV";
char constexpr const *KeyIndicesForVertexNormals = "IV";
char constexpr const *KeyVerticesWithFaceNormals = "VF";
char constexpr const *KeyIndicesForFaceNormals = "IF";

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
    glm::vec3 normal;

    bool operator==(const Vertex& other) const;
};

// creates a quad with each side length 2.0f.
void getQuad(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);

bool loadModel(
        std::unique_ptr<std::streambuf> const &modelStreamBuf,
        std::pair<std::vector<Vertex>, std::vector<uint32_t>> &verticesWithFaceNormals,
        std::pair<std::vector<Vertex>, std::vector<uint32_t>> *verticesWithVertexNormals = nullptr);

