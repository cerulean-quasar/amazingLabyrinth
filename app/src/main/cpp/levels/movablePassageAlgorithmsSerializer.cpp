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
#include <json.hpp>
#include <boost/implicit_cast.hpp>
#include "../levelTracker/internals.hpp"
#include "basic/serializer.hpp"

#include "movablePassageAlgorithmsSerializer.hpp"

char constexpr const *LockedInPlaceRef = "LockedInPlaceRef";
char constexpr const *TextureIndex = "TextureIndex";
char constexpr const *ModelIndex = "ModelIndex";
void to_json(nlohmann::json &j, ObjReference const &val) {
    j[LockedInPlaceRef] = val.isLockedInPlaceRef;
    j[TextureIndex] = val.textureIndex;
    j[ModelIndex] = val.modelIndex;
}

void from_json(nlohmann::json const &j, ObjReference &val) {
    val.objIndex = boost::none;
    val.isLockedInPlaceRef = j[LockedInPlaceRef].get<bool>();
    val.modelIndex = j[ModelIndex].get<size_t>();
    val.textureIndex = j[TextureIndex].get<size_t>();
}

std::vector<Point<uint32_t>> pathLockedInPlace(GameBoard &gameBoard, size_t startRow, size_t startCol) {
    // the path the ball has traveled in user placeable components.
    bool done = false;
    Point<uint32_t> startRC{static_cast<uint32_t>(startRow), static_cast<uint32_t>(startCol)};
    std::vector<Point<uint32_t>> ret;
    do {
        auto b = gameBoard.block(startRC.x, startRC.y);
        auto &placement = b.component()->placement(b.placementIndex());
        if (placement.prev().first != nullptr || placement.next().first != nullptr) {
            ret.emplace_back(startRC);
        } else {
            done = true;
        }
        if (placement.next().first != nullptr) {
            auto &nextPlacement = placement.next().first->placement(placement.next().second);
            startRC.x = nextPlacement.row();
            startRC.y = nextPlacement.col();
        } else {
            done = true;
        }
    } while (!done);

    return ret;
}
