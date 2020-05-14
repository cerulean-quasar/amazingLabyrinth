#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>
#include <memory>
#include <iostream>

#include <json.hpp>

void usage(char const *progName) {
    std::cerr << "Usage : " << progName << "filename" << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        usage(argv[0]);
        return 1;
    }

    char const *filename = argv[1];

    std::ifstream cborStream(filename, std::ifstream::binary);

    if (cborStream.good()) {
        cborStream.seekg(0, cborStream.end);
        size_t i = static_cast<size_t >(cborStream.tellg());
        cborStream.seekg(0, cborStream.beg);
        std::vector<uint8_t> vec;
        vec.resize(i);
        cborStream.read(reinterpret_cast<char *>(vec.data()), vec.size());

        if (!cborStream.fail()) {
            nlohmann::json j = nlohmann::json::from_cbor(vec.data(), vec.size());
            std::cout << j << std::endl;
        }
    }
}
