/**
 * Copyright 2023 Cerulean Quasar. All Rights Reserved.
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

#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <iomanip>
#include <json.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/noncopyable.hpp>

#include <glm/glm.hpp>

#include "../app/src/main/cpp/levelDrawer/modelTable/modelLoader.hpp"

enum ComponentType {
    SignedByte = 5120,
    UnsignedByte = 5121,
    SignedShort = 5122,
    UnsignedShort = 5123,
    UnsignedInt32 = 5125,
    Float = 5126
};

using Attributes = std::map<std::string, uint32_t>;

struct Primitive {
    Attributes attr;
    uint32_t indices;
};

struct Mesh {
    std::string name;
    std::vector<Primitive> primitives;
};

struct Accessor {
    uint32_t bufferView;
    uint32_t count;
    ComponentType componentType;
    std::string type;
};

struct BufferView {
    uint32_t buffer;
    uint32_t byteLength;
    uint32_t byteOffset;
};

struct Buffer {
    uint32_t byteLength;
};

struct GLBJSON {
    std::vector<Mesh> meshes;
    std::vector<Accessor> accessors;
    std::vector<BufferView> bufferViews;
    std::vector<Buffer> buffers;
};

void from_json(nlohmann::json const &j, Primitive &val) {
    val.attr = j.at("attributes").get<Attributes>();
    val.indices = j.at("indices").get<uint32_t>();
}

void from_json(nlohmann::json const&j, Mesh &val) {
    val.name = j.at("name").get<std::string>();
    val.primitives = j.at("primitives").get<std::vector<Primitive>>();
}

void from_json(nlohmann::json const&j, Accessor &val) {
    val.bufferView = j.at("bufferView").get<uint32_t>();
    val.count = j.at("count").get<uint32_t>();
    val.componentType = j.at("componentType").get<ComponentType>();
    val.type = j.at("type").get<std::string>();
}

void from_json(nlohmann::json const&j, BufferView &val) {
    val.buffer = j.at("buffer").get<uint32_t>();
    val.byteLength = j.at("byteLength").get<uint32_t>();
    val.byteOffset = j.at("byteOffset").get<uint32_t>();
}

void from_json(nlohmann::json const&j, Buffer &val) {
    val.byteLength = j.at("byteLength").get<uint32_t>();
}

void from_json(nlohmann::json const&j, GLBJSON &val) {
    val.meshes = j.at("meshes").get<std::vector<Mesh>>();
    val.accessors = j.at("accessors").get<std::vector<Accessor>>();
    val.bufferViews = j.at("bufferViews").get<std::vector<BufferView>>();
    val.buffers = j.at("buffers").get<std::vector<Buffer>>();
}

using BufferResult = boost::variant<
        std::vector<glm::vec2>,
        std::vector<glm::vec3>,
        std::vector<glm::vec4>,
        std::vector<glm::u8vec3>,
        std::vector<glm::u8vec4>,
        std::vector<glm::u16vec3>,
        std::vector<glm::u16vec4>,
        std::vector<glm::u32vec3>,
        std::vector<glm::u32vec4>,
        std::vector<int8_t>,
        std::vector<uint8_t>,
        std::vector<int16_t>,
        std::vector<uint16_t>,
        std::vector<uint32_t>>;

struct ChunkHeader {
    uint32_t length;
    uint32_t chunkType;
};

struct Header {
    uint32_t magic;
    uint32_t version;
    uint32_t length;
};

static char constexpr const *TypeScalar = "SCALAR";
static char constexpr const *TypeVec2 = "VEC2";
static char constexpr const *TypeVec3 = "VEC3";
static char constexpr const *TypeVec4 = "VEC4";

static char constexpr const *AttrPosition = "POSITION";
static char constexpr const *AttrNormal = "NORMAL";
static char constexpr const *AttrTexcoord_0 = "TEXCOORD_0";
static char constexpr const *AttrColor_0 = "COLOR_0";

enum PropertyType {
    POSITION,
    NORMAL,
    TEXCOORD0,
    COLOR0,
    INDICES
};

size_t getComponentTypeSize(ComponentType ct) {
    switch (ct) {
        case SignedByte:
            return sizeof (int8_t);
        case UnsignedByte:
            return sizeof (uint8_t);
        case SignedShort:
            return sizeof (short);
        case UnsignedShort:
            return sizeof (unsigned short);
        case UnsignedInt32:
            return sizeof (uint32_t);
        default:
            throw std::runtime_error("Invalid component type.");
    }
}

template <typename T> struct Identity {
    using type = T;
    static size_t length() { return 1; }
};

template <glm::length_t D>
struct Vector {
    template <typename T>
    struct conversion {
        using type = glm::vec<D, T, std::is_floating_point<T>::value ? glm::qualifier(0) : glm::defaultp>;
        static size_t length() { return D; }
    };
};

template <typename type>
inline void doByteSwap (type *buffer, size_t nbrTypes) {
    if (sizeof (type) > 1 && boost::endian::order::native == boost::endian::order::big) {
        for (size_t i = 0; i < nbrTypes; i++) {
            boost::endian::endian_reverse_inplace(buffer[i]);
        }
    }
}

template <>
inline void doByteSwap (float *buffer, size_t nbrTypes) {
    if (sizeof (float) > 1 && boost::endian::order::native == boost::endian::order::big) {
        for (size_t i = 0; i < nbrTypes; i++) {
            throw std::runtime_error("Byte swap: don't know how to byte swap floats");
        }
    }
}

template <template <typename T> typename Conversion, typename Visitor>
typename Visitor::result_type
visit(ComponentType componentType, Visitor& visitor) {
    switch (componentType) {
        case ComponentType::SignedByte:
            return visitor.template operator()<Conversion, ComponentType::SignedByte>();
        case ComponentType::UnsignedByte:
            return visitor.template operator()<Conversion, ComponentType::UnsignedByte>();
        case ComponentType::SignedShort:
            return visitor.template operator()<Conversion, ComponentType::SignedShort>();
        case ComponentType::UnsignedShort:
            return visitor.template operator()<Conversion, ComponentType::UnsignedShort>();
        case ComponentType::UnsignedInt32:
            return visitor.template operator()<Conversion, ComponentType::UnsignedInt32>();
        case ComponentType::Float:
            return visitor.template operator()<Conversion, ComponentType::Float>();
        default:
            throw std::runtime_error("Improper type or component type.");
    }
}

template <ComponentType componentType> struct componentTypeRep;
template <> struct componentTypeRep<ComponentType::SignedByte> { using type = int8_t; };
template <> struct componentTypeRep<ComponentType::UnsignedByte> { using type = uint8_t; };
template <> struct componentTypeRep<ComponentType::SignedShort> { using type = int16_t; };
template <> struct componentTypeRep<ComponentType::UnsignedShort> { using type = uint16_t; };
template <> struct componentTypeRep<ComponentType::UnsignedInt32> { using type = uint32_t; };
template <> struct componentTypeRep<ComponentType::Float> { using type = float; };

class FillVisitor : private boost::noncopyable {
public:
    using result_type = BufferResult;

    FillVisitor(uint32_t count, size_t byteOffset, size_t buffer2StartPos, std::istream &is)
    : m_count{count},
      m_byteOffset{byteOffset},
      m_buffer2StartPos{buffer2StartPos},
      m_is{is} {}

    template <template <typename T> typename Conversion, ComponentType componentType>
    result_type operator()() {
        FillHelper<typename componentTypeRep<componentType>::type,
                typename Conversion<typename componentTypeRep<componentType>::type>::type> fillHelper(
                        Conversion<typename componentTypeRep<componentType>::type>::length(),
                        componentType, typename Conversion<typename componentTypeRep<componentType>::type>::type());
        return fillHelper.fill(m_count, m_byteOffset, m_buffer2StartPos, m_is);
    }

private:
    template <typename ComponentTypeName, typename VecType, typename Enable = void>
    class FillHelper {
    public:
        result_type fill(uint32_t, size_t, size_t, std::istream &) {
            throw std::runtime_error("GLB: improper type or component type.  Vector length: " +
                std::to_string(m_len) + ", Component Type: " + std::to_string(m_componentType));
        }

        FillHelper(size_t len, ComponentType componentType, VecType)
        : m_len(len),
          m_componentType(componentType) {}
    private:
        size_t m_len;
        ComponentType m_componentType;
    };

    template <typename ComponentTypeName, typename VecType>
    class FillHelper<ComponentTypeName, VecType, typename std::enable_if<std::is_constructible_v<result_type, std::vector<VecType>>>::type>{
    public:
        result_type fill(uint32_t count, size_t byteOffset, size_t buffer2StartPos, std::istream &is)
        {
            std::vector<VecType> data(count);
            is.seekg(buffer2StartPos + byteOffset);
            is.read(reinterpret_cast<char *>(data.data()), count * sizeof(VecType));
            doByteSwap(reinterpret_cast<ComponentTypeName *> (data.data()),
                       data.size() * sizeof(VecType) /
                       sizeof(ComponentTypeName));
            return data;
        }

        FillHelper(size_t, ComponentType, VecType) {}
    };

    uint32_t m_count;
    size_t m_byteOffset;
    size_t m_buffer2StartPos;
    std::istream &m_is;
};

class TextureVisitor : public boost::static_visitor<void> {
public:
    template <typename InputVectorType>
    void operator()(std::vector<InputVectorType> const &) {
        throw std::runtime_error("GLB: Invalid type or component type for texture coordinates.");
    }


    explicit TextureVisitor(std::vector<glm::vec2> &outputVector, PropertyType propertyType)
        : m_outputVector{outputVector},
          m_propertyType{propertyType}
    { }

private:
    std::vector<glm::vec2> &m_outputVector;
    PropertyType m_propertyType;
};

template <>
void TextureVisitor::operator()(std::vector<glm::vec2> const &inputVector) {
    if (m_propertyType != PropertyType::TEXCOORD0) {
        throw std::runtime_error("GLB: Invalid type or component type for texture coordinate.");
    }

    m_outputVector = inputVector;
}

class PositionNormalVisitor : public boost::static_visitor<void> {
public:
    template <typename InputVectorType>
    void operator()(std::vector<InputVectorType> const &) {
        throw std::runtime_error("GLB: Invalid type or component type for position, normal, or texture coordinates.");
    }

    explicit PositionNormalVisitor(std::vector<glm::vec3> &output, PropertyType propertyType)
    : m_output{output},
      m_propertyType{propertyType}
    {}

private:
    std::vector<glm::vec3> &m_output;
    PropertyType m_propertyType;
};

template <>
void PositionNormalVisitor::operator()(std::vector<glm::vec3> const &inputVector) {
    if (m_propertyType == PropertyType::POSITION || m_propertyType == PropertyType::NORMAL) {
        m_output = inputVector;
    } else {
        throw std::runtime_error("GLB: Invalid type or component type for texture coordinates.");
    }
}

class ColorVisitor : public boost::static_visitor<void> {
public:
    template <glm::length_t vecNumElements, typename memberType, glm::qualifier qualifier>
    void operator()(std::vector<glm::vec<vecNumElements, memberType, qualifier>> const &inputVector) {
        Helper<vecNumElements, memberType, qualifier> helper(m_output);
        helper.doStuff(inputVector);
    }

    template <typename ScalarType>
    void operator()(std::vector<ScalarType> const &) {
        throw std::runtime_error("GLB: Invalid type for color.");
    }

    explicit ColorVisitor(std::vector<glm::vec4> &output)
            : m_output(output) { }

private:
    template <glm::length_t vecNumElements, typename memberType, glm::qualifier qualifier, typename Enable = void>
    class Helper {
    public:
        void doStuff(std::vector<glm::vec<vecNumElements, memberType, qualifier>> const &) {
            throw std::runtime_error("GLB: Invalid type for color.");
        }

        explicit Helper(std::vector<glm::vec4> &) {}
    };

    template <glm::length_t vecNumElements, typename memberType, glm::qualifier qualifier>
    class Helper <vecNumElements, memberType, qualifier, typename std::enable_if<vecNumElements!=2>::type> {
    public:
        void doStuff(std::vector<glm::vec<vecNumElements, memberType, qualifier>> const &inputVector) {
            memberType max = std::numeric_limits<memberType>::max();
            for (auto input: inputVector) {
                glm::vec4 out{0.0f, 0.0f, 0.0f, 1.0f};
                for (glm::length_t j = 0; j < vecNumElements; j ++) {
                    out[j] = input[j] / static_cast<float>(max);
                }
                m_output.push_back(out);
            }
        }

        explicit Helper(std::vector<glm::vec4> &output)
        : m_output(output) { }

    private:
        std::vector<glm::vec4> &m_output;
    };

    template <glm::length_t vecNumElements, glm::qualifier qualifier>
    class Helper <vecNumElements, float, qualifier, typename std::enable_if_t<vecNumElements!=2>::type> {
    public:
        void doStuff(std::vector<glm::vec<vecNumElements, float, qualifier>> const &inputVector) {
            for (auto input : inputVector) {
                glm::vec4 out{0.0f, 0.0f, 0.0f, 1.0f};
                for (glm::length_t j = 0; j < vecNumElements; j ++) {
                    out[j] = input[j];
                }
                m_output.push_back(out);
            }
        }
        explicit Helper(std::vector<glm::vec4> &output)
                : m_output(output) { }

    private:
        std::vector<glm::vec4> &m_output;
    };

    std::vector<glm::vec4> &m_output;
};

class IndicesVisitor : public boost::static_visitor<void> {
public:
    template <glm::length_t vecNumElements, typename memberType, glm::qualifier qualifier>
    void operator()(std::vector<glm::vec<vecNumElements, memberType, qualifier>> const &) {
        throw std::runtime_error("GLB: Invalid type when reading indices.");
    }

    template <typename ScalarType>
    void operator()(std::vector<ScalarType> const &inputVector) {
        if (std::is_floating_point<ScalarType>::value) {
            throw std::runtime_error("GLB: Invalid component type for indices");
        }

        m_output.resize(inputVector.size());
        for (size_t i = 0; i < inputVector.size(); i++) {
            m_output[i] = inputVector[i];
        }
    }

    explicit IndicesVisitor(std::vector<uint32_t> &output)
    : m_output{output} {}
private:
    std::vector<uint32_t> &m_output;
};

template <>
void IndicesVisitor::operator()(std::vector<uint32_t> const &inputVector) {
    m_output = inputVector;
}

template <glm::length_t D, typename T, glm::qualifier Q>
void insertIntoIndicesAndArray(
    glm::vec<D, T, Q> const &attr,
    std::vector<T> &attrs,
    std::unordered_map<glm::vec<D, T, Q>, uint32_t> &uniqueAttrs,
    std::vector<uint32_t> &indices)
{
    auto insertResult = uniqueAttrs.emplace(attr, attrs.size()/D);
    if (insertResult.second) {
        for (glm::length_t d = 0; d < D; d++) {
            attrs.push_back(attr[d]);
        }
    }

    indices.push_back(insertResult.first->second);
}

void loadModelFromGlb(
    std::ifstream &modelStream,
    std::vector<float> &positions,
    std::vector<float> &faceNormals,
    std::vector<float> &vertexNormals,
    std::vector<float> &texcoords,
    std::vector<float> &colors,
    std::vector<std::vector<uint32_t>> &indices)
{
    std::cout << "Processing GLB file" << std::endl;

    std::istream &is{modelStream};
    Header header{};

    is.read(reinterpret_cast<char*>(&header), sizeof (Header));
    doByteSwap(reinterpret_cast<uint32_t*>(&header), sizeof (Header)/sizeof (uint32_t));
    if (header.magic != 0x46546C67) {
        throw std::runtime_error("Invalid magic in header.");
    }

    /* decode the first Chunk.  This chunk should be JSON that describes the second,
     * binary chunk.
     */
    ChunkHeader chunk1{};
    is.read(reinterpret_cast<char*>(&chunk1), sizeof (ChunkHeader));
    doByteSwap(reinterpret_cast<uint32_t*>(&chunk1), sizeof (ChunkHeader)/sizeof (uint32_t));
    if (chunk1.chunkType != 0x4E4F534A) {
        throw std::runtime_error("Invalid chunk type for type of first chunk");
    }

    std::vector<char> jsonText;
    jsonText.resize(chunk1.length);
    is.read(jsonText.data(), static_cast<std::streamsize>(chunk1.length));

    ChunkHeader chunk2{};
    is.read(reinterpret_cast<char*>(&chunk2), sizeof (ChunkHeader));
    doByteSwap(reinterpret_cast<uint32_t*>(&chunk2), sizeof (ChunkHeader)/sizeof (uint32_t));
    if (chunk2.chunkType != 0x004E4942) {
        throw std::runtime_error("Invalid chunk type for type of second chunk");
    }

    nlohmann::json chunk1JSON = nlohmann::json::parse(jsonText);

    GLBJSON glbjson;
    try {
        glbjson = chunk1JSON.get<GLBJSON>();
    } catch (nlohmann::json::exception &e) {
        throw std::runtime_error(e.what());
    }

    if (glbjson.meshes.empty()) {
        throw std::runtime_error("No meshes contained in GLB file.");
    }

    /* Only support one mesh for now */
    if (glbjson.meshes[0].primitives.empty()) {
        throw std::runtime_error("No primitives for mesh");
    }

    std::map<PropertyType, uint32_t> attr2accessor;
    for (auto const &attr : glbjson.meshes[0].primitives[0].attr) {
        if (attr.first == AttrPosition) {
            attr2accessor.emplace(PropertyType::POSITION, attr.second);
        } else if (attr.first == AttrNormal) {
            attr2accessor.emplace(PropertyType::NORMAL, attr.second);
        } else if (attr.first == AttrTexcoord_0) {
            attr2accessor.emplace(PropertyType::TEXCOORD0, attr.second);
        } else if (attr.first == AttrColor_0) {
            attr2accessor.emplace(PropertyType::COLOR0, attr.second);
        } else {
            /* don't know what to do with this accessor, skip silently. */
            std::cout << "WARNING: don't know what to do with attribute: " << attr.first
                      << " Skipping.\n";
        }
    }
    attr2accessor.emplace(PropertyType::INDICES, glbjson.meshes[0].primitives[0].indices);

    size_t currentPos = is.tellg();
    std::vector<glm::vec3> intermediatePositions;
    std::vector<glm::vec3> intermediateVertexNormals;
    std::vector<glm::vec2> intermediateTextureCoordinates;
    std::vector<glm::vec4> intermediateColors;
    std::vector<uint32_t> intermediateIndices;
    for (auto const &acc : attr2accessor) {
        ComponentType componentType = glbjson.accessors[acc.second].componentType;
        std::string type = glbjson.accessors[acc.second].type;
        size_t bufferView = glbjson.accessors[acc.second].bufferView;

        BufferResult vertexData;
        if (type == TypeScalar) {
            FillVisitor visitor(glbjson.accessors[acc.second].count, glbjson.bufferViews[bufferView].byteOffset, currentPos, is);
            vertexData = visit<Identity>(componentType, visitor);
        } else if (type == TypeVec2) {
            FillVisitor visitor(glbjson.accessors[acc.second].count, glbjson.bufferViews[bufferView].byteOffset, currentPos, is);
            vertexData = visit<Vector<2>::conversion>(componentType, visitor);
        } else if (type == TypeVec3) {
            FillVisitor visitor(glbjson.accessors[acc.second].count, glbjson.bufferViews[bufferView].byteOffset, currentPos, is);
            vertexData = visit<Vector<3>::conversion>(componentType, visitor);
        } else if (type == TypeVec4) {
            FillVisitor visitor(glbjson.accessors[acc.second].count, glbjson.bufferViews[bufferView].byteOffset, currentPos, is);
            vertexData = visit<Vector<4>::conversion>(componentType, visitor);
        } else {
            throw std::runtime_error("Invalid data in GLB file.");
        }

        switch (acc.first) {
        case PropertyType::POSITION: {
            PositionNormalVisitor pntVisitor(intermediatePositions, PropertyType::POSITION);
            boost::apply_visitor(pntVisitor, vertexData);
            break;
        }
        case PropertyType::NORMAL: {
            PositionNormalVisitor pntVisitor(intermediateVertexNormals, PropertyType::NORMAL);
            boost::apply_visitor(pntVisitor, vertexData);
            break;
        }
        case PropertyType::TEXCOORD0: {
            TextureVisitor textureCoordinatesVisitor(intermediateTextureCoordinates, PropertyType::TEXCOORD0);
            boost::apply_visitor(textureCoordinatesVisitor, vertexData);
            break;
        }
        case PropertyType::COLOR0: {
            ColorVisitor colorVisitor(intermediateColors);
            boost::apply_visitor(colorVisitor, vertexData);
            break;
        }
        case PropertyType::INDICES: {
            IndicesVisitor indicesVisitor(intermediateIndices);
            boost::apply_visitor(indicesVisitor, vertexData);
            break;
        }
        }
    }

    std::cout << "number of intermediate positions: " << intermediatePositions.size() << std::endl;
    std::cout << "number of intermediate vertex normals: " << intermediateVertexNormals.size() << std::endl;
    std::cout << "number of intermediate texture coordinates: " << intermediateTextureCoordinates.size() << std::endl;
    std::cout << "number of intermediate colors: " << intermediateColors.size() << std::endl;
    std::cout << "number of intermediate indices: " << intermediateIndices.size() << std::endl;

    /* build the flattened vertex position, face normals, vertex normals, texture coordinates, color
     * and indices arrays.  There is one indices array whose members point to each of the other vertex
     * attribute array entries.  Here is the order they are stored in the index array.
     *
     * position / face normal / vertex normal / textureCoord / color
     *
     * The attribute arrays are passed back in flattened form.  That is if the attribute has 4 values, then
     * the attribute array would contain entries like:
     *
     * vi1 vi2 vi3 vi4 vj1 vj2 vj3 vj4...
     */
    std::unordered_map<glm::vec3, uint32_t> uniquePositions;
    std::unordered_map<glm::vec4, uint32_t> uniqueColors;
    std::unordered_map<glm::vec2, uint32_t> uniqueTextureCoords;
    std::unordered_map<glm::vec3, uint32_t> uniqueFaceNormals;
    std::unordered_map<glm::vec3, uint32_t> uniqueVertexNormals;
    std::vector<uint32_t> indicesOneShape;
    for (size_t i = 0; i < intermediateIndices.size(); i += 3) {
        /* find the face normal for the vertices in each triangle */
        std::array<levelDrawer::Vertex, 3> v;
        v[0].pos = intermediatePositions[intermediateIndices[i]];
        v[0].normal = intermediateVertexNormals[intermediateIndices[i]];
        v[1].pos = intermediatePositions[intermediateIndices[i + 1]];
        v[1].normal = intermediateVertexNormals[intermediateIndices[i + 1]];
        v[2].pos = intermediatePositions[intermediateIndices[i + 2]];
        v[2].normal = intermediateVertexNormals[intermediateIndices[i + 2]];
        glm::vec3 vertexNormalAverageDirection = v[0].normal + v[1].normal + v[2].normal;
        glm::vec3 faceNormal = glm::normalize(glm::cross(v[1].pos - v[0].pos, v[2].pos - v[0].pos));
        float dot = glm::dot(vertexNormalAverageDirection, faceNormal);
        if (dot < 0) {
            faceNormal = -faceNormal;
        }

        auto insertResultFN = uniqueFaceNormals.emplace(faceNormal, faceNormals.size()/3);
        if (insertResultFN.second) {
            faceNormals.push_back(faceNormal.x);
            faceNormals.push_back(faceNormal.y);
            faceNormals.push_back(faceNormal.z);
        }

        for (size_t j = 0; j < v.size(); j++) {
            insertIntoIndicesAndArray(v[j].pos, positions, uniquePositions, indicesOneShape);

            indicesOneShape.push_back(insertResultFN.first->second);

            insertIntoIndicesAndArray(v[j].normal, vertexNormals, uniqueVertexNormals, indicesOneShape);
            if (!intermediateTextureCoordinates.empty()) {
                insertIntoIndicesAndArray(intermediateTextureCoordinates[intermediateIndices[i+j]],
                    texcoords, uniqueTextureCoords, indicesOneShape);
            }
            if (!intermediateColors.empty()) {
                insertIntoIndicesAndArray(intermediateColors[intermediateIndices[i+j]],
                    colors, uniqueColors, indicesOneShape);
            }
        }
    }

    indices.push_back(indicesOneShape);
}
