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
#include <memory>

#include "renderDetailsVulkan.hpp"
#include "../../renderLoader/renderLoaderVulkan.hpp"

namespace darkChaining {

    renderDetails::ReferenceVulkan RenderDetailsVulkan::loadNew(
            char const *name,
            std::vector<char const *> const &,
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
            std::shared_ptr<vulkan::Device> const &inDevice,
            std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &parametersBase)
    {
        // initialize main render details
        auto rd = std::make_shared<RenderDetailsVulkan>(name, inDevice, surfaceDetails);

        return loadHelper(gameRequester, renderLoader, std::move(rd), surfaceDetails, parametersBase);
    }

    renderDetails::ReferenceVulkan RenderDetailsVulkan::loadExisting(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
            std::shared_ptr<renderDetails::RenderDetailsVulkan> rdBase,
            std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &parametersBase)
    {
        auto rd = std::dynamic_pointer_cast<RenderDetailsVulkan>(rdBase);
        if (rd == nullptr) {
            throw std::runtime_error("Invalid render details type.");
        }

        if (rd->structuralChangeNeeded(surfaceDetails)) {
            rd->createShadowResources(surfaceDetails);
        }

        return loadHelper(gameRequester, renderLoader, std::move(rd), surfaceDetails, parametersBase);
    }

    renderDetails::ReferenceVulkan RenderDetailsVulkan::loadHelper(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderVulkan> const &renderLoader,
            std::shared_ptr<RenderDetailsVulkan> rd,
            std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &parametersBase) {

        auto parameters = dynamic_cast<renderDetails::ParametersPerspective*>(parametersBase.get());
        if (parameters == nullptr) {
            throw std::runtime_error("Invalid render details parameter type.");
        }

        std::array<std::shared_ptr<shadows::CommonObjectDataVulkan>, numberShadowMaps> shadowCODs = { };
        auto parametersShadows = std::make_shared<renderDetails::ParametersPerspective>();
        renderDetails::ReferenceVulkan refShadows;
        auto shadowsSurfaceDetails = rd->createShadowSurfaceDetails(surfaceDetails);

        // shadows render details
        for (size_t i = 0; i < numberShadowMaps; i++) {
            renderDetails::darkInitializeShadowMapParameters(*parametersShadows, *parameters, i, i==0);

            // We have to load the shadows render details multiple times to get the COD, but it is
            // not a performance problem, because after the render details is loaded the first time,
            // the next times, it is just looked up in a list, not really any work is done other than
            // creating the COD.
            refShadows = renderLoader->load(
                    gameRequester, shadowsRenderDetailsName, shadowsSurfaceDetails,
                    parametersShadows);

            shadowCODs[i] = std::dynamic_pointer_cast<shadows::CommonObjectDataVulkan>(refShadows.commonObjectData);
            if (shadowCODs[i] == nullptr) {
                throw std::runtime_error("Invalid common object data.");
            }
        }

        auto parms = std::make_shared<renderDetails::ParametersDarkObjectVulkan>(*parameters, rd->m_samplersShadows);

        // main render details
        auto refMain = renderLoader->load(
                gameRequester, darkObjectRenderDetailsName,
                surfaceDetails, parms);

        rd->m_darkObjectRenderDetails = refMain.renderDetails;
        rd->m_shadowsRenderDetails = refShadows.renderDetails;

        return createReference(std::move(rd), refShadows, refMain, shadowCODs);
    }

    void RenderDetailsVulkan::addPreRenderPassCmdsToCommandBuffer(
            VkCommandBuffer const &commandBuffer,
            size_t /* descriptor set ID, not used */,
            levelDrawer::CommonObjectDataList const &commonObjectDataList,
            levelDrawer::DrawObjectTableVulkanList const &drawObjTableList,
            std::set<levelDrawer::ZValueReference> const &/* starter Z Value references - unused */,
            std::set<levelDrawer::ZValueReference> const &levelZValues,
            std::set<levelDrawer::ZValueReference> const &/* finisher Z Value references - unused */)
    {
        // check to see if shadows need to be rendered...
        auto cod = dynamic_cast<CommonObjectDataVulkan *>(commonObjectDataList[levelDrawer::ObjectType::LEVEL].get());
        if (cod == nullptr) {
            throw std::runtime_error("Invalid common object data for render details");
        }

        if (!cod->m_shadowMapsNeedRender) {
            return;
        }

        // The shadows rendering needs to occur before the main render pass.
        size_t stopAt = cod->m_holeShadowMapsNeedRender ? numberShadowMaps : numberShadowMaps/renderDetails::numberOfLightSourcesDarkMaze;
        for (size_t i = 0; i < stopAt; i++) {
            /* begin the shadows render pass */
            VkRenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = getVkType<>(m_renderPassesShadows[i]->renderPass().get());
            renderPassInfo.framebuffer = getVkType<>(m_framebuffersShadows[i]->framebuffer().get());
            /* size of the render area */
            renderPassInfo.renderArea.offset = {0, 0};
            auto wh = getShadowsFramebufferDimensions(std::make_pair(m_surfaceWidth, m_surfaceHeight));
            renderPassInfo.renderArea.extent = VkExtent2D{
                    wh.first,
                    wh.second};

            /* the color value to use when clearing the image with VK_ATTACHMENT_LOAD_OP_CLEAR,
             * using black with 0% opacity
             */
            std::vector<VkClearValue> clearValues{};
            clearValues.resize(2);
            clearValues[0].color = {1.0f, 0.0f, 0.0f, 1.0f};
            clearValues[1].depthStencil = {1.0, 0};
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            /* begin recording commands - start by beginning the render pass.
             * none of these functions returns an error (they return void).  There will be no error
             * handling until recording is done.
             */
            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            // only do shadows for the level itself
            m_shadowsRenderDetails->addDrawCmdsToCommandBuffer(
                    commandBuffer,
                    i + renderDetails::MODEL_MATRIX_ID_SHADOWS /* shadows ID */,
                    commonObjectDataList[levelDrawer::ObjectType::LEVEL],
                    drawObjTableList[levelDrawer::ObjectType::LEVEL],
                    levelZValues.begin(), levelZValues.end(),
                    nameString()); // only pay attention to dark chaining draw objects.

            vkCmdEndRenderPass(commandBuffer);
        }

        cod->m_holeShadowMapsNeedRender = false;
        cod->m_shadowMapsNeedRender = false;
    }

    void RenderDetailsVulkan::addDrawCmdsToCommandBuffer(
            VkCommandBuffer const &commandBuffer,
            size_t /* unused descriptor set ID */,
            std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
            std::shared_ptr<levelDrawer::DrawObjectTableVulkan> const &drawObjTable,
            std::set<levelDrawer::ZValueReference>::iterator beginZValRefs,
            std::set<levelDrawer::ZValueReference>::iterator endZValRefs,
            std::string const &)
    {
        m_darkObjectRenderDetails->addDrawCmdsToCommandBuffer(
                commandBuffer, renderDetails::MODEL_MATRIX_ID_MAIN /* main render details ID */,
                commonObjectData, drawObjTable, beginZValRefs, endZValRefs);
    }

    renderDetails::ReferenceVulkan RenderDetailsVulkan::createReference(
            std::shared_ptr<RenderDetailsVulkan> rd,
            renderDetails::ReferenceVulkan const &refShadows,
            renderDetails::ReferenceVulkan const &refDarkObject,
            std::array<std::shared_ptr<shadows::CommonObjectDataVulkan>, numberShadowMaps> shadowsCODs)
    {
        auto darkObject = std::dynamic_pointer_cast<darkObject::CommonObjectDataVulkan>(refDarkObject.commonObjectData);
        if (darkObject == nullptr) {
            throw std::runtime_error("Invalid common object data.");
        }

        auto cod = std::make_shared<CommonObjectDataVulkan>(darkObject, shadowsCODs);

        renderDetails::ReferenceVulkan ref;
        ref.renderDetails = std::move(rd);
        ref.commonObjectData = cod;
        ref.createDrawObjectData = renderDetails::ReferenceVulkan::CreateDrawObjectData(
                [createDODShadows(refShadows.createDrawObjectData),
                createDODDarkObject(refDarkObject.createDrawObjectData)] (
                        std::shared_ptr<renderDetails::DrawObjectDataVulkan> const &sharingDOD,
                        std::shared_ptr<levelDrawer::TextureDataVulkan> const &textureData,
                        glm::mat4 const &modelMatrix) ->
                        std::shared_ptr<renderDetails::DrawObjectDataVulkan>
                {
                    auto dodMain = createDODDarkObject(sharingDOD, textureData, modelMatrix);
                    std::array<std::shared_ptr<renderDetails::DrawObjectDataVulkan>, numberShadowMaps> dodsShadows = {};
                    for (auto &dodShadows : dodsShadows) {
                        dodShadows = createDODShadows(
                                dodMain, std::shared_ptr<levelDrawer::TextureDataVulkan>(),
                                modelMatrix);
                    }
                    return std::make_shared<DrawObjectDataVulkan>(dodMain, dodsShadows);
                }
        );

        ref.getProjViewForLevel = renderDetails::ReferenceVulkan::GetProjViewForLevel(
                [getPVObjectWithShadows(refDarkObject.getProjViewForLevel)]() ->
                        std::pair<glm::mat4, glm::mat4>
                {
                    return getPVObjectWithShadows();
                });

        return ref;
    }

    void RenderDetailsVulkan::createShadowResources(std::shared_ptr<vulkan::SurfaceDetails> const &surfaceDetails) {
        for (auto &framebuffer : m_framebuffersShadows) {
            framebuffer.reset();
        }
        for (auto &renderPass : m_renderPassesShadows) {
            renderPass.reset();
        }
        for (auto &samplerShadows : m_samplersShadows) {
            samplerShadows.reset();
        }
        for (auto &shadowsColorAttachment : m_shadowsColorAttachments) {
            shadowsColorAttachment.reset();
        }
        m_depthImageViewShadows.reset();

        auto wh = getShadowsFramebufferDimensions(std::make_pair(surfaceDetails->surfaceWidth, surfaceDetails->surfaceHeight));

        // shadow resources
        m_depthImageViewShadows = std::make_shared<vulkan::ImageView>(
                vulkan::ImageFactory::createDepthImage(
                        m_device, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, wh.first, wh.second),
                VK_IMAGE_ASPECT_DEPTH_BIT);

        size_t i = 0;
        for (auto &shadowsColorAttachment : m_shadowsColorAttachments) {
            shadowsColorAttachment = vulkan::ImageView::createImageViewAndImage(
                    m_device,
                    wh.first,
                    wh.second,
                    VK_FORMAT_R32G32B32A32_SFLOAT,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    VK_IMAGE_ASPECT_COLOR_BIT);

            m_samplersShadows[i] = std::make_shared<vulkan::ImageSampler>(
                    m_device, shadowsColorAttachment,
                    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                    VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
            auto colorImageInfo =
                    std::vector<vulkan::RenderPass::ImageAttachmentInfo>{
                            vulkan::RenderPass::ImageAttachmentInfo{
                                    VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
                                    shadowsColorAttachment->image()->format(),
                                    VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}};
            auto depthImageInfo =
                    std::make_shared<vulkan::RenderPass::ImageAttachmentInfo>(
                            VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
                            m_depthImageViewShadows->image()->format(),
                            VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            m_renderPassesShadows[i] = vulkan::RenderPass::createDepthTextureRenderPass(
                    m_device, colorImageInfo, depthImageInfo);

            // shadows framebuffer
            m_framebuffersShadows[i] = std::make_shared<vulkan::Framebuffer>(
                    m_device, m_renderPassesShadows[i],
                    std::vector<std::shared_ptr<vulkan::ImageView>>{shadowsColorAttachment,
                                                                    m_depthImageViewShadows},
                    wh.first, wh.second);
            i++;
        }

    }

    RegisterVulkan<renderDetails::RenderDetailsVulkan, RenderDetailsVulkan> registerVulkan(
            darkChainingRenderDetailsName, std::vector<char const *>{});
} // namespace darkChaining
