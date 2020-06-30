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
#ifndef AMAZING_LABYRINTH_RENDER_DETAILS_TABLE_HPP
#define AMAZING_LABYRINTH_RENDER_DETAILS_TABLE_HPP

#include <memory>
#include <vector>
#include <map>

#include "../common.hpp"

#include "../../renderDetails/basic/renderDetailsData.hpp"

class RenderDetails {
    friend BaseClassPtrLess<RenderDetails>;
protected:
    // returns true if this < other.
    virtual bool compareLess(RenderDetails *) { return false; }
};

class RenderDetailsTable {
public:
    explicit RenderDetailsTable(std::shared_ptr<RenderLoader> inLoader)
            : m_loader{std::move(inLoader)},
            m_renderDetailsIndexMap{},
            m_renderDetailsData{}
    {}

    virtual ~RenderDetailsTable() = default;
protected:
    std::shared_ptr<RenderLoader> m_loader;
    std::map<std::shared_ptr<RenderDetails>, size_t> m_renderDetailsIndexMap;
    std::vector<std::shared_ptr<RenderDetailsData>> m_renderDetailsDataVector;
};

#endif // AMAZING_LABYRINTH_RENDER_DETAILS_TABLE_HPP