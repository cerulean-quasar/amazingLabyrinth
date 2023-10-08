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

#include <streambuf>
#include <boost/endian/conversion.hpp>
#include <boost/noncopyable.hpp>
#include <json.hpp>

#include "../../common.hpp"
#include "modelLoader.hpp"

namespace std {
    template<>
    struct hash<glm::vec3> {
        size_t operator()(glm::vec3 vector) const {
            return ((hash<float>()(vector.x) ^
                     (hash<float>()(vector.y) << 1)) >> 1) ^
                   (hash<float>()(vector.z) << 1);
        }
    };

    template<>
    struct hash<glm::vec2> {
        size_t operator()(glm::vec2 vector) const {
            return (hash<float>()(vector.x) ^ (hash<float>()(vector.y) << 1));
        }
    };

    template<>
    struct hash<levelDrawer::Vertex> {
        size_t operator()(levelDrawer::Vertex const &vertex) const {
            return ((((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1))
                    >> 1) ^
                     (hash<glm::vec2>()(vertex.texCoord) << 1)) >> 1) ^
                   (hash<glm::vec3>()(vertex.normal) << 1);
        }
    };
}

namespace levelDrawer {
    bool Vertex::operator==(const Vertex &other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord &&
               normal == other.normal;
    }

    bool compareLessVec3(glm::vec3 const &vec1, glm::vec3 const &vec2) {
        if (vec1.x != vec2.x) {
            return vec1.x < vec2.x;
        } else if (vec1.y != vec2.y) {
            return vec1.y < vec2.y;
        } else if (vec1.z != vec2.z) {
            return vec1.z < vec2.z;
        }

        return false;
    }

    class LoadModelSaxClass {
    public:
        LoadModelSaxClass()
                : m_state{state::receivingKey},
                  m_color{0.2f, 0.2f, 0.2f},
                  m_substate{receivingArrayStart}{}

        explicit LoadModelSaxClass(glm::vec3 color)
                : m_state{state::receivingKey},
                m_color{color},
                m_substate{receivingArrayStart}{}

        void getVertices(
                std::pair<std::vector<Vertex>, std::vector<uint32_t>> &verticesWithFaceNormals,
                std::pair<std::vector<Vertex>, std::vector<uint32_t>> *verticesWithVertexNormals) {
            size_t size = 0;
            for (auto const &indices : m_indices) {
                size += indices.size();
            }
            std::unordered_map<Vertex, uint32_t> uniqueVertices{};
            uniqueVertices.reserve(size);
            std::unordered_map<Vertex, uint32_t> uniqueVerticesWithVertexNormals{};
            uniqueVerticesWithVertexNormals.reserve(size);
            for (auto const &indices : m_indices) {
                std::unordered_map<glm::vec3, glm::vec3> vertexNormals;
                vertexNormals.reserve(indices.size());
                if (verticesWithVertexNormals) {
                    for (size_t i = 0; i < indices.size() / 3; i++) {
                        glm::vec3 pos = {
                                m_vertices[3 * indices[i * 3 + 0] + 0],
                                m_vertices[3 * indices[i * 3 + 0] + 1],
                                m_vertices[3 * indices[i * 3 + 0] + 2]
                        };

                        glm::vec3 normal = {m_normals[3 * indices[i * 3 + 1] + 0],
                                            m_normals[3 * indices[i * 3 + 1] + 1],
                                            m_normals[3 * indices[i * 3 + 1] + 2]};

                        auto it = vertexNormals.find(pos);
                        if (it == vertexNormals.end()) {
                            vertexNormals.insert(std::make_pair(pos, normal));
                        } else {
                            it->second += normal;
                        }
                    }
                }

                for (size_t i = 0; i < indices.size() / 3; i++) {
                    Vertex vertex = {};
                    vertex.pos = {
                            m_vertices[3 * indices[i * 3 + 0] + 0],
                            m_vertices[3 * indices[i * 3 + 0] + 1],
                            m_vertices[3 * indices[i * 3 + 0] + 2]
                    };

                    if (!m_texCoords.empty()) {
                        vertex.texCoord = {
                                m_texCoords[2 * indices[i * 3 + 2] + 0],
                                1.0f - m_texCoords[2 * indices[i * 3 + 2] + 1]
                        };
                    } else {
                        vertex.texCoord = {0.0f, 0.0f};
                    }

                    vertex.color = m_color;

                    if (verticesWithVertexNormals) {
                        Vertex vertexWithVertexNormal = vertex;
                        auto it = vertexNormals.find(vertexWithVertexNormal.pos);
                        if (it == vertexNormals.end()) {
                            throw std::runtime_error("Vertex normal not found when loading model");
                        }
                        vertexWithVertexNormal.normal = it->second;

                        /*
                        auto itUniqueVerticesWithVertexNormals = uniqueVerticesWithVertexNormals.find(vertexWithVertexNormal);
                        if (itUniqueVerticesWithVertexNormals == uniqueVerticesWithVertexNormals.end()) {
                            auto itPair = uniqueVerticesWithVertexNormals.emplace(vertexWithVertexNormal,
                                    static_cast<uint32_t>(verticesWithVertexNormals->first.size()));
                            itUniqueVerticesWithVertexNormals = itPair.first;
                            verticesWithVertexNormals->first.push_back(vertexWithVertexNormal);
                        }*/
                        auto itPair = uniqueVerticesWithVertexNormals.emplace(
                                vertexWithVertexNormal,
                                static_cast<uint32_t>(verticesWithVertexNormals->first.size()));

                        if (itPair.second) {
                            verticesWithVertexNormals->first.push_back(vertexWithVertexNormal);
                        }
                        verticesWithVertexNormals->second.push_back(itPair.first->second);
                    }

                    vertex.normal = {m_normals[3 * indices[i * 3 + 1] + 0],
                                     m_normals[3 * indices[i * 3 + 1] + 1],
                                     m_normals[3 * indices[i * 3 + 1] + 2]};

                    auto itPair2 = uniqueVertices.emplace(vertex,
                                                          static_cast<uint32_t>(verticesWithFaceNormals.first.size()));
                    if (itPair2.second) {
                        verticesWithFaceNormals.first.push_back(vertex);
                    }
                    verticesWithFaceNormals.second.push_back(itPair2.first->second);
                }
            }
        }

        // called when null is parsed
        static bool null() { return true; }

        // called when a boolean is parsed; value is passed
        static bool boolean(bool) { return true; }

        // called when a signed or unsigned integer number is parsed; value is passed
        bool number_integer(nlohmann::json::number_integer_t val) {
            // if the integer is less than 0, that is an unexpected value and we don't know what
            // to do with it, error out.
            if (val < 0) {
                return false;
            }
            return processUnsignedInteger(static_cast<uint32_t>(val));
        }

        bool number_unsigned(nlohmann::json::number_unsigned_t val) {
            return processUnsignedInteger(static_cast<uint32_t>(val));
        }

        // called when a floating-point number is parsed; value and original string is passed
        bool number_float(nlohmann::json::number_float_t val, nlohmann::json::string_t const &) {
            switch (m_state) {
                case state::receivingVertices:
                    m_vertices.push_back(val);
                    break;
                case state::receivingNormals:
                    m_normals.push_back(val);
                    break;
                case state::receivingTexCoords:
                    m_texCoords.push_back(val);
                    break;
                default:
                    break;
            }
            return true;
        }

        // called when a string is parsed; value is passed and can be safely moved away
        static bool string(nlohmann::json::string_t &) { return true; }

        // called when an object or array begins or ends, resp. The number of elements is passed (or -1 if not known)
        static bool start_object(std::size_t) { return true; }

        static bool end_object() { return true; }

        bool start_array(std::size_t arrsize) {
            if (m_state == state::receivingIndices) {
                if (m_substate == substate::receivingArrayStart) {
                    m_substate = substate::receivingArrayStart2;
                    m_indices.reserve(arrsize);
                } else if (m_substate == substate::receivingArrayStart2) {
                    m_substate = substate::receivingArray;
                    m_indices.emplace_back();
                    m_indices[m_indices.size() - 1].reserve(arrsize);
                }
            } else if (m_state == state::receivingVertices) {
                m_vertices.reserve(arrsize);
            } else if (m_state == state::receivingNormals) {
                m_normals.reserve(arrsize);
            } else if (m_state == state::receivingTexCoords) {
                m_texCoords.reserve(arrsize);
            }
            return true;
        }

        bool end_array() {
            if (m_state == state::receivingIndices) {
                if (m_substate == substate::receivingArray) {
                    m_substate = substate::receivingArrayStart2;
                } else if (m_substate == substate::receivingArrayStart2) {
                    m_substate = substate::receivingArrayStart;
                }
            }

            return true;
        }

        // called when an object key is parsed; value is passed and can be safely moved away
        bool key(nlohmann::json::string_t &val) {
            if (val == KeyVertices) {
                m_state = state::receivingVertices;
            } else if (val == KeyNormals) {
                m_state = state::receivingNormals;
            } else if (val == KeyTexCoords) {
                m_state = state::receivingTexCoords;
            } else if (val == KeyIndices) {
                m_state = state::receivingIndices;
                m_substate = substate::receivingArrayStart;
            }

            return true;
        }

        // called when a parse error occurs; byte position, the last token, and an exception is passed
        static bool parse_error(std::size_t /*position*/, const std::string & /*last_token*/,
                         const nlohmann::json::exception &ex) {
            throw std::runtime_error(ex.what());
        }

    private:
        bool processUnsignedInteger(uint32_t val) {
            if (m_state == state::receivingIndices && !m_indices.empty()) {
                m_indices[m_indices.size() - 1].push_back(val);
            }

            return true;
        }

        enum state {
            receivingKey = 0,
            receivingNormals = 1,
            receivingVertices = 2,
            receivingTexCoords = 3,
            receivingIndices = 4
        };

        enum substate {
            receivingArrayStart = 0,
            receivingArrayStart2 = 1,
            receivingArray = 2
        };

        state m_state;
        glm::vec3 m_color;
        substate m_substate;
        std::vector<std::vector<uint32_t>> m_indices;
        std::vector<float> m_normals;
        std::vector<float> m_vertices;
        std::vector<float> m_texCoords;
    };

    class ModelReaderCBOR : public ModelReader {
    public:
        bool read(std::unique_ptr<std::streambuf> const &modelStreamBuf,
                  ModelVertices &verticesWithFaceNormals,
                  ModelVertices *verticesWithVertexNormals) override
        {
            std::istream assetIstream(modelStreamBuf.get());

            LoadModelSaxClass sax{m_color};

            nlohmann::json j = nlohmann::json::sax_parse(assetIstream, &sax,
                                                         nlohmann::json::input_format_t::cbor);
            sax.getVertices(verticesWithFaceNormals, verticesWithVertexNormals);
            return true;
        }

        explicit ModelReaderCBOR(glm::vec3 const &color)
        : m_color{color} { }

    private:
        glm::vec3 m_color;
    };

    enum ComponentType {
        SignedByte = 5120,
        UnsignedByte = 5121,
        SignedShort = 5122,
        UnsignedShort = 5123,
        UnsignedInt32 = 5125,
        Float = 5126
    };

    struct Attributes {
        uint32_t position;
        uint32_t normal;
        uint32_t textureCoord;
        uint32_t color0;
    };

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

    void from_json(nlohmann::json const &j, Attributes &val) {
        val.position = j.at("POSITION").get<uint32_t>();
        val.normal = j.at("NORMAL").get<uint32_t>();
        val.textureCoord = j.at("TEXCOORD").get<uint32_t>();
        val.color0 = j.at("COLOR_0").get<uint32_t>();
    }

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

    class ModelReaderGLB : public ModelReader {
    public:
        bool read(std::unique_ptr<std::streambuf> const &modelStreamBuf,
                          ModelVertices &verticesWithFaceNormals,
                          ModelVertices *verticesWithVertexNormals) override
        {
            std::istream is{modelStreamBuf.get()};
            Header header{};
            bool byteSwap = boost::endian::order::native == boost::endian::order::big;

            is.read(reinterpret_cast<char*>(&header), sizeof (Header));
            doByteSwap(reinterpret_cast<uint32_t*>(&header), sizeof (Header)/sizeof (uint32_t));

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

            std::array<std::pair<PropertyType, uint32_t> , 5> accessorArray{
                            std::make_pair(PropertyType::POSITION, glbjson.meshes[0].primitives[0].attr.position),
                            std::make_pair(PropertyType::NORMAL, glbjson.meshes[0].primitives[0].attr.normal),
                            std::make_pair(PropertyType::TEXTURE, glbjson.meshes[0].primitives[0].attr.textureCoord),
                            std::make_pair(PropertyType::COLOR, glbjson.meshes[0].primitives[0].attr.color0),
                            std::make_pair(PropertyType::INDICES, glbjson.meshes[0].primitives[0].indices) };

            size_t currentPos = is.tellg();
            ModelVertices outVertexData;
            for (AccessorArrayMember acc : accessorArray) {
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
                callTranslationVisitor(acc.first, vertexData, outVertexData);
            }

            /* find the face normals for the vertices */
            std::unordered_map<Vertex, uint32_t> uniqueVertices;
            for (size_t i = 0; i < outVertexData.second.size(); i += 3) {
                std::array<Vertex, 3> v;
                v[0] = outVertexData.first[outVertexData.second[i]];
                v[1] = outVertexData.first[outVertexData.second[i + 1]];
                v[2] = outVertexData.first[outVertexData.second[i + 2]];
                glm::vec3 vertexNormalAverageDirection = v[0].normal + v[1].normal + v[2].normal;
                glm::vec3 faceNormal = glm::normalize(glm::cross(v[1].pos - v[0].pos, v[2].pos - v[0].pos));
                if (glm::dot(vertexNormalAverageDirection, faceNormal) < 0) {
                    faceNormal = -faceNormal;
                }
                for (size_t j = 0; j < v.size(); j++) {
                    Vertex vf(v[i].pos, v[i].color, v[i].texCoord,faceNormal);
                    auto insertResult = uniqueVertices.emplace(vf, verticesWithFaceNormals.first.size());
                    if (insertResult.second) {
                        verticesWithFaceNormals.first.push_back(vf);
                        verticesWithFaceNormals.second.push_back(
                                verticesWithFaceNormals.first.size() - 1);
                    } else {
                        verticesWithFaceNormals.second.push_back(insertResult.first->second);
                    }
                }
            }
            if (verticesWithVertexNormals != nullptr) {
                *verticesWithVertexNormals = std::move(outVertexData);
            }

            return true;
        }

        explicit ModelReaderGLB(glm::vec3 const &color)
                : m_color(color) { }
    private:
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

        static size_t getComponentTypeSize(ComponentType ct) {
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

        enum PropertyType {
            POSITION,
            NORMAL,
            TEXTURE,
            COLOR,
            INDICES
        };

        using AccessorArrayMember = std::pair<PropertyType, uint32_t>;

        void callTranslationVisitor(PropertyType vertexDatumType, BufferResult const &inVertexData, ModelVertices &outVertexData) {
            switch (vertexDatumType) {
            case PropertyType::POSITION: {
                PositionNormalTextureVisitor pntVisitor(m_color, outVertexData, PropertyType::POSITION);
                boost::apply_visitor(pntVisitor, inVertexData);
                return;
            }
            case PropertyType::NORMAL: {
                PositionNormalTextureVisitor pntVisitor(m_color, outVertexData, PropertyType::NORMAL);
                boost::apply_visitor(pntVisitor, inVertexData);
                return;
            }
            case PropertyType::TEXTURE: {
                PositionNormalTextureVisitor pntVisitor(m_color, outVertexData, PropertyType::TEXTURE);
                boost::apply_visitor(pntVisitor, inVertexData);
                return;
            }
            case PropertyType::COLOR: {
                ColorVisitor colorVisitor(outVertexData);
                boost::apply_visitor(colorVisitor, inVertexData);
                return;
            }
            case PropertyType::INDICES: {
                IndicesVisitor indicesVisitor(outVertexData);
                boost::apply_visitor(indicesVisitor, inVertexData);
                return;
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
                FillHelper<typename Conversion<typename componentTypeRep<componentType>::type>::type,
                        typename componentTypeRep<componentType>::type> fillHelper;
                return fillHelper.fill(m_count, m_byteOffset, m_buffer2StartPos, m_is);
            }

        private:
            template <typename VecType, typename ComponentTypeName, typename Enable = void>
            class FillHelper {
            public:
                result_type fill(uint32_t, size_t, size_t, std::istream &) {
                    throw std::runtime_error("GLB: improper type or component type.");
                }

                FillHelper() = default;
            };

            template <typename VecType, typename ComponentTypeName>
            class FillHelper<VecType, ComponentTypeName, std::enable_if<std::is_constructible_v<result_type, VecType>>>{
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

                FillHelper() = default;
            };

            uint32_t m_count;
            size_t m_byteOffset;
            size_t m_buffer2StartPos;
            std::istream &m_is;
        };

        template <typename T> struct Identity { using type = T; };

        template <glm::length_t D>
        struct Vector {
            template <typename T>
            struct conversion { using type = glm::vec<D, T, std::is_floating_point<T>::value ? glm::qualifier(0) : glm::defaultp>; };
        };

        static void checkVector(glm::vec3 const &hardCodedVertexColor, size_t inputVectorSize, std::vector<Vertex> &output) {
            if (output.empty()) {
                output.resize(inputVectorSize, Vertex(glm::vec3{}, hardCodedVertexColor, glm::vec2{}, glm::vec3{}));
            }

            if (output.size() != inputVectorSize) {
                throw std::runtime_error("GLB: Invalid vector size when decoding GLB file.");
            }
        }

        class PositionNormalTextureVisitor : public boost::static_visitor<void> {
        public:
            template <typename InputVectorType>
            void operator()(std::vector<InputVectorType> const &) {
                throw std::runtime_error("GLB: Invalid type or component type for position, normal, or texture coordinates.");
            }

            template <>
            void operator()(std::vector<glm::vec3> const &inputVector) {
                checkVector(m_color, inputVector.size(), m_output.first);

                size_t i = 0;
                if (m_propertyType == PropertyType::POSITION) {
                    for (auto input: inputVector) {
                        m_output.first[i].pos = input;
                        i++;
                    }
                } else if (m_propertyType == PropertyType::NORMAL) {
                    for (auto input: inputVector) {
                        m_output.first[i].normal = input;
                        i++;
                    }
                } else {
                    throw std::runtime_error("GLB: Invalid type or component type for texture coordinates.");
                }
            }

            template <>
            void operator()(std::vector<glm::vec2> const &inputVector) {
                checkVector(m_color, inputVector.size(), m_output.first);

                if (m_propertyType != PropertyType::TEXTURE) {
                    throw std::runtime_error("GLB: Invalid type or component type for position or normal.");
                }

                size_t i = 0;
                for (auto input: inputVector) {
                    m_output.first[i].texCoord = input;
                    i++;
                }
            }

            explicit PositionNormalTextureVisitor(glm::vec3 const &defaultColor, ModelVertices &output, PropertyType propertyType)
            : m_output{output},
              m_propertyType{propertyType},
              m_color{defaultColor}{ }

        private:
            ModelVertices &m_output;
            PropertyType m_propertyType;
            glm::vec3 m_color;
        };

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

            explicit ColorVisitor(ModelVertices &output)
                    : m_output(output) { }

        private:
            template <glm::length_t vecNumElements, typename memberType, glm::qualifier qualifier, typename Enable = void>
            class Helper {
            public:
                void doStuff(std::vector<glm::vec<vecNumElements, memberType, qualifier>> const &) {
                    throw std::runtime_error("GLB: Invalid type for color.");
                }

                explicit Helper(ModelVertices &) {}
            };

            template <glm::length_t vecNumElements, typename memberType, glm::qualifier qualifier>
            class Helper <vecNumElements, memberType, qualifier, typename std::enable_if<vecNumElements!=2>::type> {
            public:
                void doStuff(std::vector<glm::vec<vecNumElements, memberType, qualifier>> const &inputVector) {
                    glm::vec3 defaultColor{};
                    checkVector(defaultColor, inputVector.size(), m_output.first);

                    size_t i = 0;
                    memberType max = std::numeric_limits<memberType>::max();
                    for (auto input: inputVector) {
                        m_output.first[i].color.r = input.r / max;
                        m_output.first[i].color.g = input.g / max;
                        m_output.first[i].color.b = input.b / max;
                        i++;
                    }
                }

                explicit Helper(ModelVertices &output)
                : m_output(output) { }

            private:
                ModelVertices &m_output;
            };

            template <glm::length_t vecNumElements, glm::qualifier qualifier>
            class Helper <vecNumElements, float, qualifier, typename std::enable_if_t<vecNumElements!=2>::type> {
                void doStuff(std::vector<glm::vec<vecNumElements, float, qualifier>> const &inputVector) {
                    glm::vec3 defaultColor{};
                    checkVector(defaultColor, inputVector.size(), m_output.first);

                    size_t i = 0;
                    for (auto input: inputVector) {
                        m_output.first[i].color.r = input.r;
                        m_output.first[i].color.g = input.g;
                        m_output.first[i].color.b = input.b;
                        i++;
                    }
                }
                explicit Helper(ModelVertices &output)
                        : m_output(output) { }

            private:
                ModelVertices &m_output;
            };

            ModelVertices &m_output;
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

                m_output.second.resize(inputVector.size());
                for (size_t i = 0; i < inputVector.size(); i++) {
                    m_output.second[i] = inputVector[i];
                }
            }

            template <>
            void operator()(std::vector<uint32_t> const &inputVector) {
                m_output.second.resize(inputVector.size());
                memcpy(m_output.second.data(), inputVector.data(), inputVector.size());
            }

            explicit IndicesVisitor(ModelVertices &output)
            : m_output{output} {}
        private:
            ModelVertices &m_output;
        };

        template <typename type>
        static inline void doByteSwap (type *buffer, size_t nbrTypes) {
            if (sizeof (type) > 1 && boost::endian::order::native == boost::endian::order::big) {
                for (size_t i = 0; i < nbrTypes; i++) {
                    boost::endian::endian_reverse_inplace(buffer[i]);
                }
            }
        }

        glm::vec3 m_color;
    };

    std::pair<ModelVertices, ModelVertices> ModelDescriptionPath::getDataWithVertexNormalsAlso(
            const std::shared_ptr<GameRequester> &gameRequester)
    {
        ModelVertices vertices;
        ModelVertices verticesWithVertexNormals;
        std::shared_ptr<ModelReader> reader;
        loadModel(gameRequester->getAssetStream(m_path), reader, vertices, &verticesWithVertexNormals);
        return std::make_pair(std::move(vertices), std::move(verticesWithVertexNormals));
    }

    std::shared_ptr<ModelReader> ModelDescriptionPath::getReader() {
        if (m_path.compare(m_path.size() - 3, 3, "glb")) {
            return std::make_shared<ModelReaderGLB>(m_color);
        } else {
            return std::make_shared<ModelReaderCBOR>(m_color);
        }
    }

    ModelVertices
    ModelDescriptionPath::getData(std::shared_ptr<GameRequester> const &gameRequester) {
        ModelVertices vertices;
        std::shared_ptr<ModelReader> reader = getReader();
        loadModel(gameRequester->getAssetStream(m_path), reader, vertices);
        return vertices;
    }

    bool ModelDescriptionPath::loadModel(
            std::unique_ptr<std::streambuf> const &modelStreamBuf,
            std::shared_ptr<ModelReader> &reader,
            ModelVertices &verticesWithFaceNormals,
            ModelVertices *verticesWithVertexNormals) {

        try {
            reader->read(modelStreamBuf, verticesWithFaceNormals, verticesWithVertexNormals);
        } catch (...) {
            return false;
        }

        return true;
    }

    ModelVertices ModelDescriptionQuad::getData(std::shared_ptr<GameRequester> const &) {
        ModelVertices vertices{};
        Vertex vertex = {};
        vertex.color = m_color;
        vertex.normal = {0.0f, 0.0f, 1.0f};

        vertex.pos = glm::vec3{-1.0f, 1.0f, 0.0f} + m_center;
        vertex.texCoord = {0.0f, 0.0f};
        vertices.first.push_back(vertex);

        vertex.pos = glm::vec3{-1.0f, -1.0f, 0.0f} + m_center;
        vertex.texCoord = {0.0f, 1.0f};
        vertices.first.push_back(vertex);

        vertex.pos = glm::vec3{1.0f, -1.0f, 0.0f} + m_center;
        vertex.texCoord = {1.0f, 1.0f};
        vertices.first.push_back(vertex);

        vertex.pos = glm::vec3{1.0f, 1.0f, 0.0f} + m_center;
        vertex.texCoord = {1.0f, 0.0f};
        vertices.first.push_back(vertex);

        vertices.second.push_back(0);
        vertices.second.push_back(1);
        vertices.second.push_back(2);

        vertices.second.push_back(0);
        vertices.second.push_back(2);
        vertices.second.push_back(3);

        return vertices;
    }

// creates a cube with each side length 2.0f.
    ModelVertices ModelDescriptionCube::getData(std::shared_ptr<GameRequester> const &) {
        ModelVertices vertices{};
        Vertex vertex = {};
        vertex.color = m_color;

        std::array<glm::vec3, 8> positions = {
                glm::vec3{-1.0f, 1.0f, 1.0f} + m_center,
                glm::vec3{-1.0f, -1.0f, 1.0f} + m_center,
                glm::vec3{1.0f, -1.0f, 1.0f} + m_center,
                glm::vec3{1.0f, 1.0f, 1.0f} + m_center,

                glm::vec3{-1.0f, 1.0f, -1.0f} + m_center,
                glm::vec3{-1.0f, -1.0f, -1.0f} + m_center,
                glm::vec3{1.0f, -1.0f, -1.0f} + m_center,
                glm::vec3{1.0f, 1.0f, -1.0f} + m_center
        };

        // the top, z = 1.0
        size_t i = vertices.first.size();
        vertex.normal = {0.0f, 0.0f, 1.0f};

        vertex.pos = positions[0];
        vertex.texCoord = {0.0f, 0.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[1];
        vertex.texCoord = {0.0f, 1.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[2];
        vertex.texCoord = {1.0f / 2.0f, 1.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[3];
        vertex.texCoord = {1.0f / 2.0f, 0.0f};
        vertices.first.push_back(vertex);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 1);
        vertices.second.push_back(i + 2);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 2);
        vertices.second.push_back(i + 3);

        // the bottom, z = -1
        i = vertices.first.size();
        vertex.normal = {0.0f, 0.0f, -1.0f};

        vertex.pos = positions[7];
        vertex.texCoord = {0.0f, 1.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[6];
        vertex.texCoord = {0.0f, 2.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[5];
        vertex.texCoord = {1.0f / 2.0f, 2.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[4];
        vertex.texCoord = {1.0f / 2.0f, 1.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 1);
        vertices.second.push_back(i + 2);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 2);
        vertices.second.push_back(i + 3);

        // the side, y = -1
        i = vertices.first.size();
        vertex.normal = {0.0f, -1.0f, 0.0f};

        vertex.pos = positions[1];
        vertex.texCoord = {0.0f, 2.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[5];
        vertex.texCoord = {0.0f, 1.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[6];
        vertex.texCoord = {1.0f / 2.0f, 1.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[2];
        vertex.texCoord = {1.0f / 2.0f, 2.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 1);
        vertices.second.push_back(i + 2);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 2);
        vertices.second.push_back(i + 3);

        // the side, y = 1
        i = vertices.first.size();
        vertex.normal = {0.0f, 1.0f, 0.0f};

        vertex.pos = positions[3];
        vertex.texCoord = {1.0f / 2.0f, 0.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[7];
        vertex.texCoord = {1.0f / 2.0f, 1.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[4];
        vertex.texCoord = {1.0f, 1.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[0];
        vertex.texCoord = {1.0f, 0.0f};
        vertices.first.push_back(vertex);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 1);
        vertices.second.push_back(i + 2);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 2);
        vertices.second.push_back(i + 3);

        // the side, x = -1
        i = vertices.first.size();
        vertex.normal = {-1.0f, 0.0f, 0.0f};

        vertex.pos = positions[0];
        vertex.texCoord = {1.0f / 2.0f, 1.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[4];
        vertex.texCoord = {1.0f / 2.0f, 2.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[5];
        vertex.texCoord = {1.0f, 2.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[1];
        vertex.texCoord = {1.0f, 1.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 1);
        vertices.second.push_back(i + 2);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 2);
        vertices.second.push_back(i + 3);

        // the side, x = 1
        i = vertices.first.size();
        vertex.normal = {1.0f, 0.0f, 0.0f};

        vertex.pos = positions[2];
        vertex.texCoord = {1.0f / 2.0f, 2.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[6];
        vertex.texCoord = {1.0f / 2.0f, 1.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[7];
        vertex.texCoord = {1.0f, 1.0f};
        vertices.first.push_back(vertex);

        vertex.pos = positions[3];
        vertex.texCoord = {1.0f, 2.0f / 3.0f};
        vertices.first.push_back(vertex);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 1);
        vertices.second.push_back(i + 2);

        vertices.second.push_back(i + 0);
        vertices.second.push_back(i + 2);
        vertices.second.push_back(i + 3);

        return vertices;
    }
}
