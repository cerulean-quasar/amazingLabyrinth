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
#include <memory>

#include "../basic/renderDetailsCommonGL.hpp"
#include "renderDetailsDataGL.hpp"

namespace textureWithShadows {
    RenderDetailsDataGL::RenderDetailsDataGL(std::shared_ptr<GameRequester> const &inGameRequester,
                                             uint32_t inWidth, uint32_t inHeight)
            : RenderDetailsData(inWidth, inHeight),
              m_mainProgramID{loadShaders(inGameRequester, SHADER_VERT_FILE, SHADER_FRAG_FILE)},
              m_depthProgramID{loadShaders(inGameRequester, DEPTH_VERT_FILE, SIMPLE_FRAG_FILE)} {}

}