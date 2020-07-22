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
#ifndef AMAZING_LABYRINTH_NORMALMAP_RENDER_DETAILS_VULKAN_HPP
#define AMAZING_LABYRINTH_NORMALMAP_RENDER_DETAILS_VULKAN_HPP

#include <memory>
#include <glm/glm.h>
#include <glm/gtc/matrix_transform.hpp>

#include "../../graphicsVulkan.hpp"
#include "../../mathGraphics.hpp"
#include "../../levelDrawer/drawObjectTable/drawObjectTable.hpp"
#include "../renderDetails.hpp"
#include "../../renderLoader/renderLoaderVulkan.hpp"
#include "../renderDetailsVulkan.hpp"

#include "config.hpp"

namespace normalMap {
    class RenderDetailsVulkan;

    class CommonObjectDataVulkan : public renderDetails::CommonObjectDataOrtho {
        friend RenderDetailsVulkan;
    public:
        std::pair<glm::mat4, glm::mat4> getProjViewForLevel() override {
            return std::make_pair<glm::mat4, glm::mat4>(
                    getOrthoMatrix(m_minusX, m_plusX, m_minusY, m_plusY,
                                    m_nearPlane, m_farPlane, false, true),
                    view());
        }

        std::pair<glm::mat4, glm::mat4> getProjViewForRender() {
            return std::make_pair<glm::mat4, glm::mat4>(
                    getOrthoMatrix(m_minusX, m_plusX, m_minusY, m_plusY,
                                   m_nearPlane, m_farPlane, true, true),
                    view());
        }

        glm::vec3 getLightSource() override {
            return viewPoint();
        }

        glm::mat4 getViewLightSource() override {
            return view();
        }

        float nearestDepth() { return m_nearestDepth; }
        float farthestDepth() { return m_farthestDepth; }

        /*
        std::shared_ptr<vulkan::Buffer> const &cameraBuffer() { return m_camera; }

        uint32_t cameraBufferSize() { return sizeof(CommonUBO); }
        */

        ~CommonObjectDataVulkan() override = default;

    protected:
        void update() override {
            /*
            CommonUBO commonUbo;
            commonUbo.proj = m_preTransform *
                             getPerspectiveMatrix(m_viewAngle, m_aspectRatio, m_nearPlane, m_farPlane, true, true);

            // eye at the m_viewPoint, looking at the m_lookAt position, pointing up is m_up.
            commonUbo.view = view();

            m_camera->copyRawTo(&commonUbo, sizeof(commonUbo));
            */
        }

    private:
        /*
        struct CommonUBO {
            glm::mat4 proj;
            glm::mat4 view;
        };

        std::shared_ptr<vulkan::Buffer> m_camera;
        */
        glm::mat4 m_preTransform;
        float m_nearestDepth;
        float m_farthestDepth;

        CommonObjectDataVulkan(
                //std::shared_ptr<vulkan::Buffer> buffer,
                glm::mat4 preTransform,
                float inNearestDepth,
                float inFarthestDepth,
                Config config,
                float width,
                float height)
                : renderDetails::CommonObjectDataOrtho(-width/2, width/2, -height/2, height/2,
                        config.nearPlane, config.farPlane, config.viewPoint, config.lookAt, config.up),
                /*m_camera{std::move(buffer)},
                  */
                  m_preTransform{preTransform},
                  m_nearestDepth{inNearestDepth},
                  m_farthestDepth{inFarthestDepth}
        {}
    };

    /* for passing data other than the vertex data to the vertex shader */
    class DrawObjectDataVulkan : public renderDetails::DrawObjectDataVulkan {
        friend RenderDetailsVulkan;
    public:
        // todo: fix this.  This function should not be called, but if it were it would do the wrong thing.
        // the correct thing to do is to separate the data that comes from the CommonObjectData into
        // a separate buffer.
        void update(glm::mat4 const &modelMatrix) override {
            PerObjectUBO ubo{};
            ubo.modelMatrix = modelMatrix;
            m_uniformBuffer->copyRawTo(&ubo, sizeof(ubo));
        }

        std::shared_ptr<vulkan::Buffer> const &bufferModelMatrix() override { return m_uniformBuffer; }
        std::shared_ptr<vulkan::DescriptorSet> const &descriptorSet(uint32_t id) override { return m_descriptorSet; }

        ~DrawObjectDataVulkan() override = default;

    private:
        struct PerObjectUBO {
            glm::mat4 mvp;
            glm::mat4 modelMatrix;
            float farthestDepth;
            float nearestDepth;
        };

        std::shared_ptr<vulkan::DescriptorSet> m_descriptorSet;
        std::shared_ptr<vulkan::Buffer> m_uniformBuffer;

        DrawObjectDataVulkan(std::shared_ptr<vulkan::Device> const &inDevice,
                             std::shared_ptr<CommonObjectDataVulkan> const &inCommonObjectData,
                             std::shared_ptr<vulkan::DescriptorSet> inDescriptorSet,
                             std::shared_ptr<vulkan::Buffer> inUniformBuffer)
                : m_descriptorSet{std::move(inDescriptorSet)},
                  m_uniformBuffer{std::move(inUniformBuffer)}
        {
            updateDescriptorSet(inDevice, inCommonObjectData);
        }

        void updateDescriptorSet(std::shared_ptr<vulkan::Device> const &inDevice,
                                 std::shared_ptr<CommonObjectDataVulkan> const &inCommonObjectData);
    };

    class DescriptorSetLayout : public vulkan::DescriptorSetLayout {
    public:
        DescriptorSetLayout(std::shared_ptr<vulkan::Device> inDevice)
                : m_device(inDevice)
        {
            m_poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            m_poolSizes[0].descriptorCount = 1;
            m_poolInfo = {};
            m_poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            m_poolInfo.poolSizeCount = static_cast<uint32_t>(m_poolSizes.size());
            m_poolInfo.pPoolSizes = m_poolSizes.data();
            m_poolInfo.maxSets = m_numberOfDescriptorSetsInPool;
            m_poolInfo.pNext = nullptr;

            createDescriptorSetLayout();
        }
        std::shared_ptr<VkDescriptorSetLayout_T> const &descriptorSetLayout() override {
            return m_descriptorSetLayout;
        }
        uint32_t numberOfDescriptors() override { return m_numberOfDescriptorSetsInPool; }
        VkDescriptorPoolCreateInfo const &poolCreateInfo() override {
            return m_poolInfo;
        }
        virtual ~DescriptorSetLayout() {}

    private:
        static uint32_t constexpr m_numberOfDescriptorSetsInPool = 1024;

        std::shared_ptr<vulkan::Device> m_device;
        std::shared_ptr<VkDescriptorSetLayout_T> m_descriptorSetLayout;
        VkDescriptorPoolCreateInfo m_poolInfo;
        std::array<VkDescriptorPoolSize, 1> m_poolSizes;

        void createDescriptorSetLayout();
    };

    class RenderDetailsVulkan : public renderDetails::RenderDetailsVulkan {
    public:
        static char const *name() { return depthMapRenderDetailsName; }

        static renderDetails::ReferenceVulkan loadNew(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<RenderLoaderVulkan> const &,
                std::shared_ptr<vulkan::Device> const &inDevice,
                std::shared_ptr<renderDetails::Parameters> const &parametersBase,
                Config const &config)
        {
            auto parameters =
                    dynamic_cast<renderDetails::ParametersWithSurfaceWidthHeightAtDepthVulkan const &>(parametersBase);
            if (parameters == nullptr) {
                throw std::runtime_error("Invalid render details parameter type.");
            }

            auto rd = std::make_shared<RenderDetailsVulkan>(
                    gameRequester, inDevice, nullptr, parameters);

            auto cod = rd->createCommonObjectData(parameters, config);

            return createReference(std::move(rd), std::move(cod));
        }

        static renderDetails::ReferenceVulkan loadExisting(
                std::shared_ptr<GameRequester> const &,
                std::shared_ptr<RenderLoaderVulkan> const &,
                std::shared_ptr<renderDetails::RenderDetailsVulkan> const &rdBase,
                std::shared_ptr<renderDetails::Parameters> const &parametersBase,
                Config const &config)
        {
            auto parameters =
                    dynamic_cast<renderDetails::ParametersWithSurfaceWidthHeightAtDepthVulkan const &>(parametersBase);
            if (parameters == nullptr) {
                throw std::runtime_error("Invalid render details parameter type.");
            }

            auto rd = dynamic_cast<RenderDetailsVulkan*>(rdBase.get());
            if (rd == nullptr) {
                throw std::runtime_error("Invalid render details type.");
            }

            auto cod = rd->createCommonObjectData(parameters, config);

            return createReference(std::move(rdBase), std::move(cod));
        }

        void postProcessImageBuffer(
                std::shared_ptr<renderDetails::CommonObjectData> const &,
                std::vector<float> const &input,
                std::vector<float> &results) override
        {
            bitmapToNormals(input, m_surfaceWidth, m_surfaceHeight, 4, true, results);
        }

        void addDrawCmdsToCommandBuffer(
                VkCommandBuffer const &commandBuffer,
                size_t descriptorSetID,
                std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
                std::shared_ptr<levelDrawer::DrawObjectTableVulkan> const &drawObjTable,
                std::vector<size_t> const &drawObjectsIndices) override;

        void reload(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
                ParametersVulkan const &parameters) override;

        std::shared_ptr<vulkan::Device> const &device() override { return m_device; }
        std::shared_ptr<vulkan::DescriptorPools> const &descriptorPools() override {
            return m_descriptorPools;
        }

        bool overrideClearColor(glm::vec4 &clearColor) override {
            clearColor = {0.5f, 0.5f, 1.0f, 1.0f};
            return true;
        }

        ~RenderDetailsVulkan() override = default;

    private:
        static char constexpr const *SHADER_SIMPLE_FRAG_FILE = "shaders/simple.frag.spv";
        static char constexpr const *SHADER_NORMAL_VERT_FILE ="shaders/normal.vert.spv";

        std::shared_ptr<vulkan::Device> m_device;

        std::shared_ptr<vulkan::DescriptorPools> m_descriptorPools;
        std::shared_ptr<DescriptorSetLayout> m_descriptorSetLayout;
        std::shared_ptr<vulkan::Pipeline> m_pipeline;

        RenderDetailsVulkan(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<vulkan::Device> const &inDevice,
                std::shared_ptr<vulkan::Pipeline> const &basePipeline,
                renderDetails::ParametersVulkan const *parameters)
            : RenderDetailsVulkan{parameters->width, parameters->height},
            m_device{inDevice},
            m_descriptorSetLayout{std::make_shared<DescriptorSetLayout>(m_device)},
            m_descriptorPools{std::make_shared<vulkan::DescriptorPools>(m_device, m_descriptorSetLayout)},
            m_pipeline{std::make_shared<vulkan::Pipeline>(
                        gameRequester, m_device,
                        VkExtent2D{m_width, m_height},
                        parameters->renderPass, m_descriptorPools, getBindingDescription(),
                        getAttributeDescriptions(),
                        SHADER_NORMAL_VERT_FILE, SHADER_SIMPLE_FRAG_FILE, basePipeline)}
        {}

        static renderDetails::ReferenceVulkan createReference(
                std::shared_ptr<renderDetails::RenderDetailsVulkan> rd,
                std::shared_ptr<CommonObjectDataVulkan> cod);

        static std::shared_ptr<CommonObjectDataVulkan> createCommonObjectData(
                renderDetails::ParametersWithSurfaceWidthHeightAtDepthVulkan const *parameters,
                Config const &config)
        {
            return std::make_shared<CommonObjectDataVulkan>(
                    parameters->preTransform,
                    parameters->nearestDepth,
                    parameters->farthestDepth,
                    config,
                    parameters->widthAtDepth,
                    parameters->heightAtDepth);
        }
    };
}
#endif // AMAZING_LABYRINTH_NORMALMAP_RENDER_DETAILS_VULKAN_HPP
