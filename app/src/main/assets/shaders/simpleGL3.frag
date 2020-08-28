#version 300 es
precision highp float;
precision highp int;

/**
 * Copyright 2020 Cerulean Quasar. All Rights Reserved.
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

in vec3 fragColor;
layout(location = 0) out uvec4 fragColorOut;

void main() {
    float maxUint = 4294967295.0;
    fragColorOut = uvec4(
        //25040000,
        uint(fragColor.r * maxUint),
        uint(fragColor.g * maxUint),
        uint(fragColor.b * maxUint),
        uint(maxUint));
}
