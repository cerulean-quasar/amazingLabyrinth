#include <stdexcept>
/**
 * Copyright 2019 Cerulean Quasar. All Rights Reserved.
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
#include <limits>
#include <unistd.h>
#include <fcntl.h>

#include "random.hpp"

Random::~Random() {
    if (fd != -1) {
        close(fd);
    }
}

template<typename T> T Random::get() {
    if (fd == -1) {
        fd = open("/dev/urandom", O_RDONLY);
    }

    if (fd == -1) {
        throw std::runtime_error("Could not open /dev/urandom");
    }

    T random;

    ssize_t readlen = read(fd, &random, sizeof (random));

    if (readlen != sizeof (random)) {
        throw std::runtime_error("Could not read enough random data");
    }

    return random;
}

unsigned int Random::getUInt(unsigned int lowerBound, unsigned int upperBound) {
    unsigned int range = upperBound - lowerBound + 1;
    unsigned int too_big = std::numeric_limits<unsigned int>::max() / range * range;

    unsigned int random;

    do {
        random = get<unsigned int>();
    } while (random >= too_big);

    return random % range + lowerBound;
}

float Random::getFloat(float lowerBound, float upperBound) {
    float range = upperBound - lowerBound;
    if (range <= 0) {
        throw std::runtime_error("Could not read enough random data.");
    }

    auto random = get<unsigned int>();
    double rc = random;
    rc = rc / std::numeric_limits<unsigned int>::max() * range + lowerBound;
    return static_cast<float>(rc);
}
