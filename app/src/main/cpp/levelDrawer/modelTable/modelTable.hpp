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
#ifndef AMAZING_LABYRINTH_MODEL_TABLE_HPP
#define AMAZING_LABYRINTH_MODEL_TABLE_HPP

#include <vector>
#include <map>
#include <memory>

#include "../../common.hpp"
#include "../common.hpp"
#include "modelLoader.hpp"

namespace levelDrawer {
    template <typename ModelDataType>
    class ModelTable {
    public:
        std::shared_ptr <ModelDataType> const &
        addModel(std::shared_ptr <GameRequester> const &gameRequester,
                 std::shared_ptr <ModelDescription> const &modelDescription)
        {
            std::shared_ptr<ModelDataType> md;

            auto item = m_modelMap.emplace(modelDescription, std::weak_ptr<ModelDataType>());
            if (item.second || item.first->second.expired()) {
                md = getModelData(gameRequester, modelDescription);
                item.first->second = md;
            } else {
                md = item.first->second.lock();
            }
            return std::move(md);
        }

        void prune() {
            for (auto it = m_modelMap.begin(); it != m_modelMap.end(); it++) {
                if (it->second.expired()) {
                    it = m_modelMap.erase(it);
                }
            }
        }

        ModelTable() = default;

        virtual ~ModelTable() = default;

    private:
        virtual std::shared_ptr <ModelDataType>
        getModelData(std::shared_ptr <GameRequester> const &gameRequester,
                     std::shared_ptr <ModelDescription> const &modelDescription) = 0;

        std::map <std::shared_ptr<ModelDescription>, std::weak_ptr<ModelDataType>, BaseClassPtrLess<ModelDescription>> m_modelMap;
    };
}

#endif // AMAZING_LABYRINTH_MODEL_TABLE_HPP
