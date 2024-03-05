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
#ifndef AMAZING_LABYRINTH_COMMON_HPP
#define AMAZING_LABYRINTH_COMMON_HPP

#include <vector>
#include <set>
#include <array>
#include <string>
#include <memory>
#include <streambuf>
#include <functional>
#include <atomic>
#include <cassert>
#include <variant>

using FeatureValue = std::variant<void*, size_t>;

namespace renderDetails {
    enum FeatureType {
        shadows,
        texture,
        color,
        chaining,
        specular,
        phosphorescentEdge,
        numberFeatures // must be last
    };

    enum DrawingStyle {
        shadowMap,
        standard,
        dark,
        darkV2,
        depthMap,
        normalMap,
        numberDrawingStyles
    };

    class FeatureList {
    public:
        using FeatureListInternal = std::set<std::pair<std::size_t, FeatureValue>>;

        FeatureList() : m_features() {}

        FeatureList(std::vector<FeatureType> const &types)
                : m_features()
        {
            setFeature(types, {});
        }

        FeatureList(std::vector<FeatureType> const &types, std::vector<FeatureValue> const &values)
            : m_features()
        {
            setFeature(types, values);
        }

        void setFeature(FeatureType type, FeatureValue const &value = nullptr) {
            m_features.insert(std::make_pair(type, value));
        }

        void setFeature(std::vector<FeatureType> const &types) {
            setFeature(types, {});
        }

        void setFeature(std::vector<FeatureType> const &types, std::vector<FeatureValue> const &values) {
            size_t valuesSize = values.size();
            if (valuesSize != 0 && valuesSize != types.size()) {
                throw std::runtime_error("There is a mismatch in the number of Feature Types and corresponding values");
            }

            for (size_t i = 0; i < types.size(); i++) {
                if (valuesSize == 0) {
                    auto it = m_features.insert(std::make_pair(types[i], nullptr));
                } else {
                    m_features.insert(std::make_pair(types[i], values[i]));
                }
            }
        }

        void unsetFeature(std::vector<FeatureType> const &types, std::vector<FeatureValue> const &values) {
            size_t valuesSize = values.size();
            if (valuesSize != 0 && valuesSize != types.size()) {
                throw std::runtime_error("There is a mismatch in the number of Feature Types and corresponding values");
            }

            for (size_t i = 0; i < types.size(); i++) {
                if (valuesSize == 0) {
                    m_features.erase(std::make_pair(types[i], nullptr));
                } else {
                    m_features.erase(std::make_pair(types[i], values[i]));
                }
            }
        }

        void unsetFeature(FeatureType type) {
            unsetFeature(type, nullptr);
        }

        void unsetFeature(FeatureType type, FeatureValue const &value) {
            m_features.erase(std::make_pair(type, value));
        }

        bool isFeaturePresent(FeatureType const &type) const {
            return isFeaturePresent(type, nullptr);
        }

        bool isFeaturePresent(FeatureType const &type, FeatureValue const &value) const {
            auto it = m_features.find(std::make_pair(type, value));
            return it != m_features.end();
        }

        FeatureListInternal const &features() const { return m_features; }

        bool operator==(FeatureList const &f) const {
            return m_features == f.m_features;
        }

        bool operator!=(FeatureList const &f) const {
            return (*this == f);
        }

        bool operator<(FeatureList const &f) const {
            if (m_features.size() != f.m_features.size()) {
                return m_features.size() < f.m_features.size();
            } else {
                auto it = m_features.begin();
                auto itOther = f.m_features.begin();
                for (;
                     it != m_features.end() && itOther != f.m_features.end();
                     it++, itOther++)
                {
                    if (*it != *itOther) {
                        return *it < *itOther;
                    }
                }

                return false;
            }
        }
    private:
        FeatureListInternal m_features;
    };

    struct Query {
    public:
        Query(DrawingStyle inStyle)
            :style(inStyle),
            requiredFeatures(),
            optionalFeatures() {}

        Query(
                DrawingStyle inStyle,
                std::vector<FeatureType> const &inRequiredFeatures,
                std::vector<FeatureType> const &inOptionalFeatures)
                : style{inStyle},
                  requiredFeatures(inRequiredFeatures),
                  optionalFeatures(inOptionalFeatures) {}

        Query(
                DrawingStyle inStyle,
                FeatureList inRequiredFeatures,
                FeatureList inOptionalFeatures)
                : style{inStyle},
                  requiredFeatures(std::move(inRequiredFeatures)),
                  optionalFeatures(std::move(inOptionalFeatures)) {}

        std::size_t style;
        FeatureList requiredFeatures;
        FeatureList optionalFeatures;
    };

    class Description {
    public:
        auto drawingMethod() const { return static_cast<DrawingStyle>(m_style); }

        auto const &features() const { return m_features; }

        unsigned int getMatchPotential(Query const &query) {
            if (query.style != m_style) {
                return 0;
            }

            unsigned int score = 1;
            auto const &features = m_features.features();
            for (auto const &featureQ : query.requiredFeatures.features()) {
                if (!features.contains(featureQ)) {
                    return 0;
                }
            }

            for (auto const &featureQ : query.optionalFeatures.features()) {
                if (features.contains(featureQ)) {
                    score++;
                }
            }

            auto const &featuresQO = query.optionalFeatures.features();
            auto const &featuresQR = query.requiredFeatures.features();
            for (auto const &feature : features) {
                if (!featuresQO.contains(feature) && !featuresQR.contains(feature)) {
                    return 0;
                }
            }

            return score;
        }

        bool empty() const {
            FeatureValue value;
            value.emplace<void*>(nullptr);
            std::hash<FeatureValue>()(value);
            return m_style == DrawingStyle::numberDrawingStyles && m_features.features().empty();
        }

        bool operator==(Description const &d) const {
            return m_style == d.m_style && m_features == d.m_features;
        }

        bool operator!=(Description const &d) const {
            return !(*this == d);
        }

        bool operator<(Description const &d) const {
            if (m_style != d.m_style) {
                return m_style < d.m_style;
            } else {
                return m_features < d.m_features;
            }
        }

        Description()
                : m_style(DrawingStyle::numberDrawingStyles),
                  m_features()
        {}

        Description(DrawingStyle style)
                : m_style(style),
                  m_features() {}

        Description(DrawingStyle style,
                    std::vector<FeatureType> const &inFeatures)
                : m_style(style),
                  m_features(inFeatures) {}

    private:
        std::size_t m_style;
        FeatureList m_features;
    };
}

/*
struct HashFeatureValue : public boost::static_visitor<size_t> {
    size_t operator()(size_t value) {
        return std::hash<size_t>()(value);
    }

    size_t operator()(void *value) {
        return std::hash<void *>()(value);
    }

    HashFeatureValue() = default;
};
namespace std {
    template <>
    struct hash<renderDetails::FeatureList> {
        size_t operator()(renderDetails::FeatureList const &feature) const {
            return std::hash<renderDetails::FeatureList::FeatureListInternal>()(feature.features());
        }
    };
}
 */


struct GraphicsDescription {
public:
    std::string m_graphicsName;
    std::string m_version;
    std::string m_deviceName;
    std::vector<std::string> m_extraInfo;

    GraphicsDescription(std::string inGraphicsName, std::string inVersion, std::string inDeviceName,
            std::vector<std::string> &&inExtraInfo)
        : m_graphicsName{std::move(inGraphicsName)},
          m_version{std::move(inVersion)},
          m_deviceName{std::move(inDeviceName)},
          m_extraInfo{std::move(inExtraInfo)}
    {
    }
};

class FileRequester {
public:
    virtual std::unique_ptr<std::streambuf> getAssetStream(std::string const &file) = 0;
    virtual std::unique_ptr<std::streambuf> getLevelTableAssetStream() = 0;
    virtual std::string getSaveDataFileName() = 0;
    virtual ~FileRequester() = default;
};

class JRequester {
public:
    virtual void sendError(std::string const &error) = 0;
    virtual void sendError(char const *error) = 0;
    virtual void sendGraphicsDescription(GraphicsDescription const &description,
                                         bool hasAccelerometer, bool isVulkanImplementation) = 0;
    virtual void sendKeepAliveEnabled(bool keepAliveEnabled) = 0;
    virtual std::vector<char> getTextImage(std::string text, uint32_t &width, uint32_t &height,
            uint32_t &channels) = 0;

    virtual ~JRequester() = default;
};

class GameRequester :public FileRequester, public JRequester {
public:
    ~GameRequester() override = default;
};

std::vector<char> readFile(std::shared_ptr<FileRequester> const &requester, std::string const &filename);
void checkGraphicsError();

template <typename ProtectedType, typename Enable = void> class ReferenceCountedInteger;

template <typename HandleObject>
class ReferenceCountedInteger<HandleObject, typename std::enable_if<std::is_integral<HandleObject>::value>::type> {
public:
    using DeleterType = std::function<void(HandleObject)>;

    auto const *get() const
    {
        if (m_data != nullptr) {
            return &(m_data->value);
        } else {
            return &defaultValue;
        }
    }

    ReferenceCountedInteger() : m_data{nullptr} { }

    ReferenceCountedInteger(HandleObject value, DeleterType deleter)
            : m_data{new Data(value, deleter)}
    { }

    ReferenceCountedInteger(ReferenceCountedInteger const &other)
            : m_data{other.m_data}
    {
        if (m_data != nullptr) {
            m_data->referenceCount++;
        }
    }

    ReferenceCountedInteger(ReferenceCountedInteger &&other)
        : m_data{nullptr}
    {
        using std::swap;

        swap(m_data, other.m_data);
    }

    ReferenceCountedInteger &operator=(ReferenceCountedInteger const &other) {
        using std::swap;

        ReferenceCountedInteger tmp(other);

        swap(m_data, tmp.m_data);

        return *this;
    }

    ReferenceCountedInteger &operator=(ReferenceCountedInteger &&other) {
        using std::swap;

        swap(m_data, other.m_data);

        return *this;
    }

    ~ReferenceCountedInteger() {
        if (m_data == nullptr) {
            return;
        }

        auto newCount = -- m_data->referenceCount;

        assert(newCount >= 0);

        if (newCount == 0) {
            m_data->deleter(m_data->value);
            delete m_data;
        }
    }

private:
    static HandleObject constexpr const defaultValue = 0;

    struct Data {
        HandleObject value;
        DeleterType const deleter;
        std::atomic<int> referenceCount;

        Data(HandleObject value_, DeleterType deleter_) : value{value_}, deleter{deleter_}, referenceCount{1} {}
    };

    Data *m_data;
};

#endif // AMAZING_LABYRINTH_COMMON_HPP
