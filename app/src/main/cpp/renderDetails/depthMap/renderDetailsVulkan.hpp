/**
 * Copyright 2022 Cerulean Quasar. All Rights Reserved.
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
#ifndef AMAZING_LABYRINTH_DEPTHMAP_RENDER_DETAILS_VULKAN_HPP
#define AMAZING_LABYRINTH_DEPTHMAP_RENDER_DETAILS_VULKAN_HPP

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../graphicsVulkan.hpp"
#include "../../mathGraphics.hpp"
#include "../../levelDrawer/drawObjectTable/drawObjectTable.hpp"
#include "../renderDetails.hpp"
#include "../renderDetailsVulkan.hpp"

#include "config.hpp"
#include "../../../../../../../../Android/Sdk/ndk/21.3.6528147/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/vulkan/vulkan_core.h"

namespace depthMap {
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

        std::shared_ptr<vulkan::Buffer> const &buffer() { return m_buffer; }

        uint32_t bufferSize() { return sizeof(CommonUBO); }

        CommonObjectDataVulkan(
                std::shared_ptr<vulkan::Buffer> inBuffer,
                glm::mat4 preTransform,
                float inNearestDepth,
                float inFarthestDepth,
                Config config,
                float width,
                float height)
                : renderDetails::CommonObjectDataOrtho(-width/2, width/2, -height/2, height/2,
                                                       config.nearPlane, config.farPlane, config.viewPoint, config.lookAt, config.up),
                  m_buffer{std::move(inBuffer)},
                  m_preTransform{preTransform},
                  m_nearestDepth{inNearestDepth},
                  m_farthestDepth{inFarthestDepth}
        {
            doUpdate();
        }

        ~CommonObjectDataVulkan() override = default;

    protected:
        void update() override {
            doUpdate();
        }

        void doUpdate() {
            CommonUBO commonUbo;
            commonUbo.projView = m_preTransform *
                    getOrthoMatrix(m_minusX, m_plusX, m_minusY, m_plusY,
                                   m_nearPlane, m_farPlane, true, true) *
                    view();

            commonUbo.nearestDepth = m_nearestDepth;
            commonUbo.farthestDepth = m_farthestDepth;

            m_buffer->copyRawTo(&commonUbo, sizeof(commonUbo));
        }

    private:
        struct CommonUBO {
            glm::mat4 projView;
            float nearestDepth;
            float farthestDepth;
        };

        std::shared_ptr<vulkan::Buffer> m_buffer;
        glm::mat4 m_preTransform;
        float m_nearestDepth;
        float m_farthestDepth;
    };

    /* for passing data other than the vertex data to the vertex shader */
    class DrawObjectDataVulkan : public renderDetails::DrawObjectDataVulkan {
        friend RenderDetailsVulkan;
    public:
        void update(glm::mat4 const &inModelMatrix) override {
            PerObjectUBO ubo{};
            ubo.modelMatrix = inModelMatrix;
            m_uniformBuffer->copyRawTo(&ubo, sizeof(ubo));
            m_modelMatrix = inModelMatrix;
        }

        void updateModelMatrixNoBufferUpdate(glm::mat4 const &inModelMatrix) override {
            m_modelMatrix = inModelMatrix;
        }

        bool hasTexture() override { return false; }
        std::shared_ptr<vulkan::Buffer> const &bufferModelMatrix() override { return m_uniformBuffer; }
        std::shared_ptr<vulkan::DescriptorSet> const &descriptorSet(uint32_t) override { return m_descriptorSet; }
        glm::mat4 modelMatrix(uint32_t) override { return m_modelMatrix; }

        DrawObjectDataVulkan(std::shared_ptr<vulkan::Device> const &inDevice,
                             std::shared_ptr<CommonObjectDataVulkan> const &inCommonObjectData,
                             std::shared_ptr<vulkan::DescriptorSet> inDescriptorSet,
                             std::shared_ptr<vulkan::Buffer> inUniformBuffer,
                             glm::mat4 const &inModelMatrix)
                : m_descriptorSet{std::move(inDescriptorSet)},
                  m_uniformBuffer{std::move(inUniformBuffer)},
                  m_modelMatrix{inModelMatrix}
        {
            updateDescriptorSet(inDevice, inCommonObjectData);
        }

        ~DrawObjectDataVulkan() override = default;

    private:
        struct PerObjectUBO {
            glm::mat4 modelMatrix;
        };

        std::shared_ptr<vulkan::DescriptorSet> m_descriptorSet;
        std::shared_ptr<vulkan::Buffer> m_uniformBuffer;
        glm::mat4 m_modelMatrix;

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
            m_poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            m_poolSizes[1].descriptorCount = 1;

            m_poolInfo = {};
            m_poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            m_poolInfo.poolSizeCount = static_cast<uint32_t>(m_poolSizes.size());
            m_poolInfo.pPoolSizes = m_poolSizes.data();
            m_poolInfo.maxSets = m_numberOfDescriptorSetsInPool;
            m_poolInfo.pNext = nullptr;

            createDescriptorSetLayout();
        }

        std::shared_ptr<VkDescriptorSetLayout_CQ> const &descriptorSetLayout() override {
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
        std::shared_ptr<VkDescriptorSetLayout_CQ> m_descriptorSetLayout;
        VkDescriptorPoolCreateInfo m_poolInfo;
        std::array<VkDescriptorPoolSize, 2> m_poolSizes;

        void createDescriptorSetLayout();
    };

    class RenderDetailsVulkan : public renderDetails::RenderDetailsVulkan {
    public:
        std::string nameString() override { return name(); }
        static char const *name() { return depthMapRenderDetailsName; }

        static renderDetails::ReferenceVulkan loadNew(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<RenderLoaderVulkan> const &,
                std::shared_ptr<vulkan::Device> const &inDevice,
                std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails,
                std::shared_ptr<renderDetails::Parameters> const &parametersBase,
                Config const &config)
        {
            auto parameters =
                    dynamic_cast<renderDetails::ParametersDepthMap*>(parametersBase.get());
            if (parameters == nullptr) {
                throw std::runtime_error("Invalid render details parameter type.");
            }

            auto rd = std::make_shared<RenderDetailsVulkan>(
                    gameRequester, inDevice, nullptr, surfaceDetails);

            auto cod = rd->createCommonObjectData(surfaceDetails->preTransform, parameters, config);

            return createReference(std::move(rd), std::move(cod));
        }

        static renderDetails::ReferenceVulkan loadExisting(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<RenderLoaderVulkan> const &,
                std::shared_ptr<renderDetails::RenderDetailsVulkan> const &rdBase,
                std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails,
                std::shared_ptr<renderDetails::Parameters> const &parametersBase,
                Config const &config)
        {
            auto parameters =
                    dynamic_cast<renderDetails::ParametersDepthMap*>(parametersBase.get());
            if (parameters == nullptr) {
                throw std::runtime_error("Invalid render details parameter type.");
            }

            auto rd = std::dynamic_pointer_cast<RenderDetailsVulkan>(rdBase);
            if (rd == nullptr) {
                throw std::runtime_error("Invalid render details type.");
            }

            if (rd->structuralChangeNeeded(surfaceDetails)) {
                rd->reload(gameRequester, surfaceDetails);
            }

            auto cod = rd->createCommonObjectData(surfaceDetails->preTransform, parameters, config);

            return createReference(std::move(rd), std::move(cod));
        }

        void postProcessImageBuffer(
                std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
                std::vector<float> const &input,
                std::vector<float> &results) override
        {
            auto cod = dynamic_cast<CommonObjectDataVulkan*>(commonObjectData.get());
            bitmapToDepthMap(input, cod->farthestDepth(), cod->nearestDepth(),
                    m_surfaceWidth, m_surfaceHeight, 4, true, results);
        }

        void addDrawCmdsToCommandBuffer(
                VkCommandBuffer const &commandBuffer,
                size_t descriptorSetID,
                std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
                std::shared_ptr<levelDrawer::DrawObjectTableVulkan> const &drawObjTable,
                std::set<levelDrawer::ZValueReference>::iterator beginZValRefs,
                std::set<levelDrawer::ZValueReference>::iterator endZValRefs) override;

        bool structuralChangeNeeded(
                std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails) override
        {
            return structuralChangeNeededHelper(surfaceDetails, m_pipeline->renderPass());
        }

        void reload(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails);

        std::shared_ptr<vulkan::Device> const &device() override { return m_device; }
        std::shared_ptr<vulkan::DescriptorPools> const &descriptorPools() { return m_descriptorPools; }

        bool overrideClearColor(glm::vec4 &clearColor) override {
            clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
            return true;
        }

        RenderDetailsVulkan(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<vulkan::Device> const &inDevice,
                std::shared_ptr<vulkan::Pipeline> const &basePipeline,
                std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails)
                : renderDetails::RenderDetailsVulkan{surfaceDetails->surfaceWidth, surfaceDetails->surfaceHeight},
                  m_device{inDevice},
                  m_descriptorSetLayout{std::make_shared<DescriptorSetLayout>(m_device)},
                  m_descriptorPools{std::make_shared<vulkan::DescriptorPools>(m_device, m_descriptorSetLayout)},
                  m_pipeline{}
        {
            std::string vertFile{SHADER_LINEAR_DEPTH_VERT_FILE};
            std::string fragFile{SHADER_SIMPLE_FRAG_FILE};
            VkExtent2D extent{m_surfaceWidth, m_surfaceHeight};
            m_pipeline = std::make_shared<vulkan::Pipeline>(
                    gameRequester, m_device,
                    extent,
                    surfaceDetails->renderPass, m_descriptorPools, getBindingDescription(),
                    getAttributeDescriptions(),
                    vertFile, fragFile, basePipeline);
        }

        ~RenderDetailsVulkan() override = default;

    private:
        static char constexpr const *SHADER_SIMPLE_FRAG_FILE = "shaders/simple.frag.spv";
        static char constexpr const *SHADER_LINEAR_DEPTH_VERT_FILE ="shaders/linearDepth.vert.spv";

        std::shared_ptr<vulkan::Device> m_device;

        std::shared_ptr<DescriptorSetLayout> m_descriptorSetLayout;
        std::shared_ptr<vulkan::DescriptorPools> m_descriptorPools;
        std::shared_ptr<vulkan::Pipeline> m_pipeline;

        static renderDetails::ReferenceVulkan createReference(
                std::shared_ptr<RenderDetailsVulkan> rd,
                std::shared_ptr<CommonObjectDataVulkan> cod);

        std::shared_ptr<CommonObjectDataVulkan> createCommonObjectData(
                glm::mat4 const &preTransform,
                renderDetails::ParametersDepthMap const *parameters,
                Config const &config)
        {
            auto buffer = renderDetails::createUniformBuffer(
                    m_device, sizeof (CommonObjectDataVulkan::CommonUBO));
            return std::make_shared<CommonObjectDataVulkan>(
                    std::move(buffer),
                    preTransform,
                    parameters->nearestDepth,
                    parameters->farthestDepth,
                    config,
                    parameters->widthAtDepth,
                    parameters->heightAtDepth);
        }
    };
}
#endif // AMAZING_LABYRINTH_DEPTHMAP_RENDER_DETAILS_VULKAN_HPP
