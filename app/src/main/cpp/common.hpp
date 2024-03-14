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
#include <array>
#include <string>
#include <memory>
#include <streambuf>
#include <functional>
#include <atomic>
#include <cassert>

namespace renderDetails {
    enum Features {
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
        dark1light,
        dark2lights,
        darkV2,
        depthMap,
        normalMap,
        numberDrawingStyles
    };

    class FeatureList {
        using RawFeatureList = std::vector<bool>;
    public:
        FeatureList()
        : m_features(Features::numberFeatures, false) {}

        FeatureList(std::vector<Features> const &features)
        : m_features(Features::numberFeatures, false)
        {
            for (auto const &feature : features) {
                m_features[feature] = true;
            }
        }

        size_t getHash() const {
            return std::hash<RawFeatureList>()(m_features);
        }

        void bitwiseAnd(FeatureList const &other) {
            for (size_t i = 0; i < Features::numberFeatures; i++) {
                m_features[i] = m_features[i] && other.m_features[i];
            }
        }

        void bitwiseOr(FeatureList const &other) {
            for (size_t i = 0; i < Features::numberFeatures; i++) {
                m_features[i] = m_features[i] || other.m_features[i];
            }
        }

        void setFeature(Features feature, bool value = true) {
            m_features[feature] = value;
        }

        void setFeature(std::vector<Features> features, bool value = true) {
            for (auto const &feature : features) {
                m_features[feature] = value;
            }
        }

        bool getFeatureValue(Features feature) const {
            return m_features[feature];
        }

        bool operator==(FeatureList const &other) const {
            return m_features == other.m_features;
        }

        bool operator!=(FeatureList const &other) const {
            return m_features != other.m_features;
        }

        bool operator<(FeatureList const &other) const {
            size_t selfScore = 0;
            size_t i = 1;
            for (auto const &feature : m_features) {
                selfScore += i * feature;
                i++;
            }

            size_t otherScore = 0;
            i = 1;
            for (auto const &feature : other.m_features) {
                otherScore += i * feature;
                i++;
            }

            return selfScore < otherScore;
        }
    private:
        RawFeatureList m_features;
    };

    struct Query {
    public:
        Query(DrawingStyle inStyle)
            :style(inStyle),
            requiredFeatures(),
            optionalFeatures() {}

        Query(
                DrawingStyle inStyle,
                std::vector<Features> const &inRequiredFeatures,
                std::vector<Features> const &inOptionalFeatures)
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

        DrawingStyle style;
        FeatureList requiredFeatures;
        FeatureList optionalFeatures;
    };

    class Description {
    public:
        static Description const &empty() {
            static const Description emptyDescription{};
            return emptyDescription;
        }

        Description()
            : m_style(DrawingStyle::numberDrawingStyles),
              m_features()
        {}

        Description(DrawingStyle style)
            : m_style(style),
              m_features() {}

        Description(DrawingStyle style,
                    std::vector<Features> const &inFeatures)
                : m_style(style),
                  m_features(inFeatures) {}

        auto drawingMethod() const { return m_style; }

        auto const &features() const { return m_features; }

        unsigned int getMatchPotential(Query const &query) {
            if (query.style != m_style) {
                return 0;
            }

            unsigned int score = 1;
            for (size_t i = 0; i < Features::numberFeatures; i++) {
                if (m_features.getFeatureValue(static_cast<Features>(i)))
                {
                    if (query.requiredFeatures.getFeatureValue(static_cast<Features>(i))) {
                        continue;
                    } else if (query.optionalFeatures.getFeatureValue(static_cast<Features>(i))) {
                        score++;
                        continue;
                    } else {
                        return 0;
                    }
                } else if (query.requiredFeatures.getFeatureValue(static_cast<Features>(i))) {
                    return 0;
                }
            }

            return score;
        }

        bool operator==(Description const &other) const {
            if (m_style != other.m_style) {
                return false;
            } else if (m_features != other.m_features) {
                return false;
            }

            return true;
        }

        bool operator!=(Description const &other) const {
            return !(*this == other);
        }

        bool operator<(Description const &other) const {
            if (m_style < other.m_style) {
                return true;
            } else if (m_features < other.m_features) {
                return true;
            }

            return false;
        }

    private:
        DrawingStyle m_style;
        FeatureList m_features;
    };
}

namespace std {
    template<>
    struct hash<renderDetails::FeatureList> {
        size_t operator()(renderDetails::FeatureList const &features) const {
            return features.getHash();
        }
    };

    template <>
    struct hash<renderDetails::Description> {
        size_t operator()(renderDetails::Description const &description) const {
            return description.drawingMethod() ^
                (std::hash<renderDetails::FeatureList>()(description.features()) << 1);
        }
    };
}

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
