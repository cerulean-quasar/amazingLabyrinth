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

#include <modelLoader.hpp>

void loadModelFromObj(
    std::ifstream &modelStream,
    std::vector<float> &vertices,
    std::vector<float> &normals,
    std::vector<float> &texcoords,
    std::vector<std::vector<uint32_t>> &indices);

void usage(char const *progName) {
    std::cerr << "Usage : " << progName << "[args] filename [filenames]" << std::endl
              << " -d <output Directory>" << std::endl
              << " -j (use json for modern cpp to produce cbor)" << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    std::vector<std::string> filenames;
    std::string outputDir;
    bool jsonForCpp = false;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch(argv[i][1]) {
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
        try {
            std::ifstream fileStream(filename);
            if (!fileStream.good()) {
                std::cerr << "Could not open file for read: " << filename << std::endl;
                return 1;
            }
            std::vector<float> v, n, tx;
            std::vector<std::vector<uint32_t>> in;
            loadModelFromObj(fileStream, v, n, tx, in);


            std::ofstream outStream(outfilename, std::ofstream::binary);
            if (!outStream.good()) {
                std::cerr << "Could not open file for write: " << outfilename << std::endl;
                return 1;
            }

            if (jsonForCpp) {
                nlohmann::json j;
                j[levelDrawer::KeyVertices] = v;
                j[levelDrawer::KeyNormals] = n;
                j[levelDrawer::KeyTexCoords] = tx;
                j[levelDrawer::KeyIndices] = in;

                std::vector<uint8_t> data = nlohmann::json::to_cbor(j);

                outStream.write(reinterpret_cast<char*>(data.data()), data.size());
            } else {
                size_t len = 4;
                cbor_item_t *cmap = cbor_new_definite_map(len);
                cbor_item_t *array = cbor_new_definite_array(v.size());
                for (auto f : v) {
                    cbor_array_push(array, cbor_build_float4(f));
                }
                cbor_map_add(cmap, (struct cbor_pair) {
                    .key = cbor_move(cbor_build_string(levelDrawer::KeyVertices)),
                    .value = cbor_move(array)});

                cbor_item_t *array2 = cbor_new_definite_array(n.size());
                for (auto f : n) {
                    cbor_array_push(array2, cbor_build_float4(f));
                }
                cbor_map_add(cmap, (struct cbor_pair) {
                    .key = cbor_move(cbor_build_string(levelDrawer::KeyNormals)),
                    .value = cbor_move(array2)});

                cbor_item_t *array3 = cbor_new_definite_array(tx.size());
                for (auto f : tx) {
                    cbor_array_push(array3, cbor_build_float4(f));
                }
                cbor_map_add(cmap, (struct cbor_pair) {
                    .key = cbor_move(cbor_build_string(levelDrawer::KeyTexCoords)),
                    .value = cbor_move(array3)});

                cbor_item_t *array4 = cbor_new_definite_array(in.size());
                for (auto indices : in) {
                    cbor_item_t *array4a = cbor_new_definite_array(indices.size());
                    for (auto i : indices) {
                        cbor_array_push(array4a, cbor_build_uint32(i));
                    }
                    cbor_array_push(array4, cbor_move(array4a));
                }
                cbor_map_add(cmap, (struct cbor_pair) {
                    .key = cbor_move(cbor_build_string(levelDrawer::KeyIndices)),
                    .value = cbor_move(array4)});

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
            std::cerr << "error occurred while creating file: "
                      << outfilename << " from " << filename << std::endl;
            return 1;
        }
    }
}

void loadModelFromObj(
    std::ifstream &modelStream,
    std::vector<float> &vertices,
    std::vector<float> &normals,
    std::vector<float> &texcoords,
    std::vector<std::vector<uint32_t>> &indices)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, &modelStream)) {
        throw std::runtime_error(err);
    }

    std::cout << "number vertices: " << attrib.vertices.size() << std::endl;
    std::cout << "number normals: " << attrib.normals.size() << std::endl;
    std::cout << "number texture coordinates: " << attrib.texcoords.size() << std::endl;
    vertices = std::move(attrib.vertices);
    normals = std::move(attrib.normals);
    texcoords = std::move(attrib.texcoords);
    for (auto const & shape : shapes) {
        std::cout << "number indices: " << shape.mesh.indices.size() << std::endl;
        std::vector<uint32_t> ind;
        for (auto const & index : shape.mesh.indices) {
            ind.push_back(index.vertex_index);
            ind.push_back(index.normal_index);
            ind.push_back(index.texcoord_index);
        }
        indices.push_back(std::move(ind));
    }
}

