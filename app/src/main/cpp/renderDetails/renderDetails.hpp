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
#include "../levelTracker/levelTracker.hpp"
#include "../levelDrawer/textureTable/textureLoader.hpp"

namespace renderDetails {
    template <typename RenderDetailsType, typename CommonObjectDataType, typename DrawObjectDataType>
    struct Reference {
        using CreateDrawObjectData = std::function<std::shared_ptr<DrawObjectDataType>(
                std::shared_ptr<DrawObjectDataType> const &,
                std::shared_ptr<levelDrawer::TextureData> const &,
                glm::mat4 const &)>;
        std::shared_ptr<RenderDetailsType> renderDetails;
        std::shared_ptr<CommonObjectDataType> commonObjectData;
        CreateDrawObjectData createDrawObjectData;
    };

    class CommonObjectData {
    public:
        CommonObjectData() = default;
        virtual ~CommonObjectData() = default;
    protected:
        virtual void update() = 0;
    };

    class CommonObjectDataView : public CommonObjectData {
    public:
        glm::mat4 view() {
            return glm::lookAt(m_viewPoint, m_lookAt, m_up);
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

        ~CommonObjectDataView() override = default;
    protected:
        CommonObjectDataView(
                glm::vec3 viewPoint,
                glm::vec3 lookAt,
                glm::vec3 up)
            : CommonObjectData(),
            m_viewPoint{std::move(viewPoint)},
            m_lookAt{std::move(lookAt)},
            m_up{std::move(up)}
        {}

        glm::vec3 m_viewPoint;
        glm::vec3 m_lookAt;
        glm::vec3 m_up;
    };

    class CommonObjectDataPerspective : public CommonObjectDataView {
    public:
        virtual glm::mat4 getPerspectiveMatrixForLevel() = 0;

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

        virtual ~CommonObjectDataPerspective() = default;
    protected:
        float m_viewAngle;
        float m_aspectRatio;
        float m_nearPlane;
        float m_farPlane;
    };

    class CommonObjectDataOrtho : public CommonObjectDataView {
    public:
        virtual glm::mat4 getPerspectiveMatrixForLevel() = 0;

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

        virtual ~RenderDetails() = default;

    protected:
        uint32_t m_surfaceWidth;
        uint32_t m_surfaceHeight;
    };
}
#endif // AMAZING_LABYRINTH_RENDER_DETAILS_HPP
