/**
 * Copyright 2018 Cerulean Quasar. All Rights Reserved.
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <istream>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "maze.hpp"
#include "android.hpp"
#include "random.hpp"
#include "vulkanWrapper.hpp"

std::string const TEXTURE_PATH_WALLS("textures/wall.png");
std::string const TEXTURE_PATH_FLOOR("textures/floor.png");
std::string const TEXTURE_PATH_BALL("textures/ball.png");
std::string const TEXTURE_PATH_HOLE("textures/hole.png");

static std::string const MODEL_WALL("models/wall.obj");
static std::string const MODEL_BALL("models/ball.obj");
static std::string const MODEL_HOLE("models/hole.obj");

namespace std {
    template<> struct hash<glm::vec3> {
        size_t operator()(glm::vec3 vector) const {
            return ((hash<float>()(vector.x) ^
                     (hash<float>()(vector.y) << 1)) >> 1) ^
                   (hash<float>()(vector.z) << 1);
        }
    };

    template<> struct hash<glm::vec2> {
        size_t operator()(glm::vec2 vector) const {
            return (hash<float>()(vector.x) ^ (hash<float>()(vector.y) << 1));
        }
    };

    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.texCoord) << 1)) >> 1) ^ (hash<glm::vec3>()(vertex.normal) << 1);
        }
    };
}

VkVertexInputBindingDescription Vertex::getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription = {};

    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);

    /* move to the next data entry after each vertex.  VK_VERTEX_INPUT_RATE_INSTANCE
     * moves to the next data entry after each instance, but we are not using instanced
     * rendering
     */
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 4> Vertex::getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};

    /* position */
    attributeDescriptions[0].binding = 0; /* binding description to use */
    attributeDescriptions[0].location = 0; /* matches the location in the vertex shader */
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    /* color */
    attributeDescriptions[1].binding = 0; /* binding description to use */
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    /* texture coordinate */
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    /* normal vector */
    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex, normal);
    return attributeDescriptions;
}

bool Vertex::operator==(const Vertex& other) const {
    return pos == other.pos && color == other.color && texCoord == other.texCoord && normal == other.normal;
}

void Maze::setView() {
    /* glm::lookAt takes the eye position, the center position, and the up axis as parameters */
    view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Maze::getViewLightSource() {
    return glm::lookAt(getLightingSource(), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

void Maze::updateAcceleration(float x, float y, float z) {
    ball.acceleration = glm::vec3(-x,-y,0.0f);
}

bool Maze::updateData() {
    if (finished) {
        // the maze is finished, do nothing and return false (drawing is not necessary).
        return false;
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count();
    prevTime = currentTime;

    ball.velocity += ball.acceleration * time;
    ball.position += ball.velocity * time;

    auto cell = getCell(ball.row, ball.col);
    float cellCenterX = getColumnCenterPosition(ball.col);
    float cellCenterY = getRowCenterPosition(ball.row);
    if (cell.isEnd()) {
        finished = true;
        ball.position.x = cellCenterX;
        ball.position.y = cellCenterY;
        ball.velocity = {0.0f, 0.0f, 0.0f};
        return true;
    }
    if (cell.leftWallExists() && ball.position.x < cellCenterX) {
        if (ball.velocity.x < 0.0f) {
            ball.velocity.x = 0.0f;
        }
        ball.position.x = cellCenterX;
    }
    if (cell.rightWallExists() && cellCenterX < ball.position.x) {
        if (ball.velocity.x > 0.0f) {
            ball.velocity.x = 0.0f;
        }
        ball.position.x = cellCenterX;
    }
    if (cell.bottomWallExists() && cellCenterY < ball.position.y) {
        if (ball.velocity.y > 0.0f) {
            ball.velocity.y = 0.0f;
        }
        ball.position.y = cellCenterY;
    }
    if (cell.topWallExists() && ball.position.y < cellCenterY) {
        if (ball.velocity.y < 0.0f) {
            ball.velocity.y = 0.0f;
        }
        ball.position.y = cellCenterY;
    }

    float cellHeight = 2.0f / numberRows;
    float cellWidth = 2.0f / numberColumns;

    float delta = cellWidth/5.0f;
    if (ball.position.x > cellCenterX + delta || ball.position.x < cellCenterX - delta) {
        ball.position.y = cellCenterY;
        ball.velocity.y = 0.0f;
    }

    delta = cellHeight/5.0f;
    if (ball.position.y > cellCenterY + delta || ball.position.y < cellCenterY - delta) {
        ball.position.x = cellCenterX;
        ball.velocity.x = 0.0f;
    }

    float deltax = ball.position.x - cellCenterX;
    float deltay = ball.position.y - cellCenterY;

    if (deltay > cellHeight || deltay < -cellHeight || deltax > cellWidth || deltax < -cellWidth) {
        deltax +=0.00001;
    }

    if (deltay > cellHeight/2.0f && ball.row != numberRows - 1) {
        ball.row++;
    } else if (deltay < -cellHeight/2.0f && ball.row != 0) {
        ball.row--;
    }

    if (deltax > cellWidth/2.0f && ball.col != numberColumns - 1) {
        ball.col++;
    } else if (deltax < -cellWidth/2.0f && ball.col != 0) {
        ball.col--;
    }

    glm::vec3 axis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), ball.velocity);
    if (glm::length(axis) != 0) {
        float scaleFactor = 10.0f;
        glm::quat q = glm::angleAxis(glm::length(ball.velocity)*time*scaleFactor, glm::normalize(axis));

        ball.totalRotated = glm::normalize(q * ball.totalRotated);
    }
    modelMatrixBall = glm::translate(ball.position) * glm::toMat4(ball.totalRotated) * scaleBall;

    bool drawingNecessary = glm::length(ball.position - ball.prevPosition) > 0.00005;
    ball.prevPosition = ball.position;
    return drawingNecessary;
}

void Maze::updatePerspectiveMatrix(int surfaceWidth, int surfaceHeight) {
    /* perspective matrix: takes the perspective projection, the aspect ratio, near and far
     * view planes.
     */
    proj = glm::perspective(glm::radians(45.0f), surfaceWidth / (float) surfaceHeight, 0.1f, 100.0f);
}

void Maze::generate() {
    Random random;
    std::vector<std::pair<unsigned int, unsigned int> > path;
    unsigned int rowStart, columnStart;
    unsigned int rowEnd, columnEnd;

    // the desired distance that they are apart is the average of the number of rows and the number
    // of columns divided by 2.
    unsigned int desiredDistanceSquared = (numberRows+numberColumns)*(numberRows+numberColumns)/16;
    unsigned int distanceSquared;
    do {
        rowStart = random.getUInt(0, numberRows-1);
        columnStart = random.getUInt(0, numberColumns-1);
        rowEnd = random.getUInt(0, numberRows-1);
        columnEnd = random.getUInt(0, numberColumns-1);
        distanceSquared = (rowEnd - rowStart)*(rowEnd - rowStart) +
            (columnEnd - columnStart)*(columnEnd - columnStart);
    } while (distanceSquared < desiredDistanceSquared);

    cells[rowStart][columnStart].mIsStart = true;
    cells[rowEnd][columnEnd].mIsEnd = true;

    cells[rowStart][columnStart].mVisited = true;
    path.push_back(std::make_pair(rowStart, columnStart));

    while (!path.empty()) {
        std::pair<unsigned int, unsigned int> current = path.back();
        if (cells[current.first][current.second].isEnd()) {
            path.pop_back();
            continue;
        }

        std::vector<std::pair<unsigned int, unsigned int> > nextCellOptions;
        addCellOption(current.first-1, current.second, nextCellOptions);
        addCellOption(current.first, current.second-1, nextCellOptions);
        addCellOption(current.first+1, current.second, nextCellOptions);
        addCellOption(current.first, current.second+1, nextCellOptions);

        if (nextCellOptions.empty()) {
            path.pop_back();
            continue;
        }
        unsigned int i = random.getUInt(0, nextCellOptions.size() - 1);

        std::pair<unsigned int, unsigned int> next = nextCellOptions[i];

        if (current.first == next.first) {
            if (next.second < current.second) {
                // the new cell is on the left of the current one.
                cells[current.first][current.second].mLeftWallExists = false;
                cells[next.first][next.second].mRightWallExists = false;
            } else {
                cells[current.first][current.second].mRightWallExists = false;
                cells[next.first][next.second].mLeftWallExists = false;
            }
        } else {
            if (next.first < current.first) {
                // the new cell is on the top of the current one.
                cells[current.first][current.second].mTopWallExists = false;
                cells[next.first][next.second].mBottomWallExists = false;
            } else {
                cells[current.first][current.second].mBottomWallExists = false;
                cells[next.first][next.second].mTopWallExists = false;
            }
        }

        cells[next.first][next.second].mVisited = true;
        path.push_back(next);
    }
}

void Maze::addCellOption(unsigned int r, unsigned int c, std::vector<std::pair<unsigned int, unsigned int> > &options) {
    if (r < numberRows && c < numberColumns && !cells[r][c].visited()) {
        options.push_back(std::make_pair(r, c));
    }
}

Cell const &Maze::getCell(unsigned int row, unsigned int column) {
    return cells[row][column];
}

void Maze::loadModels() {
    loadModel(MODEL_WALL, vertices, indices);
    loadModel(MODEL_BALL, ballVertices, ballIndices);
    loadModel(MODEL_HOLE, holeVertices, holeIndices);
    loadModelFloor();
}

void Maze::loadModelFloor() {
    getQuad(floorVertices, floorIndices);
}

void Maze::loadModel(std::string const & modelFile, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    AssetStreambuf assetStreambuf(assetWrapper->getAsset(modelFile));
    std::istream assetIstream(&assetStreambuf);

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, &assetIstream)) {
        throw std::runtime_error(err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices = {};
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex = {};
            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = {0.2f, 0.2f, 0.2f};

            vertex.normal = { attrib.normals[3 * index.normal_index +0],
                              attrib.normals[3 * index.normal_index +1],
                              attrib.normals[3 * index.normal_index +2] };

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
}

float Maze::getRowCenterPosition(unsigned int row) {
    return 2.0f / (numberRows * numberBlocksPerCell) * (row*numberBlocksPerCell +1.5f) - 1.0f;
}

float Maze::getColumnCenterPosition(unsigned int col) {
    return 2.0f / (numberColumns * numberBlocksPerCell) * (col*numberBlocksPerCell+1.0f) - 1.0f;
}

glm::vec3 Maze::getCellCenterPosition(unsigned int row, unsigned int col) {
   return glm::vec3(getColumnCenterPosition(col),
                    getRowCenterPosition(row),
                    -1.0f - 3.0f/(2.0f*(numberRows+numberColumns)));
}

void Maze::generateModelMatrices() {
    // print
    unsigned int rowEnd;
    unsigned int colEnd;
    bool wallsExist[numberRows*numberBlocksPerCell+1][numberColumns*numberBlocksPerCell+1];
    memset(wallsExist, '\0', sizeof (wallsExist));
    for (unsigned int i = 0; i < numberRows*numberBlocksPerCell; i+=numberBlocksPerCell) {
        for (unsigned int j = 0; j < numberColumns * numberBlocksPerCell; j += numberBlocksPerCell) {
            Cell const &cell = getCell(i / numberBlocksPerCell, j / numberBlocksPerCell);
            if (cell.topWallExists()) {
                for (unsigned int k = 0; k <= numberBlocksPerCell; k++) {
                    wallsExist[i][j + k] = true;
                }
            }

            if (cell.leftWallExists()) {
                for (unsigned int k = 0; k < numberBlocksPerCell+1; k++) {
                    wallsExist[i+k][j] = true;
                }
            }

            // the ball
            if (cell.isStart()) {
                ball.row = i / numberBlocksPerCell;
                ball.col = j / numberBlocksPerCell;
            }

            // the hole
            if (cell.isEnd()) {
                rowEnd = i/numberBlocksPerCell;
                colEnd = j/numberBlocksPerCell;
            }
        }
        // right border
        for (unsigned int k = 0; k < numberBlocksPerCell; k++) {
            wallsExist[i+k][numberColumns * numberBlocksPerCell] = true;
        }
    }

    // bottom wall.
    for (unsigned int i = 0; i < numberColumns*numberBlocksPerCell+1; i++) {
        wallsExist[numberRows*numberBlocksPerCell][i] = true;
    }


    glm::mat4 trans;
    glm::mat4 scale  = glm::scale(glm::vec3(1.0f/(numberColumns*numberBlocksPerCell),
                                            1.0f/(numberRows*numberBlocksPerCell),
                                            1.0f/(numberRows+numberColumns)));

    // Create the model matrices.

    // the walls.
    for (unsigned int i = 0; i < numberRows*numberBlocksPerCell+1; i++) {
        for (unsigned int j = 0; j < numberColumns*numberBlocksPerCell+1; j++) {
            if (wallsExist[i][j]) {
                trans = glm::translate(
                        glm::vec3(2.0f / (numberColumns * numberBlocksPerCell) * j - 1.0f,
                                  2.0f / (numberRows * numberBlocksPerCell) * i - 1.0f,
                                  -1.0f));
                modelMatricesMaze.push_back(trans * scale);
            }
        }
    }

    // the ball
    ball.position = getCellCenterPosition(ball.row, ball.col);

    // cause the frame to be drawn when the program comes up for the first time.
    ball.prevPosition = {-10.0,0.0,0.0};

    trans = glm::translate(ball.position);
    modelMatrixBall = trans*scaleBall;

    // the hole
    trans = glm::translate(getCellCenterPosition(rowEnd, colEnd));
    modelMatrixHole = trans*scaleBall;

    // the floor.
    floorModelMatrix = glm::translate(glm::vec3(0.0f, 0.0f, -1.0f -3.0f/(2.0f*(numberRows+numberColumns))));
}

std::vector<DrawObject> Maze::getStaticDrawObjects() {
    std::vector<DrawObject> objs;

    // the floor
    DrawObject floor = {};
    floor.indices = floorIndices;
    floor.vertices = floorVertices;
    floor.imagePath = TEXTURE_PATH_FLOOR;
    floor.modelMatrices.push_back(floorModelMatrix);
    objs.push_back(floor);

    // the hole
    DrawObject hole = {};
    hole.indices = holeIndices;
    hole.vertices = holeVertices;
    hole.imagePath = TEXTURE_PATH_HOLE;
    hole.modelMatrices.push_back(modelMatrixHole);
    objs.push_back(hole);

    // the walls
    DrawObject wall = {};
    wall.indices = indices;
    wall.vertices = vertices;
    wall.imagePath = TEXTURE_PATH_WALLS;
    wall.modelMatrices = modelMatricesMaze;
    objs.push_back(wall);

    return objs;
}

std::vector<DrawObject> Maze::getDynamicDrawObjects() {
    std::vector<DrawObject> objs;

    // the ball
    DrawObject ball1 = {};
    ball1.indices = ballIndices;
    ball1.vertices = ballVertices;
    ball1.imagePath = TEXTURE_PATH_BALL;
    ball1.modelMatrices.push_back(modelMatrixBall);
    objs.push_back(ball1);

    return objs;
}

