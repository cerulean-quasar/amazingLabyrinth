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
#ifndef AMAZING_LABYRINTH_GRAPHICS_HPP
#define AMAZING_LABYRINTH_GRAPHICS_HPP
#include <set>
#include <vector>

#include "android.hpp"

int istreamRead(void *userData, char *data, int size);
void istreamSkip(void *userData, int n);
int istreamEof(void *userData);

extern std::string const TEXTURE_PATH_WALLS;
extern std::string const TEXTURE_PATH_FLOOR;
extern std::string const TEXTURE_PATH_BALL;
extern std::string const TEXTURE_PATH_HOLE;
extern unsigned int const MAZE_COLS;
extern unsigned int const MAZE_ROWS;

class Graphics {
public:
    virtual void init(WindowType *window) = 0;

    virtual void initThread()=0;

    virtual void cleanup()=0;

    virtual void updateAcceleration(float x, float y, float z)=0;

    virtual void drawFrame()=0;

    virtual bool updateData()=0;

    virtual void destroyWindow()=0;

    virtual void recreateSwapChain()=0;

    virtual void cleanupThread()=0;

    virtual ~Graphics() { }
};
#endif