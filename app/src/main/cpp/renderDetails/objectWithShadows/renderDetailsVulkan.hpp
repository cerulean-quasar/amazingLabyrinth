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
#ifndef AMAZING_LABYRINTH_OBJECTWITHSHADOWS_RENDER_DETAILS_VULKAN_HPP
#define AMAZING_LABYRINTH_OBJECTWITHSHADOWS_RENDER_DETAILS_VULKAN_HPP

#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../graphicsVulkan.hpp"
#include "../../levelDrawer/drawObjectTable/drawObjectTable.hpp"
#include "../renderDetails.hpp"
#include "../shadows/renderDetailsVulkan.hpp"
#include "../renderDetailsVulkan.hpp"

namespace objectWithShadows {
    class RenderDetailsVulkan;

    class CommonObjectDataVulkan : public renderDetails::CommonObjectDataPerspective {
        friend RenderDetailsVulkan;
    public:
        std::shared_ptr<vulkan::Buffer> const &cameraBuffer() { return m_cameraBuffer; }
        uint32_t cameraBufferSize() { return sizeof (CommonVertexUBO); }
        std::shared_ptr<vulkan::Buffer> const &lightingBuffer() { return m_lightingSourceBuffer; }
        uint32_t lightingBufferSize() { return sizeof (CommonFragmentUBO); }
        std::shared_ptr<vulkan::ImageSampler> const &shadowsSampler() { return m_shadowsSampler; }

        std::pair<glm::mat4, glm::mat4> getProjViewForLevel() override {
            /* perspective matrix: takes the perspective projection, the aspect ratio, near and far
             * view planes.
             */
            return std::make_pair<glm::mat4, glm::mat4>(
                    getPerspectiveMatrix(m_viewAngle, m_aspectRatio, m_nearPlane, m_farPlane,
                                         false, true),
                    view());
        }

        CommonObjectDataVulkan(
                std::shared_ptr<vulkan::Buffer> cameraBuffer,
                std::shared_ptr<vulkan::Buffer> lightingSourceBuffer,
                glm::mat4 const &preTransform,
                float aspectRatio,
                renderDetails::ParametersObjectWithShadowsVulkan const &parameters)
                : renderDetails::CommonObjectDataPerspective(parameters, aspectRatio),
                  m_preTransform{preTransform},
                  m_cameraBuffer{std::move(cameraBuffer)},
                  m_lightingSourceBuffer{std::move(lightingSourceBuffer)},
                  m_shadowsSampler{parameters.shadowsSampler}
        {
            if (m_lightSources.size() > 1) {
                throw std::runtime_error("Incorrect number of light sources for render details.");
            }
            doUpdate();
        }

        ~CommonObjectDataVulkan() override = default;
    private:
        struct CommonVertexUBO {
            glm::mat4 projView;
            glm::mat4 projLightView;
        };

        struct CommonFragmentUBO {
            glm::vec3 lightingSource;
        };

        glm::mat4 m_preTransform;
        std::shared_ptr<vulkan::Buffer> m_cameraBuffer;
        std::shared_ptr<vulkan::Buffer> m_lightingSourceBuffer;
        std::shared_ptr<vulkan::ImageSampler> m_shadowsSampler;

        void doUpdate() {
            CommonVertexUBO commonUbo;
            glm::mat4 proj = m_preTransform *
                             getPerspectiveMatrix(m_viewAngle, m_aspectRatio, m_nearPlane, m_farPlane, true, true);

            // eye at the m_viewPoint, looking at the m_lookAt position, pointing up is m_up.
            commonUbo.projView = proj * glm::lookAt(m_viewPoint, m_lookAt, m_up);
            commonUbo.projLightView = proj * glm::lookAt(m_lightSources[0], m_lookAt, m_up);

            m_cameraBuffer->copyRawTo(&commonUbo, sizeof(commonUbo));

            CommonFragmentUBO commonFragmentUbo;
            commonFragmentUbo.lightingSource = m_lightSources[0];
            m_lightingSourceBuffer->copyRawTo(&commonFragmentUbo, sizeof(commonFragmentUbo));
        }
    };

    class TextureDescriptorSetLayout : public vulkan::DescriptorSetLayout {
    public:
        std::shared_ptr<VkDescriptorSetLayout_CQ> const &descriptorSetLayout() override {
            return m_descriptorSetLayout;
        }

        uint32_t numberOfDescriptors() override { return m_numberOfDescriptorSetsInPool; }

        VkDescriptorPoolCreateInfo const &poolCreateInfo() override {
            return m_poolInfo;
        }

        TextureDescriptorSetLayout(std::shared_ptr<vulkan::Device> inDevice)
                : m_device(inDevice) {
            m_poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            m_poolSizes[0].descriptorCount = m_numberOfDescriptorSetsInPool;
            m_poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            m_poolSizes[1].descriptorCount = m_numberOfDescriptorSetsInPool;
            m_poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            m_poolSizes[2].descriptorCount = m_numberOfDescriptorSetsInPool;
            m_poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            m_poolSizes[3].descriptorCount = m_numberOfDescriptorSetsInPool;
            m_poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            m_poolSizes[4].descriptorCount = m_numberOfDescriptorSetsInPool;
            m_poolInfo = {};
            m_poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            m_poolInfo.poolSizeCount = static_cast<uint32_t>(m_poolSizes.size());
            m_poolInfo.pPoolSizes = m_poolSizes.data();
            m_poolInfo.maxSets = m_numberOfDescriptorSetsInPool;
            m_poolInfo.pNext = nullptr;

            createDescriptorSetLayout();
        }

        ~TextureDescriptorSetLayout() override = default;

    private:
        static uint32_t constexpr m_numberOfDescriptorSetsInPool = 1024;

        std::shared_ptr<vulkan::Device> m_device;
        std::shared_ptr<VkDescriptorSetLayout_CQ> m_descriptorSetLayout;
        VkDescriptorPoolCreateInfo m_poolInfo;
        std::array<VkDescriptorPoolSize, 5> m_poolSizes;

        void createDescriptorSetLayout();
    };

    class ColorDescriptorSetLayout : public vulkan::DescriptorSetLayout {
    public:
        std::shared_ptr<VkDescriptorSetLayout_CQ> const &descriptorSetLayout() override {
            return m_descriptorSetLayout;
        }

        uint32_t numberOfDescriptors() override { return m_numberOfDescriptorSetsInPool; }

        VkDescriptorPoolCreateInfo const &poolCreateInfo() override {
            return m_poolInfo;
        }

        ColorDescriptorSetLayout(std::shared_ptr<vulkan::Device> inDevice)
                : m_device(inDevice) {
            m_poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            m_poolSizes[0].descriptorCount = m_numberOfDescriptorSetsInPool;
            m_poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            m_poolSizes[1].descriptorCount = m_numberOfDescriptorSetsInPool;
            m_poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            m_poolSizes[2].descriptorCount = m_numberOfDescriptorSetsInPool;
            m_poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            m_poolSizes[3].descriptorCount = m_numberOfDescriptorSetsInPool;
            m_poolInfo = {};
            m_poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            m_poolInfo.poolSizeCount = static_cast<uint32_t>(m_poolSizes.size());
            m_poolInfo.pPoolSizes = m_poolSizes.data();
            m_poolInfo.maxSets = m_numberOfDescriptorSetsInPool;
            m_poolInfo.pNext = nullptr;

            createDescriptorSetLayout();
        }

        ~ColorDescriptorSetLayout() override = default;

    private:
        static uint32_t constexpr m_numberOfDescriptorSetsInPool = 1024;

        std::shared_ptr<vulkan::Device> m_device;
        std::shared_ptr<VkDescriptorSetLayout_CQ> m_descriptorSetLayout;
        VkDescriptorPoolCreateInfo m_poolInfo;
        std::array<VkDescriptorPoolSize, 4> m_poolSizes;

        void createDescriptorSetLayout();
    };

    /* for passing data other than the vertex data to the vertex shader */
    class DrawObjectDataVulkan : public renderDetails::DrawObjectDataVulkan {
        friend RenderDetailsVulkan;
    public:
        bool hasTexture() override { return m_hasTexture; }

        void update(glm::mat4 const &inModelMatrix) override {
            PerObjectUBO ubo{};
            ubo.modelMatrix = inModelMatrix;
            m_uniformBuffer->copyRawTo(&ubo, sizeof (ubo));
            m_modelMatrix = inModelMatrix;
        }

        bool updateTextureData(
                std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
                std::shared_ptr<levelDrawer::TextureDataVulkan> const &textureData) override
        {
            if (m_hasTexture && textureData) {
                auto cod = std::dynamic_pointer_cast<CommonObjectDataVulkan>(commonObjectData);
                if (cod == nullptr) {
                    return false;
                }
                textureUpdateDescriptorSet(m_device, cod, textureData);
            } else if (!m_hasTexture && !textureData) {
                // no need to update the descriptor set, but return true since the update is successful
                return true;
            }
            return false;
        }

        void updateModelMatrixNoBufferUpdate(glm::mat4 const &modelMatrix) override {
            m_modelMatrix = modelMatrix;
        }

        std::shared_ptr<vulkan::Buffer> const &bufferModelMatrix() override {
            return m_uniformBuffer;
        }

        std::shared_ptr<vulkan::DescriptorSet> const &descriptorSet(uint32_t) override {
            return m_descriptorSet;
        }

        glm::mat4 modelMatrix(uint32_t) override { return m_modelMatrix; }

        DrawObjectDataVulkan(std::shared_ptr<vulkan::Device> const &inDevice,
                std::shared_ptr<CommonObjectDataVulkan> const &cod,
                std::shared_ptr<levelDrawer::TextureDataVulkan> const &textureData,
                std::shared_ptr<vulkan::DescriptorSet> inDescriptorSet,
                std::shared_ptr<vulkan::Buffer> inBuffer,
                glm::mat4 const &inModelMatrix)
                : m_hasTexture{textureData != nullptr},
                  m_device{inDevice},
                  m_descriptorSet{std::move(inDescriptorSet)},
                  m_uniformBuffer{std::move(inBuffer)},
                  m_modelMatrix{inModelMatrix}
        {
            if (m_hasTexture) {
                textureUpdateDescriptorSet(inDevice, cod, textureData);
            } else {
                colorUpdateDescriptorSet(inDevice, cod);
            }
        }

        ~DrawObjectDataVulkan() override = default;
    private:
        struct PerObjectUBO {
            glm::mat4 modelMatrix;
        };

        bool m_hasTexture;
        std::shared_ptr<vulkan::Device> m_device;
        std::shared_ptr<vulkan::DescriptorSet> m_descriptorSet;
        std::shared_ptr<vulkan::Buffer> m_uniformBuffer;
        glm::mat4 m_modelMatrix;

        void textureUpdateDescriptorSet(
                std::shared_ptr<vulkan::Device> const &inDevice,
                std::shared_ptr<CommonObjectDataVulkan> const &cod,
                std::shared_ptr<levelDrawer::TextureDataVulkan> const &textureData);

        void colorUpdateDescriptorSet(
                std::shared_ptr<vulkan::Device> const &inDevice,
                std::shared_ptr<CommonObjectDataVulkan> const &cod);
    };

    class RenderDetailsVulkan : public renderDetails::RenderDetailsVulkan {
    public:
        std::string nameString() override { return name(); }
        static char const *name() { return objectWithShadowsRenderDetailsName; }

        static renderDetails::ReferenceVulkan loadNew(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<RenderLoaderVulkan> const &,
                std::shared_ptr<vulkan::Device> const &inDevice,
                std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails,
                std::shared_ptr<renderDetails::Parameters> const &parametersBase)
        {
            auto parameters = dynamic_cast<renderDetails::ParametersObjectWithShadowsVulkan*>(parametersBase.get());
            if (parameters == nullptr) {
                throw std::runtime_error("Invalid render details parameter type.");
            }

            auto rd = std::make_shared<RenderDetailsVulkan>(
                    gameRequester, inDevice, nullptr, surfaceDetails);

            auto cod = rd->createCommonObjectData(surfaceDetails->preTransform, parameters);

            return createReference(std::move(rd), std::move(cod));
        }

        static renderDetails::ReferenceVulkan loadExisting(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<RenderLoaderVulkan> const &,
                std::shared_ptr<renderDetails::RenderDetailsVulkan> rdBase,
                std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails,
                std::shared_ptr<renderDetails::Parameters> const &parametersBase)
        {
            auto parameters = dynamic_cast<renderDetails::ParametersObjectWithShadowsVulkan*>(parametersBase.get());
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

            auto cod = rd->createCommonObjectData(surfaceDetails->preTransform, parameters);

            return createReference(std::move(rd), std::move(cod));
        }

        void addDrawCmdsToCommandBuffer(
                VkCommandBuffer const &commandBuffer,
                size_t descriptorSetID,
                std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
                std::shared_ptr<levelDrawer::DrawObjectTableVulkan> const &drawObjTable,
                std::set<levelDrawer::ZValueReference>::iterator beginZValRefs,
                std::set<levelDrawer::ZValueReference>::iterator endZValRefs,
                std::string const &renderDetailsName) override;

        bool structuralChangeNeeded(
                std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails) override
        {
            // Texture and Color pipelines share a render pass so they will give the same answer,
            // just pick one.
            return structuralChangeNeededHelper(surfaceDetails, m_pipelineTexture->renderPass());
        }

        void reload(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails);

        std::shared_ptr<vulkan::Device> const &device() override { return m_device; }

        RenderDetailsVulkan(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<vulkan::Device> const &inDevice,
                std::shared_ptr<vulkan::Pipeline> const &basePipeline,
                std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails)
                : renderDetails::RenderDetailsVulkan{surfaceDetails->surfaceWidth, surfaceDetails->surfaceHeight},
                  m_device{inDevice},
                  m_descriptorSetLayoutTexture{std::make_shared<TextureDescriptorSetLayout>(m_device)},
                  m_descriptorPoolsTexture{std::make_shared<vulkan::DescriptorPools>(m_device, m_descriptorSetLayoutTexture)},
                  m_pipelineTexture{std::make_shared<vulkan::Pipeline>(
                          gameRequester, m_device, VkExtent2D{m_surfaceWidth, m_surfaceHeight},
                          surfaceDetails->renderPass, m_descriptorPoolsTexture,
                          getBindingDescription(),
                          getAttributeDescriptions(),
                          SHADER_VERT_FILE, TEXTURE_SHADER_FRAG_FILE, basePipeline)},
                  m_descriptorSetLayoutColor{std::make_shared<ColorDescriptorSetLayout>(m_device)},
                  m_descriptorPoolsColor{std::make_shared<vulkan::DescriptorPools>(m_device, m_descriptorSetLayoutColor)},
                  m_pipelineColor{std::make_shared<vulkan::Pipeline>(
                          gameRequester, m_device, VkExtent2D{m_surfaceWidth, m_surfaceHeight},
                          surfaceDetails->renderPass, m_descriptorPoolsColor,
                          getBindingDescription(),
                          getAttributeDescriptions(),
                          SHADER_VERT_FILE, COLOR_SHADER_FRAG_FILE, m_pipelineTexture)}
        {}

        ~RenderDetailsVulkan() override = default;

    private:
        static char constexpr const *SHADER_VERT_FILE = "shaders/shader.vert.spv";
        static char constexpr const *TEXTURE_SHADER_FRAG_FILE = "shaders/shader.frag.spv";
        static char constexpr const *COLOR_SHADER_FRAG_FILE = "shaders/colorShader.frag.spv";

        std::shared_ptr<vulkan::Device> m_device;

        /* object with texture resources */
        std::shared_ptr<TextureDescriptorSetLayout> m_descriptorSetLayoutTexture;
        std::shared_ptr<vulkan::DescriptorPools> m_descriptorPoolsTexture;
        std::shared_ptr<vulkan::Pipeline> m_pipelineTexture;

        /* object with vertex coloring only resources */
        std::shared_ptr<ColorDescriptorSetLayout> m_descriptorSetLayoutColor;
        std::shared_ptr<vulkan::DescriptorPools> m_descriptorPoolsColor;
        std::shared_ptr<vulkan::Pipeline> m_pipelineColor;

        std::shared_ptr<CommonObjectDataVulkan> createCommonObjectData(
                glm::mat4 const &preTransform,
                renderDetails::ParametersObjectWithShadowsVulkan const *parameters)
        {
            auto vertexUbo = renderDetails::createUniformBuffer(
                    m_device, sizeof (CommonObjectDataVulkan::CommonVertexUBO));
            auto fragUbo = renderDetails::createUniformBuffer(
                    m_device, sizeof (CommonObjectDataVulkan::CommonFragmentUBO));

            return std::make_shared<CommonObjectDataVulkan>(
                    vertexUbo, fragUbo, preTransform,
                    m_surfaceWidth/static_cast<float>(m_surfaceHeight), *parameters);
        }

        static renderDetails::ReferenceVulkan createReference(
                std::shared_ptr<RenderDetailsVulkan> rd,
                std::shared_ptr<CommonObjectDataVulkan> cod);
    };
}

#endif // AMAZING_LABYRINTH_OBJECTWITHSHADOWS_RENDER_DETAILS_VULKAN_HPP
