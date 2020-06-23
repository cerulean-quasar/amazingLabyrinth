#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>
#include <memory>
#include <iostream>

#include <json.hpp>

void usage(char const *progName) {
    std::cerr << "Usage : " << progName << " infilename outfilename" << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        usage(argv[0]);
        return 1;
    }

    char const *filename = argv[1];
    char const *outfilename = argv[2];

    std::ifstream jsonStream(filename);
    std::ofstream cborStream(outfilename);

    if (jsonStream.good() && cborStream.good()) {
        nlohmann::json j;
        jsonStream >> j;
        jsonStream.close();
        std::vector<uint8_t> data = nlohmann::json::to_cbor(j);
        cborStream.write(reinterpret_cast<char const *>(data.data()), data.size());
        cborStream.close();
        return 0;
    } else {
        return 1;
    }
}
