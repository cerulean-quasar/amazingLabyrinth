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
#ifndef AMAZING_LABYRINTH_LEVEL_DRAWER_COMMON_HPP
#define AMAZING_LABYRINTH_LEVEL_DRAWER_COMMON_HPP

#include <memory>

template <typename BaseClass>
class BaseClassPtrLess {
public:
    bool operator() (std::shared_ptr<BaseClass> const &p1,
                     std::shared_ptr<BaseClass> const &p2) const {
        BaseClass &p1ref = *p1;
        BaseClass &p2ref = *p2;
        std::type_info const &c1 = typeid(p1ref);
        std::type_info const &c2 = typeid(p2ref);
        if (c1 != c2) {
            return c1.before(c2);
        } else if (p1.get() == p2.get()) {
            return false;
        } else {
            return p1->compareLess(p2.get());
        }
    }
};

#endif // AMAZING_LABYRINTH_LEVEL_DRAWER_COMMON_HPP