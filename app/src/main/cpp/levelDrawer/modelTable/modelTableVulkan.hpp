/**
 * Copyright 2023 Cerulean Quasar. All Rights Reserved.
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

namespace levelDrawer {
    class ModelDataVulkan {
    public:
        inline uint32_t numberIndices() { return m_numberIndices; }

        inline std::shared_ptr<vulkan::Buffer> const &vertexBuffer() { return m_vertexBuffer; }

        inline std::shared_ptr<vulkan::Buffer> const &indexBuffer() { return m_indexBuffer; }

        inline uint32_t numberIndicesWithVertexNormals() { return m_numberIndicesWithVertexNormals; }

        inline std::shared_ptr<vulkan::Buffer> const &vertexBufferWithVertexNormals() {
            if (m_numberIndicesWithVertexNormals == 0) {
                throw std::runtime_error("Vertex normals not requested at model creation, but requested at model usage.");
            }
            return m_vertexBufferWithVertexNormals;
        }

        inline std::shared_ptr<vulkan::Buffer> const &indexBufferWithVertexNormals() {
            if (m_numberIndicesWithVertexNormals == 0) {
                throw std::runtime_error("Vertex normals not requested at model creation, but requested at model usage.");
            }
            return m_indexBufferWithVertexNormals;
        }

        ModelDataVulkan(std::shared_ptr<GameRequester> const &gameRequester,
                        std::shared_ptr<vulkan::Device> const &inDevice,
                        std::shared_ptr<vulkan::CommandPool> const &inPool,
                        std::shared_ptr<ModelDescription> const &model)
                        : m_numberIndices{},
                        m_indexBufferWithVertexNormals{}
        {
            std::pair<ModelVertices, ModelVertices> modelData = model->getData(gameRequester);

            ModelVertices *firstVerticesToLoad = nullptr;
            ModelVertices *secondVerticesToLoad = nullptr;
            switch (model->normalsToLoad()) {
                case 0:
                    throw std::runtime_error("ModelDescription is not loading any vertices.");
                case levelDrawer::ModelDescription::LOAD_BOTH:
                    secondVerticesToLoad = &modelData.second;
                    /* continue on */
                case levelDrawer::ModelDescription::LOAD_FACE_NORMALS:
                    firstVerticesToLoad = &modelData.first;
                    break;
                case levelDrawer::ModelDescription::LOAD_VERTEX_NORMALS:
                    firstVerticesToLoad = &modelData.second;
                    break;
            }

            if (firstVerticesToLoad->first.size() == 0 || firstVerticesToLoad->second.size() == 0) {
                throw std::runtime_error("Error: model has no vertices or indices");
            }

            m_numberIndices = firstVerticesToLoad->second.size();

            m_vertexBuffer = std::make_shared<vulkan::Buffer>(inDevice, sizeof(firstVerticesToLoad->first[0]) *
                                                                        firstVerticesToLoad->first.size(),
                                                              VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            m_indexBuffer = std::make_shared<vulkan::Buffer>(inDevice, sizeof(firstVerticesToLoad->second[0]) *
                                                                       firstVerticesToLoad->second.size(),
                                                             VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                             VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            copyVerticesToBuffer<Vertex>(inPool, firstVerticesToLoad->first, *m_vertexBuffer);
            vulkan::copyIndicesToBuffer(inPool, firstVerticesToLoad->second, *m_indexBuffer);

            if (secondVerticesToLoad) {
                m_numberIndicesWithVertexNormals = modelData.second.second.size();
                m_vertexBufferWithVertexNormals = std::make_shared<vulkan::Buffer>(inDevice,
                                                                                   sizeof(secondVerticesToLoad->first[0]) *
                                                                                   secondVerticesToLoad->first.size(),
                                                                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                m_indexBufferWithVertexNormals = std::make_shared<vulkan::Buffer>(inDevice,
                                                                                  sizeof(secondVerticesToLoad->second[0]) *
                                                                                  secondVerticesToLoad->second.size(),
                                                                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                                                  VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                copyVerticesToBuffer<Vertex>(inPool, secondVerticesToLoad->first,
                                             *m_vertexBufferWithVertexNormals);
                vulkan::copyIndicesToBuffer(inPool, secondVerticesToLoad->second,
                                            *m_indexBufferWithVertexNormals);
            }
        }

        ~ModelDataVulkan() = default;

    private:
        /* vertex buffer and index buffer. the index buffer indicates which vertices to draw and in
         * the specified order.  Note, vertices can be listed twice if they should be part of more
         * than one triangle.
         */
        std::shared_ptr<vulkan::Buffer> m_vertexBuffer;
        std::shared_ptr<vulkan::Buffer> m_indexBuffer;
        uint32_t m_numberIndices;

        std::shared_ptr<vulkan::Buffer> m_vertexBufferWithVertexNormals;
        std::shared_ptr<vulkan::Buffer> m_indexBufferWithVertexNormals;
        uint32_t m_numberIndicesWithVertexNormals;

        template <typename VertexType>
        static void copyVerticesToBuffer(std::shared_ptr<vulkan::CommandPool> const &cmdpool,
                                         std::vector<VertexType> const &vertices,
                                         vulkan::Buffer &buffer)
        {
            VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

            /* use a staging buffer in the CPU accessible memory to copy the data into graphics card
             * memory.  Then use a copy command to copy the data into fast graphics card only memory.
             */
            vulkan::Buffer stagingBuffer(cmdpool->device(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            stagingBuffer.copyRawTo(vertices.data(), bufferSize);

            buffer.copyTo(cmdpool, stagingBuffer, bufferSize);
        }
    };

    class ModelTableVulkan : public ModelTable<ModelDataVulkan> {
    public:
        ModelTableVulkan(
                std::shared_ptr<vulkan::Device> inDevice,
                std::shared_ptr<vulkan::CommandPool> inCommandPool)
                : ModelTable<ModelDataVulkan>{},
                  m_device{inDevice},
                  m_commandPool{inCommandPool} {}

        ~ModelTableVulkan() override = default;

    protected:
        std::shared_ptr<ModelDataVulkan>
        getModelData(std::shared_ptr<GameRequester> const &gameRequester,
                     std::shared_ptr<ModelDescription> const &modelDescription) override {
            return std::make_shared<ModelDataVulkan>(gameRequester, m_device, m_commandPool,
                                                     modelDescription);
        }

    private:
        std::shared_ptr<vulkan::Device> m_device;
        std::shared_ptr<vulkan::CommandPool> m_commandPool;
    };
}

#endif // AMAZING_LABYRINTH_MODEL_TABLE_VULKAN_HPP
