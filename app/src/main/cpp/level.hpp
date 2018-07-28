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

#ifndef AMAZING_LABYRINTH_LEVEL_HPP
#define AMAZING_LABYRINTH_LEVEL_HPP

#include "levelFinish.hpp"

class Level {
protected:
    bool finished;
    glm::mat4 proj;
    glm::mat4 view;
    glm::mat4 viewLightingSource;
    glm::vec3 lightingSource;
public:
    virtual void loadModels() = 0;
    virtual void generate() = 0;
    virtual glm::vec4 getBackgroundColor() = 0;
    virtual void updateAcceleration(float x, float y, float z) = 0;
    virtual bool updateData() = 0;
    virtual void generateModelMatrices() = 0;
    virtual bool updateStaticDrawObjects(DrawObjectTable &objs) = 0;
    virtual bool updateDynamicDrawObjects(DrawObjectTable &objs) = 0;
    virtual void setView();
    virtual void updatePerspectiveMatrix(int surfaceWidth, int surfaceHeight);
    virtual void setLightingSource();
    virtual void setViewLightingSource();

    void init(uint32_t width, uint32_t height);
    bool isFinished() { return finished; }
    glm::mat4 getProjectionMatrix() { return proj; }
    glm::mat4 getViewMatrix() { return view; }
    glm::vec3 getLightingSource() { return lightingSource; }
    glm::mat4 getViewLightSource() { return viewLightingSource; }

    Level() : finished(false) { }
    virtual ~Level() {}

};
#endif