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

class ModelTable {
public:
    virtual size_t addModel(std::shared_ptr<GameRequester> const &gameRequester,
                    std::shared_ptr<ModelDescription> const &modelDescription) = 0;

    virtual ~ModelTable() = default;
};

template <typename ModelDataType>
class ModelTableGeneric : public ModelTable {
public:
    std::shared_ptr<ModelDataType> const &getModelData(size_t index) {
        return m_modelData[index];
    }

    size_t addModel(std::shared_ptr<GameRequester> const &gameRequester,
                      std::shared_ptr<ModelDescription> const &modelDescription)
    {
        auto item = m_modelIndexMap.emplace(modelDescription, 0);
        if (!item.second) {
            m_modelDataVector.emplace_back(getModelData(gameRequester, modelDescription));
            item.first->second = m_modelDataVector.size() - 1;
        }
        return item.first->second;
    }

    ~ModelTableGeneric() override = default;
protected:
    virtual std::shared_ptr<ModelDataType> getModelData(std::shared_ptr<GameRequester> const &gameRequester,
                                     std::shared_ptr<ModelDescription> const &modelDescription) = 0;

    std::map<std::shared_ptr<ModelDescription>, size_t, BaseClassPtrLess<ModelDescription>> m_modelIndexMap;
    std::vector<std::shared_ptr<ModelDataType>> m_modelDataVector;
};
#endif // AMAZING_LABYRINTH_MODEL_TABLE_HPP
