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
#ifndef AMAZING_LABYRINTH_RENDER_DETAILS_DATA_HPP
#define AMAZING_LABYRINTH_RENDER_DETAILS_DATA_HPP

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../levelTracker/levelTracker.hpp"
#include "../../levelDrawer/textureTable/textureLoader.hpp"

namespace renderDetails {
    class CommonObjectData {
    public:
        virtual glm::mat4 getPerspectiveMatrixForLevel() = 0;

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

        CommonObjectData(
            float viewAngle,
            float aspectRatio,
            float nearPlane,
            float farPlane,
            glm::vec3 viewPoint,
            glm::vec3 lookAt,
            glm::vec3 up)
            : m_viewAngle{viewAngle},
            m_aspectRatio{aspectRatio},
            m_nearPlane{nearPlane},
            m_farPlane{farPlane},
            m_viewPoint{viewPoint},
            m_lookAt{lookAt},
            m_up{up}
        {}

        virtual ~CommonObjectData() = default;
    protected:
        virtual void update() = 0;

        float m_viewAngle;
        float m_aspectRatio;
        float m_nearPlane;
        float m_farPlane;

        glm::vec3 m_viewPoint;
        glm::vec3 m_lookAt;
        glm::vec3 m_up;
    };

    class DrawObjectData {
    public:
        virtual void update(glm::mat4 const &) = 0;
        virtual ~DrawObjectData() = default;
    };

    class RenderDetailsData {
    public:
        virtual std::shared_ptr <DrawObjectData> createDrawObjectData(
                std::shared_ptr <TextureData> &textureData,
                glm::mat4 const &modelMatrix) = 0;

        RenderDetailsData(uint32_t inWidth, uint32_t inHeight)
                : m_surfaceWidth{inWidth},
                  m_surfaceHeight{inHeight}
        {}

        virtual ~RenderDetailsData() = default;

    protected:
        uint32_t m_surfaceWidth;
        uint32_t m_surfaceHeight;
    };
}
#endif
