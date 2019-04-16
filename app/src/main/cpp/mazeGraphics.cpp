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

#include "mazeGraphics.hpp"

void LevelSequence::updatePerspectiveMatrix(uint32_t surfaceWidth, uint32_t surfaceHeight) {
    /* perspective matrix: takes the perspective projection, the aspect ratio, near and far
     * view planes.
     */
    m_proj = getPerspectiveMatrix(surfaceWidth, surfaceHeight);
}

void LevelSequence::setViewLightingSource() {
    m_viewLightingSource = glm::lookAt(m_lightingSource, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

void LevelSequence::setView() {
    /* glm::lookAt takes the eye position, the center position, and the up axis as parameters */
    m_view = getViewMatrix();
}

void LevelSequence::setLightingSource() {
    m_lightingSource = glm::vec3(0.0f, 0.0f, 1.28f/*0.01-1.28*/);
}