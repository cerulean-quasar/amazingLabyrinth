#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <iomanip>

#include <glm/glm.hpp>

#include <json.hpp>

#include <cbor.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <graphics.hpp>

void to_json(nlohmann::json &j, Vertex const &v);

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

void loadModelFromObj(
    std::ifstream &modelStreamBuf,
    std::pair<std::vector<Vertex>, std::vector<uint32_t>> &verticesWithFaceNormals,
    std::pair<std::vector<Vertex>, std::vector<uint32_t>> *verticesWithVertexNormals);

void usage(char const *progName) {
    std::cerr << "Usage : " << progName << "[args] filename [filenames]" << std::endl
              << " -d <output Directory>" << std::endl
              << " -n (include vertex normals)" << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    std::vector<std::string> filenames;
    std::string outputDir;
    bool includeVertexNormals = false;
    bool jsonForCpp = false;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch(argv[i][1]) {
            case 'n':
                includeVertexNormals = true;
                continue;
            case 'd':
                outputDir = argv[++i];
                continue;
            case 'j':
                jsonForCpp = true;
                continue;
            default:
                usage(argv[0]);
                return 1;
            }
        }

        filenames.emplace_back(argv[i]);
    }

    for (auto &filename : filenames) {
        std::pair<std::vector<Vertex>, std::vector<uint32_t>> verticesWithFaceNormals;
        std::pair<std::vector<Vertex>, std::vector<uint32_t>> verticesWithVertexNormals;
        std::pair<std::vector<Vertex>, std::vector<uint32_t>> *v = nullptr;
        if (includeVertexNormals) {
            v = &verticesWithVertexNormals;
        }

        std::ifstream fileStream(filename);
        if (!fileStream.good()) {
            std::cerr << "Could not open file for read: " << filename << std::endl;
            return 1;
        }
        loadModelFromObj(fileStream, verticesWithFaceNormals, v);

        size_t slashpos = filename.find_last_of('/');
        if (slashpos == std::string::npos) {
            slashpos = 0;
        } else {
            slashpos ++;
        }
        size_t dotpos = filename.find_last_of('.');
        size_t len = std::string::npos;
        if (dotpos != std::string::npos) {
            if (dotpos > slashpos) {
                len = dotpos - slashpos;
            }
        }
        std::string outfilename(outputDir + filename.substr(slashpos, len) + ".modelcbor");
        std::vector<float> fn;
        for (auto &vertex : verticesWithFaceNormals.first) {
            fn.push_back(vertex.pos.x);
            fn.push_back(vertex.pos.y);
            fn.push_back(vertex.pos.z);

            fn.push_back(vertex.normal.x);
            fn.push_back(vertex.normal.y);
            fn.push_back(vertex.normal.z);

            fn.push_back(vertex.texCoord.x);
            fn.push_back(vertex.texCoord.y);
        }

        std::vector<float> vn;
        if (includeVertexNormals) {
            for (auto &vertex : verticesWithVertexNormals.first) {
                vn.push_back(vertex.pos.x);
                vn.push_back(vertex.pos.y);
                vn.push_back(vertex.pos.z);

                vn.push_back(vertex.normal.x);
                vn.push_back(vertex.normal.y);
                vn.push_back(vertex.normal.z);
            }
        }

        try {
            std::ofstream outStream(outfilename, std::ofstream::binary);
            if (!outStream.good()) {
                std::cerr << "Could not open file for write: " << outfilename << std::endl;
                return 1;
            }

            if (jsonForCpp) {
                nlohmann::json j;
                j[KeyVerticesWithFaceNormals] = fn;
                j[KeyIndicesForFaceNormals] = verticesWithFaceNormals.second;

                if (includeVertexNormals) {
                    j[KeyVerticesWithVertexNormals] = vn;
                    j[KeyIndicesForVertexNormals] = verticesWithVertexNormals.second;
                }

                std::vector<uint8_t> data = nlohmann::json::to_cbor(j);

                outStream.write(reinterpret_cast<char*>(data.data()), data.size());
            } else {
                size_t len = 2;
                if (includeVertexNormals) {
                    len = 4;
                }
                cbor_item_t *cmap = cbor_new_definite_map(len);
                cbor_item_t *array = cbor_new_definite_array(fn.size());
                for (auto f : fn) {
                    cbor_array_push(array, cbor_build_float4(f));
                }
                cbor_map_add(cmap, (struct cbor_pair) {
                    .key = cbor_move(cbor_build_string(KeyVerticesWithFaceNormals)),
                    .value = cbor_move(array)});

                cbor_item_t *array2 = cbor_new_definite_array(verticesWithFaceNormals.second.size());
                for (auto i : verticesWithFaceNormals.second) {
                    cbor_array_push(array2, cbor_build_uint32(i));
                }
                cbor_map_add(cmap, (struct cbor_pair) {
                    .key = cbor_move(cbor_build_string(KeyIndicesForFaceNormals)),
                    .value = cbor_move(array2)});

                if (includeVertexNormals) {
                    cbor_item_t *array3 = cbor_new_definite_array(vn.size());
                    for (auto f : vn) {
                        cbor_array_push(array3, cbor_build_float4(f));
                    }
                    cbor_map_add(cmap, (struct cbor_pair) {
                        .key = cbor_move(cbor_build_string(KeyVerticesWithVertexNormals)),
                        .value = cbor_move(array3)});

                    cbor_item_t *array4 = cbor_new_definite_array(verticesWithVertexNormals.second.size());
                    for (auto i : verticesWithVertexNormals.second) {
                        cbor_array_push(array4, cbor_build_uint32(i));
                    }
                    cbor_map_add(cmap, (struct cbor_pair) {
                        .key = cbor_move(cbor_build_string(KeyIndicesForVertexNormals)),
                        .value = cbor_move(array4)});
                }
                size_t buffer_size = 0;
                unsigned char *buffer = nullptr;

                size_t lenwritten;
                lenwritten = cbor_serialize_alloc(cmap, &buffer, &buffer_size);
                if (lenwritten == 0) {
                    std::cout << "Failed to encode to cbor" << std::endl;
                    return 1;
                }
                outStream.write(reinterpret_cast<char*>(buffer), lenwritten);

                free(buffer);
                cbor_decref(&cmap);
            }
        } catch (...) {
            std::cerr << "JSON error occurred while creating file: "
                      << outfilename << " from " << filename << std::endl;
            return 1;
        }
    }
}

void loadModelFromObj(
    std::ifstream &modelStream,
    std::pair<std::vector<Vertex>, std::vector<uint32_t>> &verticesWithFaceNormals,
    std::pair<std::vector<Vertex>, std::vector<uint32_t>> *verticesWithVertexNormals)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, &modelStream)) {
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

