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
#ifndef AMAZING_LABYRINTH_MODEL_TABLE_VULKAN_HPP
#define AMAZING_LABYRINTH_MODEL_TABLE_VULKAN_HPP

#include <vector>
#include <map>
#include <memory>

#include "../../common.hpp"
#include "modelLoader.hpp"
#include "modelTable.hpp"
#include "../../graphicsVulkan.hpp"

class ModelDataVulkan {
public:
    ModelDataVulkan(std::shared_ptr<GameRequester> const &gameRequester,
                    std::shared_ptr<vulkan::Device> const &inDevice,
                    std::shared_ptr<vulkan::CommandPool> const &inPool,
                    std::shared_ptr<ModelDescription> const &model)
    {
        auto modelData = model->getData(gameRequester);
        if (modelData.first.size() == 0 || modelData.second.size() == 0) {
            throw std::runtime_error("Error: model has no vertices or indices");
        }

        m_vertexBuffer = std::make_shared<vulkan::Buffer>(inDevice, sizeof(modelData.first[0]) * modelData.first.size(),
                       VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        m_indexBuffer = std::make_shared<vulkan::Buffer>(inDevice, sizeof(modelData.second[0]) * modelData.second.size(),
                       VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        copyVerticesToBuffer(inPool, modelData.first);
        copyIndicesToBuffer(inPool, modelData.second);
    }

    inline std::shared_ptr<vulkan::Buffer> const &vertexBuffer() { return m_vertexBuffer; }
    inline std::shared_ptr<vulkan::Buffer> const &indexBuffer() { return m_indexBuffer; }

    ~ModelDataVulkan() = default;
private:
    /* vertex buffer and index buffer. the index buffer indicates which vertices to draw and in
     * the specified order.  Note, vertices can be listed twice if they should be part of more
     * than one triangle.
     */
    std::shared_ptr<vulkan::Buffer> m_vertexBuffer;
    std::shared_ptr<vulkan::Buffer> m_indexBuffer;

    void copyVerticesToBuffer(std::shared_ptr<vulkan::CommandPool> const &cmdpool,
                              std::vector<Vertex> const &vertices)
    {
        vulkan::copyVerticesToBuffer<Vertex>(cmdpool, vertices, m_vertexBuffer);
    }

    void copyIndicesToBuffer(std::shared_ptr<vulkan::CommandPool> const &cmdpool,
                             std::vector<uint32_t> const &indices)
    {
        vulkan::copyIndicesToBuffer(cmdpool, indices, m_indexBuffer);
    }
};

class ModelTableVulkan : public ModelTable<ModelDataVulkan> {
public:
    ModelTableVulkan(
            std::shared_ptr<vulkan::Device> inDevice,
            std::shared_ptr<vulkan::CommandPool> inCommandPool)
            : ModelTableGeneric<ModelDataVulkan>{},
            m_device{inDevice},
            m_commandPool{inCommandPool}
    {}

    ~ModelTableVulkan() override = default;
protected:
    std::shared_ptr<ModelDataVulkan> getModelData(std::shared_ptr<GameRequester> const &gameRequester,
                           std::shared_ptr<ModelDescription> const &modelDescription) override
    {
        return std::make_shared<ModelDataVulkan>(gameRequester, m_device, m_commandPool,
                modelDescription);
    }

private:
    std::shared_ptr<vulkan::Device> m_device;
    std::shared_ptr<vulkan::CommandPool> m_commandPool;
};
#endif // AMAZING_LABYRINTH_MODEL_TABLE_VULKAN_HPP
