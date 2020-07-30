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
#ifndef AMAZING_LABYRINTH_RENDER_DETAILS_HPP
#define AMAZING_LABYRINTH_RENDER_DETAILS_HPP

#include <memory>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../levelTracker/levelTracker.hpp"
#include "../levelDrawer/textureTable/textureLoader.hpp"

namespace renderDetails {
    struct Parameters {
        uint32_t width;
        uint32_t height;

        virtual ~Parameters() = default;
    };

    class CommonObjectData {
    public:
        virtual std::pair<glm::mat4, glm::mat4> getProjViewForLevel() = 0;
        virtual glm::vec3 getLightSource() = 0;
        virtual glm::mat4 getViewLightSource() = 0;
        CommonObjectData() = default;
        virtual ~CommonObjectData() = default;
    protected:
        virtual void update() {} // todo: revisit this: only needed for Vulkan
    };

    template <typename RenderDetailsType, typename TextureDataType, typename DrawObjectDataType>
    struct Reference {
        using CreateDrawObjectData = std::function<std::shared_ptr<DrawObjectDataType>(
                std::shared_ptr<DrawObjectDataType> const &,
                std::shared_ptr<TextureDataType> const &,
                glm::mat4 const &)>;
        using GetProjViewForLevel = std::function<std::pair<glm::mat4, glm::mat4>()>;
        std::shared_ptr<RenderDetailsType> renderDetails;
        std::shared_ptr<CommonObjectData> commonObjectData;
        CreateDrawObjectData createDrawObjectData;
        GetProjViewForLevel getProjViewForLevel;
    };

    class CommonObjectDataView : public CommonObjectData {
    public:
        glm::mat4 view() {
            return glm::lookAt(m_viewPoint, m_lookAt, m_up);
        }

        glm::vec3 viewPoint() {
            return m_viewPoint;
        }

        void setView(
                glm::vec3 const &viewPoint,
                glm::vec3 const &lookAt,
                glm::vec3 const &up)
        {
            m_viewPoint = viewPoint;
            m_lookAt = lookAt;
            m_up = up;
            update();
        }

        CommonObjectDataView(
                glm::vec3 viewPoint,
                glm::vec3 lookAt,
                glm::vec3 up)
                : CommonObjectData(),
                  m_viewPoint{std::move(viewPoint)},
                  m_lookAt{std::move(lookAt)},
                  m_up{std::move(up)}
        {}

        ~CommonObjectDataView() override = default;
    protected:
        glm::vec3 m_viewPoint;
        glm::vec3 m_lookAt;
        glm::vec3 m_up;
    };

    class CommonObjectDataPerspective : public CommonObjectDataView {
    public:
        void setProjection(
                float viewAngle,
                float aspectRatio,
                float nearPlane,
                float farPlane)
        {
            m_viewAngle = viewAngle;
            m_aspectRatio = aspectRatio;
            m_nearPlane = nearPlane;
            m_farPlane = farPlane;
            update();
        }

        CommonObjectDataPerspective(
            float viewAngle,
            float aspectRatio,
            float nearPlane,
            float farPlane,
            glm::vec3 viewPoint,
            glm::vec3 lookAt,
            glm::vec3 up)
            : CommonObjectDataView{viewPoint, lookAt, up},
            m_viewAngle{viewAngle},
            m_aspectRatio{aspectRatio},
            m_nearPlane{nearPlane},
            m_farPlane{farPlane}
        {}

        ~CommonObjectDataPerspective() override = default;
    protected:
        float m_viewAngle;
        float m_aspectRatio;
        float m_nearPlane;
        float m_farPlane;
    };

    class CommonObjectDataOrtho : public CommonObjectDataView {
    public:
        void setProjection(
                float minusX,
                float plusX,
                float minusY,
                float plusY,
                float nearPlane,
                float farPlane)
        {
            m_minusX = minusX;
            m_plusX = plusX;
            m_minusY = minusY;
            m_plusY = plusY;
            m_nearPlane = nearPlane;
            m_farPlane = farPlane;
            update();
        }

        float nearPlane() { return m_nearPlane; }
        float farPlane() { return m_farPlane; }

        CommonObjectDataOrtho(
                float minusX,
                float plusX,
                float minusY,
                float plusY,
                float nearPlane,
                float farPlane,
                glm::vec3 viewPoint,
                glm::vec3 lookAt,
                glm::vec3 up)
                : CommonObjectDataView{viewPoint, lookAt, up},
                  m_minusX{minusX},
                  m_plusX{plusX},
                  m_minusY{minusY},
                  m_plusY{plusY},
                  m_nearPlane{nearPlane},
                  m_farPlane{farPlane}
        {}

        ~CommonObjectDataOrtho() override = default;
    protected:
        float m_minusX;
        float m_plusX;
        float m_minusY;
        float m_plusY;
        float m_nearPlane;
        float m_farPlane;
    };

    class DrawObjectData {
    public:
        virtual void update(glm::mat4 const &) = 0;
        virtual ~DrawObjectData() = default;
    };

    class RenderDetails {
    public:
        RenderDetails(uint32_t inWidth, uint32_t inHeight)
                : m_surfaceWidth{inWidth},
                  m_surfaceHeight{inHeight}
        {}

        uint32_t width() { return m_surfaceWidth; }
        uint32_t height() { return m_surfaceHeight; }

        virtual std::string nameString() = 0;

        virtual ~RenderDetails() = default;

    protected:
        uint32_t m_surfaceWidth;
        uint32_t m_surfaceHeight;
    };

    // todo: fix these redefinitions
    static size_t constexpr const nbrDrawObjectTables = 3;
    using DrawObjReference = uint64_t;
    using DrawObjRefsForDrawList = std::array<std::vector<DrawObjReference>, nbrDrawObjectTables>;
    using CommonObjectDataList = std::array<std::shared_ptr<CommonObjectData>, nbrDrawObjectTables>;
}
#endif // AMAZING_LABYRINTH_RENDER_DETAILS_HPP
