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

void checkGraphicsError() {
    GLenum rc = glGetError();
    if (rc != GL_NO_ERROR) {
        std::string c;
        switch (rc) {
            case GL_INVALID_ENUM:
                c = "GL_INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                c = "GL_INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                c = "GL_INVALID_OPERATION";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                c = "GL_INVALID_FRAMEBUFFER_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                c = "GL_OUT_OF_MEMORY";
                break;
            default:
                c = "Unknown return code.";
        }
        throw std::runtime_error(std::string("A graphics error occurred: ") + c);
    }
}
