/**
 * Copyright 2018 Cerulean Quasar. All Rights Reserved.
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

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/transform.hpp>

#include "level.hpp"

void Level::updatePerspectiveMatrix(int surfaceWidth, int surfaceHeight) {
    /* perspective matrix: takes the perspective projection, the aspect ratio, near and far
     * view planes.
     */
    proj = glm::perspective(glm::radians(45.0f), surfaceWidth / (float) surfaceHeight, 0.1f, 100.0f);
}

void Level::setViewLightingSource() {
    viewLightingSource = glm::lookAt(lightingSource, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

void Level::setView() {
    /* glm::lookAt takes the eye position, the center position, and the up axis as parameters */
    view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

void Level::setLightingSource() {
    lightingSource = glm::vec3(0.0f, 0.0f, 1.28f/*0.01-1.28*/);
}

void Level::init(uint32_t width, uint32_t height) {
    setLightingSource();
    setViewLightingSource();
    loadModels();
    setView();
    updatePerspectiveMatrix(width, height);
    generate();
    generateModelMatrices();
}