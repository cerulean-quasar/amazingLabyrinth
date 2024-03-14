/**
 * Copyright 2024 Cerulean Quasar. All Rights Reserved.
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
#ifndef AMAZING_LABYRINTH_RENDER_DETAILS_HPP
#define AMAZING_LABYRINTH_RENDER_DETAILS_HPP

#include <memory>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../levelTracker/levelTracker.hpp"
#include "../levelDrawer/textureTable/textureLoader.hpp"

namespace renderDetails {
    // only really matters to the shadowsChaining render details.  It tells the Draw Object data
    // which descriptor set to return when requested.  MODEL_MATRIX_ID_MAIN is for the main
    // render details descriptor set, and MODEL_MATRIX_ID_SHADOWS is for the shadows render details
    // descriptor set.
    enum {
        MODEL_MATRIX_ID_MAIN,
        MODEL_MATRIX_ID_SHADOWS
    };

    class CommonObjectDataBase {
    public:
        virtual void update(ParametersBase const &parameters) = 0;
        CommonObjectDataBase(ParametersBase const &) { }
        CommonObjectDataBase() = default;
        virtual ~CommonObjectDataBase() = default;
    };

    class CommonObjectData : public CommonObjectDataBase {
    public:
        CommonObjectData(std::shared_ptr<Parameters> const &parameters)
            : CommonObjectDataBase(),
            m_nearPlane{parameters->nearPlane},
            m_farPlane{parameters->farPlane} {}

        CommonObjectData(Parameters const &parameters)
                : m_nearPlane{parameters.nearPlane},
                  m_farPlane{parameters.farPlane} {}

        void update(renderDetails::ParametersBase const &parametersBase) override {
            auto parameters = dynamic_cast<renderDetails::Parameters const &>(parametersBase);

            update(parameters);
        }

        void update(renderDetails::Parameters const &parameters) {
            m_nearPlane = parameters.nearPlane;
            m_farPlane = parameters.farPlane;
        }

        ~CommonObjectData() override = default;

    protected:
        float m_nearPlane;
        float m_farPlane;
    };

    class CommonObjectDataLightSources : public CommonObjectData {
    public:
        glm::vec3 getLightSource(size_t i) {
            if (i >= m_lightSources.size()) {
                throw std::runtime_error("lighting source requested is out of range.");
            }

            return m_lightSources[i];
        }

        glm::mat4 getViewLightSource(size_t i, glm::vec3 const &lookAt, glm::vec3 const &up) {
            if (i >= m_lightSources.size()) {
                throw std::runtime_error("lighting source requested is out of range.");
            }

            return glm::lookAt(m_lightSources[i], lookAt, up);
        }

        void update(renderDetails::ParametersBase const &parametersBase) override {
            auto const &parameters = dynamic_cast<renderDetails::ParametersLightSources const &>(parametersBase);

            update(parameters);
        }

        void update(renderDetails::ParametersLightSources const &parameters)  {
            CommonObjectData::update(parameters);
            m_lightSources = parameters.lightingSources;
        }

        CommonObjectDataLightSources(std::shared_ptr<ParametersLightSources> const &parameters)
            : CommonObjectData{parameters},
            m_lightSources(parameters->lightingSources) {}

        CommonObjectDataLightSources(ParametersLightSources const &parameters)
                : CommonObjectData{parameters},
                m_lightSources(parameters.lightingSources) {}

        ~CommonObjectDataLightSources() override = default;

    protected:
        std::vector<glm::vec3> m_lightSources;
    };

    class CommonObjectDataView : public CommonObjectDataLightSources {
    public:
        glm::mat4 view() {
            // eye at the m_viewPoint, looking at the m_lookAt position, pointing up is m_up.
            return glm::lookAt(m_viewPoint, m_lookAt, m_up);
        }

        glm::vec3 viewPoint() {
            return m_viewPoint;
        }

        glm::mat4 getViewLightSource(size_t i) {
            if (i > m_lightSources.size() - 1) {
                throw std::runtime_error("lighting source requested is out of range.");
            }

            return glm::lookAt(m_lightSources[i], m_lookAt, m_up);
        }

        void update(renderDetails::ParametersBase const &parametersBase) override {
            auto const &parameters = dynamic_cast<renderDetails::ParametersView const &>(parametersBase);

            update(parameters);
        }

        void update(renderDetails::ParametersView const &parameters)  {
            CommonObjectDataLightSources::update(parameters);
            m_viewPoint = parameters.viewPoint;
            m_lookAt = parameters.lookAt;
            m_up = parameters.up;
        }

        CommonObjectDataView(std::shared_ptr<ParametersView> const &parameters)
                : CommonObjectDataLightSources{parameters},
                  m_viewPoint{parameters->viewPoint},
                  m_lookAt{parameters->lookAt},
                  m_up{parameters->up}
        {}

        CommonObjectDataView(ParametersView const &parameters)
                : CommonObjectDataLightSources{parameters},
                  m_viewPoint{parameters.viewPoint},
                  m_lookAt{parameters.lookAt},
                  m_up{parameters.up}
        {}

        ~CommonObjectDataView() override = default;
    protected:
        glm::vec3 m_viewPoint;
        glm::vec3 m_lookAt;
        glm::vec3 m_up;
    };

    class CommonObjectDataPerspective : public CommonObjectDataView {
    public:
        virtual std::pair<glm::mat4, glm::mat4> getProjViewForLevel() = 0;

        void update(renderDetails::ParametersBase const &parametersBase) override {
            auto const &parameters = dynamic_cast<renderDetails::ParametersPerspective const &>(parametersBase);

            update(parameters);
        }

        void update(renderDetails::ParametersPerspective const &parameters)  {
            CommonObjectDataView::update(parameters);
            m_viewAngle = parameters.viewAngle;
            m_up = parameters.up;
        }

        CommonObjectDataPerspective(CommonObjectDataPerspective &) = delete;

        CommonObjectDataPerspective(
            std::shared_ptr<ParametersPerspective> const &parameters,
            float aspectRatio)
            : CommonObjectDataView{parameters},
            m_viewAngle{parameters->viewAngle},
            m_aspectRatio{aspectRatio}
        {}

        CommonObjectDataPerspective(
                ParametersPerspective const &parameters,
                float aspectRatio)
                : CommonObjectDataView{parameters},
                  m_viewAngle{parameters.viewAngle},
                  m_aspectRatio{aspectRatio}
        {}

        ~CommonObjectDataPerspective() override = default;
    protected:
        float m_viewAngle;
        float m_aspectRatio;
    };

    class CommonObjectDataOrtho : public CommonObjectDataView {
    public:
        float nearPlane() { return m_nearPlane; }
        float farPlane() { return m_farPlane; }

        virtual std::pair<glm::mat4, glm::mat4> getProjViewForLevel() = 0;

        void update(renderDetails::ParametersBase const &parametersBase) override {
            auto const &parameters = dynamic_cast<renderDetails::ParametersOrtho const &>(parametersBase);

            update(parameters);
        }

        void update(renderDetails::ParametersOrtho const &parameters)  {
            CommonObjectDataView::update(parameters);
            m_minusX = parameters.minusX;
            m_plusX = parameters.plusX;
            m_minusY = parameters.minusY;
            m_plusY = parameters.plusY;
        }

        CommonObjectDataOrtho(
                std::shared_ptr<ParametersOrtho> const &parameters)
                : CommonObjectDataView{parameters},
                  m_minusX{parameters->minusX},
                  m_plusX{parameters->plusX},
                  m_minusY{parameters->minusY},
                  m_plusY{parameters->plusY}
        {}

        CommonObjectDataOrtho(
                ParametersOrtho const &parameters)
                : CommonObjectDataView{parameters},
                  m_minusX{parameters.minusX},
                  m_plusX{parameters.plusX},
                  m_minusY{parameters.minusY},
                  m_plusY{parameters.plusY}
        {}

        ~CommonObjectDataOrtho() override = default;
    protected:
        float m_minusX;
        float m_plusX;
        float m_minusY;
        float m_plusY;
    };

    template <typename RenderDetailsType, typename TextureDataType, typename DrawObjectDataType>
    struct Reference {
        using CreateDrawObjectData = std::function<std::shared_ptr<DrawObjectDataType>(
                std::shared_ptr<DrawObjectDataType> const &,
                std::shared_ptr<TextureDataType> const &,
                glm::mat4 const &)>;
        using GetProjViewForLevel = std::function<std::pair<glm::mat4, glm::mat4>()>;
        std::shared_ptr<RenderDetailsType> renderDetails;
        std::shared_ptr<CommonObjectDataBase> commonObjectData;
        CreateDrawObjectData createDrawObjectData;
        GetProjViewForLevel getProjViewForLevel;
    };

    class DrawObjectData {
    public:
        // this function is required in vulkan as well because it is used to determine the z-order
        // of objects so that they can be drawn farthest to nearest.  (We obtain the z value from
        // the model matrix.)
        virtual glm::mat4 modelMatrix(uint32_t) = 0;

        virtual void update(glm::mat4 const &) = 0;

        virtual ~DrawObjectData() = default;
    };

    class RenderDetails {
    public:
        RenderDetails(Description inDescription, uint32_t inWidth, uint32_t inHeight)
                : m_description{std::move(inDescription)},
                  m_surfaceWidth{inWidth},
                  m_surfaceHeight{inHeight}
        {}

        Description const &description() const {
            return m_description;
        }

        uint32_t width() { return m_surfaceWidth; }
        uint32_t height() { return m_surfaceHeight; }

        virtual ~RenderDetails() = default;

    protected:
        Description const m_description;
        uint32_t m_surfaceWidth;
        uint32_t m_surfaceHeight;
    };
}
#endif // AMAZING_LABYRINTH_RENDER_DETAILS_HPP
