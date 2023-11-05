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

#include <modelLoader.hpp>

void loadModelFromObj(
    std::ifstream &modelStream,
    std::vector<float> &vertices,
    std::vector<float> &faceNormals,
    std::vector<float> &vertexNormals,
    std::vector<float> &texcoords,
    std::vector<float> &colors,
    std::vector<std::vector<uint32_t>> &indices);

void loadModelFromGlb(
        std::ifstream &modelStream,
        std::vector<float> &vertices,
        std::vector<float> &faceNormals,
        std::vector<float> &vertexNormals,
        std::vector<float> &texcoords,
        std::vector<float> &colors,
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
        size_t strlen = std::string::npos;
        if (dotpos != std::string::npos) {
            if (dotpos > slashpos) {
                strlen = dotpos - slashpos;
            }
        }
        std::string outfilename(outputDir + filename.substr(slashpos, strlen) + ".modelcbor");
        std::string inType = filename.substr(dotpos+1);
        try {
            std::ifstream fileStream(filename);
            if (!fileStream.good()) {
                std::cerr << "Could not open file for read: " << filename << std::endl;
                return 1;
            }
            std::vector<float> v, nf, nv, tx, c;
            std::vector<std::vector<uint32_t>> in;
            if (inType == "glb") {
                loadModelFromGlb(fileStream, v, nf, nv, tx, c, in);
            } else if (inType == "obj"){
                loadModelFromObj(fileStream, v, nf, nv, tx, c, in);
            } else {
                std::cerr << "Unrecognizable file type: " << inType;
                return 1;
            }

            std::cout << "number vertices: " << v.size() / 3 << std::endl;
            std::cout << "number vertex normals: " << nv.size() / 3 << std::endl;
            std::cout << "number face normals: " << nf.size() / 3 << std::endl;
            std::cout << "number texture coordinates: " << tx.size() / 2 << std::endl;
            std::cout << "number colors: " << c.size() / 4 << std::endl;

            size_t nbrIndices = 0;
            for (auto const &indices : in) {
                nbrIndices += indices.size();
            }
            size_t nbrTypes = 0;
            nbrTypes += v.empty() ? 0 : 1;
            nbrTypes += nv.empty() ? 0 : 1;
            nbrTypes += nf.empty() ? 0 : 1;
            nbrTypes += tx.empty() ? 0 : 1;
            nbrTypes += c.empty() ? 0 : 1;
            std::cout << "number indices: " << nbrIndices/nbrTypes << std::endl;

            std::ofstream outStream(outfilename, std::ofstream::binary);
            if (!outStream.good()) {
                std::cerr << "Could not open file for write: " << outfilename << std::endl;
                return 1;
            }

            if (jsonForCpp) {
                nlohmann::json j;
                j[levelDrawer::KeyVertices] = v;
                j[levelDrawer::KeyFaceNormals] = nf;
                j[levelDrawer::KeyVertexNormals] = nv;
                if (!tx.empty()) {
                    j[levelDrawer::KeyTexCoords] = tx;
                }
                if (!c.empty()) {
                    j[levelDrawer::KeyColors] = c;
                }
                j[levelDrawer::KeyIndices] = in;

                std::vector<uint8_t> data = nlohmann::json::to_cbor(j);

                outStream.write(reinterpret_cast<char*>(data.data()), data.size());
            } else {
                size_t len = 5;
                if (!tx.empty()) {
                    len++;
                }
                if (!c.empty()) {
                    len++;
                }
                cbor_item_t *cmap = cbor_new_definite_map(len);

                cbor_item_t *array1 = cbor_new_definite_array(v.size());
                for (auto f : v) {
                    cbor_array_push(array1, cbor_build_float4(f));
                }
                cbor_map_add(cmap, (struct cbor_pair) {
                    .key = cbor_move(cbor_build_string(levelDrawer::KeyVertices)),
                    .value = cbor_move(array1)});

                cbor_item_t *array2 = cbor_new_definite_array(nf.size());
                for (auto f : nf) {
                    cbor_array_push(array2, cbor_build_float4(f));
                }
                cbor_map_add(cmap, (struct cbor_pair) {
                    .key = cbor_move(cbor_build_string(levelDrawer::KeyFaceNormals)),
                    .value = cbor_move(array2)});
 
                cbor_item_t *array3 = cbor_new_definite_array(nv.size());
                for (auto f : nv) {
                    cbor_array_push(array3, cbor_build_float4(f));
                }
                cbor_map_add(cmap, (struct cbor_pair) {
                    .key = cbor_move(cbor_build_string(levelDrawer::KeyVertexNormals)),
                    .value = cbor_move(array3)});

                if (!tx.empty()) {
                    cbor_item_t *array4 = cbor_new_definite_array(tx.size());
                    for (auto f : tx) {
                        cbor_array_push(array4, cbor_build_float4(f));
                    }
                    cbor_map_add(cmap, (struct cbor_pair) {
                        .key = cbor_move(cbor_build_string(levelDrawer::KeyTexCoords)),
                        .value = cbor_move(array4)});
                }

                if (!c.empty()) {
                    cbor_item_t *array5 = cbor_new_definite_array(c.size());
                    for (auto f : c) {
                        cbor_array_push(array5, cbor_build_float4(f));
                    }
                    cbor_map_add(cmap, (struct cbor_pair) {
                        .key = cbor_move(cbor_build_string(levelDrawer::KeyColors)),
                        .value = cbor_move(array5)});
                }

                cbor_item_t *array6 = cbor_new_definite_array(in.size());
                for (auto const &indices : in) {
                    cbor_item_t *array6a = cbor_new_definite_array(indices.size());
                    for (auto const &i : indices) {
                        cbor_array_push(array6a, cbor_build_uint32(i));
                    }
                    cbor_array_push(array6, cbor_move(array6a));
                }
                cbor_map_add(cmap, (struct cbor_pair) {
                    .key = cbor_move(cbor_build_string(levelDrawer::KeyIndices)),
                    .value = cbor_move(array6)});

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
        } catch (nlohmann::json::exception const &e) {
            std::cerr << "a JSON error occurred while creating file: "
                      << outfilename << " from " << filename << " error: " << e.what() << std::endl;
        } catch (std::exception const &e) {
            std::cerr << "error occurred while creating file: "
                      << outfilename << " from " << filename << " error: " << e.what() << std::endl;
            return 1;
        } catch (...) {
            std::cerr << "error occurred while creating file: "
                      << outfilename << " from " << filename << std::endl;
        }
    }
}
