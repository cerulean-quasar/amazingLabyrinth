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
#ifndef AMAZING_LABYRINTH_LEVEL_DRAWER_COMMON_HPP
#define AMAZING_LABYRINTH_LEVEL_DRAWER_COMMON_HPP

#include <memory>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <glm/glm.hpp>

namespace levelTracker {
    static float constexpr m_maxZLevel = -1.0f;
    static float constexpr m_maxZLevelStarter = 0.0f;
    static float constexpr m_maxZLevelFinisher = 0.5f;
}

namespace renderDetails {
    class CommonObjectData;

    // extra Parameters for render details.  Only some render details require these and the
    // requester of the render details (i.e. the level) needs to know what these are and what
    // to put in them.
    struct Parameters {
        float nearPlane;
        float farPlane;
        glm::vec3 lookAt;
        glm::vec3 up;
        virtual ~Parameters() = default;
    };

    struct ParametersLightSource : public Parameters {
        float viewAngle;
        glm::vec3 lightingSource;
        ~ParametersLightSource() override = default;
    };

    struct ParametersObject : public ParametersLightSource {
        glm::vec3 viewPoint;

        ~ParametersObject() override = default;
    };

    struct ParametersDepthMap : public Parameters {
        float widthAtDepth;
        float heightAtDepth;
        float nearestDepth;
        float farthestDepth;
        glm::vec3 viewPoint;

        ~ParametersDepthMap() override = default;
    };

    struct ParametersNormalMap : public Parameters {
        float widthAtDepth;
        float heightAtDepth;
        glm::vec3 viewPoint;

        ~ParametersNormalMap() override = default;
    };

    using PostprocessingDataInputGL = boost::variant<std::vector<uint16_t>, std::vector<uint8_t>>;
}

namespace levelDrawer {
    struct DefaultConfig {
        static float constexpr const viewAngle = 3.1415926f/4.0f;
        static float constexpr const nearPlane = 0.5f;
        static float constexpr const farPlane = 5.0f;
        static glm::vec3 constexpr const viewPoint{0.0f, 0.0f, 1.0f};
        static glm::vec3 constexpr const lightingSource{1.0f, 1.0f, 1.5f};
        static glm::vec3 constexpr const lookAt{0.0f, 0.0f, levelTracker::m_maxZLevel};
        static glm::vec3 constexpr const up{0.0f, 1.0f, 0.0f};
    };

    template<typename BaseClass>
    class BaseClassPtrLess {
    public:
        bool operator()(std::shared_ptr<BaseClass> const &p1,
                        std::shared_ptr<BaseClass> const &p2) const {
            BaseClass &p1ref = *p1;
            BaseClass &p2ref = *p2;
            std::type_info const &c1 = typeid(p1ref);
            std::type_info const &c2 = typeid(p2ref);
            if (c1 != c2) {
                return c1.before(c2);
            } else if (p1.get() == p2.get()) {
                return false;
            } else {
                return p1->compareLess(p2.get());
            }
        }
    };

    class ModelDescription;
    class TextureDescription;
    using ModelsTextures = std::vector<std::pair<std::shared_ptr<ModelDescription>, std::shared_ptr<TextureDescription>>>;
    enum ObjectType {
        STARTER,
        LEVEL,
        FINISHER
    };

    using DrawObjReference = uint64_t;
    using DrawObjDataReference = uint64_t;

    static size_t constexpr const nbrDrawObjectTables = 3;

    using CommonObjectDataList = std::array<std::shared_ptr<renderDetails::CommonObjectData>, nbrDrawObjectTables>;

    template <typename traits> class DrawObjectTable;

    struct DrawObjectVulkanTraits;
    using DrawObjectTableVulkan = DrawObjectTable<DrawObjectVulkanTraits>;
    using DrawObjectTableVulkanList = std::array<std::shared_ptr<DrawObjectTableVulkan>, nbrDrawObjectTables>;

    struct DrawObjectGLTraits;
    using DrawObjectTableGL = DrawObjectTable<DrawObjectGLTraits>;
    using DrawObjectTableGList = std::array<std::shared_ptr<DrawObjectTableGL>, nbrDrawObjectTables>;

    struct ZValueReference {
        static float constexpr errVal = 0.000001f;
        boost::optional<float> z;
        DrawObjReference drawObjectReference;
        boost::optional<DrawObjDataReference>  drawObjectDataReference;

        ZValueReference(
                boost::optional<float> inZ,
                DrawObjReference inDrawObjectReference,
                boost::optional<DrawObjDataReference> inDrawObjectDataReference)
                : z{inZ},
                drawObjectReference{inDrawObjectReference},
                drawObjectDataReference{inDrawObjectDataReference}
        {}

        ZValueReference(ZValueReference const &other) = default;
        ZValueReference(ZValueReference &&other) = default;
        ZValueReference &operator=(ZValueReference &other) = default;

        bool operator ==(ZValueReference const &other) const {
            return !(*this < other) && !(other < *this);
        }

        bool operator <(ZValueReference const &other) const {
            if (z != boost::none && other.z != boost::none &&
               (z.get() > other.z.get() + errVal || z.get() < other.z.get() - errVal))
            {
                return z.get() < other.z.get();
            }

            if (drawObjectReference != other.drawObjectReference) {
                return drawObjectReference < other.drawObjectReference;
            }

            if (drawObjectDataReference != boost::none && other.drawObjectDataReference != boost::none) {
                return drawObjectDataReference.get() < other.drawObjectDataReference.get();
            }

            return false;
        }
    };
}
#endif // AMAZING_LABYRINTH_LEVEL_DRAWER_COMMON_HPP