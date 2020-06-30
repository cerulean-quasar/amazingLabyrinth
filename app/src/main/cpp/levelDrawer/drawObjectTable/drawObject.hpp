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
#ifndef AMAZING_LABYRINTH_DRAW_OBJECT_HPP
#define AMAZING_LABYRINTH_DRAW_OBJECT_HPP

#include <momory>
#include <glm/glm.h>
#include <boost/optional.hpp>
#include "../textureTable/textureLoader.hpp"

class DrawObject {
    boost::optional<size_t> renderDetailsIndex() { return m_renderDetailsIndex; }
    size_t modelIndex() { return m_modelIndex; }
    boost::optional<size_t> textureIndex() { return m_textureIndex; }

    void addLocation(std::shared_ptr<DrawObjectData> location) {
        m_locations.push_back(location);
    }

    DrawObject(size_t renderDetailsIndex_, size_t modelIndex_, size_t textureIndex_)
        : m_renderDetailsIndex{renderDetailsIndex_},
          m_modelIndex{modelIndex_},
          m_textureIndex{textureIndex_}
    {}
private:
    boost::optional<size_t> m_renderDetailsIndex;
    size_t m_modelIndex;
    boost::optional<size_t> m_textureIndex;
    std::vector<std::shared_ptr<DrawObjectData>> m_locations;
};

#endif // AMAZING_LABYRINTH_DRAW_OBJECT_HPP