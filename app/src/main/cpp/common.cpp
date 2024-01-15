/**
 * Copyright 2024 Cerulean Quasar. All Rights Reserved.
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
#include <iostream>
#include "common.hpp"

std::vector<char> readFile(std::shared_ptr<FileRequester> const &requester, std::string const &filename) {
    std::unique_ptr<std::streambuf> assetStreambuf = requester->getAssetStream(filename);
    std::istream reader(assetStreambuf.get());
    std::vector<char> data;
    unsigned long const readSize = 1024;

    while (!reader.eof()) {
        unsigned long size = data.size();
        data.resize(size + readSize);
        long bytesRead = reader.read(data.data()+size, readSize).gcount();

        if (bytesRead != readSize) {
            data.resize(size + bytesRead);
        }
    }

    return data;
}