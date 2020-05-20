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
#include "movablePassage.hpp"
#include "../level.hpp"

std::pair<uint32_t, uint32_t> GameBoard::findRC(glm::vec2 position) {
    uint32_t row;
    uint32_t col;

    row = static_cast<uint32_t>(std::floor((position.x/m_width + 0.5f) * m_blocks[0].size()));
    col = static_cast<uint32_t>(std::floor((position.y/m_height + 0.5f) * m_blocks.size()));

    return std::make_pair(row, col);
}

// returs true if a redraw is needed.
// expects the position and distance in world space.
// the distance is relative to the previous moved position, not relative to the start position.
bool GameBoard::drag(glm::vec2 const &startPosition, glm::vec2 const &distance) {
    std::pair<uint32_t, uint32_t> rc = findRC(startPosition);
    if (m_moveInProgress) {
        if (rc.first != m_moveStartingPosition.first || rc.second != m_moveStartingPosition.second) {
            // a move started, but the previous one never completed.  Just move the old piece back
            // in place and start the new move
            auto &b = m_blocks[m_moveStartingPosition.first][m_moveStartingPosition.second];
            b.component()->placement(b.placementIndex()).moveDone();
            m_moveStartingPosition = rc;
            m_moveInProgress = false;
        }
    }
    auto &b = m_blocks[rc.first][rc.second];
    if (!m_moveInProgress) {
        if ((b.blockType() == GameBoardBlock::BlockType::offBoard ||
             b.blockType() == GameBoardBlock::BlockType::onBoard) &&
            b.component() != nullptr &&
            b.component()->type() != Component::ComponentType::noMovementRock &&
            b.component()->type() != Component::ComponentType::noMovementDirt)
        {
            auto component = b.component();
            auto &placement = component->placement(b.placementIndex());
            if (!placement.lockedIntoPlace() && placement.prev().first == nullptr) {
                m_moveInProgress = true;
            }
        } else {
            return false;
        }
    }
    if (m_moveInProgress) {
        b.component()->placement(b.placementIndex()).movePlacement(distance);
    }
    return true;
}

// returs true if a redraw is needed.
// expects the position in world space.
bool GameBoard::dragEnded(glm::vec2 const &endPosition) {
    if (!m_moveInProgress) {
        return false;
    }

    std::pair<uint32_t, uint32_t> rc = findRC(endPosition);
    auto &bEnd = m_blocks[rc.first][rc.second];
    if ((bEnd.component() != nullptr &&
         bEnd.component()->type() == Component::ComponentType::noMovementDirt &&
         bEnd.blockType() == GameBoardBlock::BlockType::onBoard) ||
        (bEnd.component() != nullptr &&
         bEnd.component()->type() == Component::ComponentType::noMovementRock &&
         bEnd.blockType() != GameBoardBlock::BlockType::offBoard))
    {
        // a move was started and it succeeded.  It needs to be completed now.
        auto &b = m_blocks[m_moveStartingPosition.first][m_moveStartingPosition.second];
        b.component()->placement(b.placementIndex()).moveDone();
        b.component()->placement(b.placementIndex()).setRC(rc.first, rc.second);
        bEnd.component()->placement(bEnd.placementIndex()).setRC(m_moveStartingPosition.first,
                                                                 m_moveStartingPosition.second);
        auto tmp = b.component();
        auto tmpIndex = b.placementIndex();
        b.setComponent(bEnd.component(), bEnd.placementIndex());
        bEnd.setComponent(tmp, tmpIndex);
        m_moveInProgress = false;
        return true;
    }

    // move failed - there is a non-dirt component at the spot to be moved to,
    // or the target spot is end or start space on the board.
    bEnd.component()->placement(bEnd.placementIndex()).moveDone();

    // we still need to redraw even if the move failed.  to move the component back to the
    // original spot.
    return true;
}

bool GameBoard::tap(glm::vec2 const &position) {
    std::pair<uint32_t, uint32_t> rc = findRC(position);
    auto &b = m_blocks[rc.first][rc.second];
    if (b.component() == nullptr || (b.blockType() != GameBoardBlock::BlockType::onBoard &&
        b.blockType() != GameBoardBlock::BlockType::offBoard))
    {
        return false;
    }
    auto &placement = b.component()->placement(b.placementIndex());
    if (placement.lockedIntoPlace() && placement.prev().first != nullptr) {
        return false;
    }
    placement.rotate();
}

void MovablePassage::initSetGameBoard(
        uint32_t nbrTilesX,
        uint32_t nbrTilesY) {
    float tileSizeX = m_width / m_nbrTilesX;
    float tileSizeY = m_height / (m_nbrTilesY + GameBoard::m_nbrTileRowsForStart +
                                  GameBoard::m_nbrTileRowsForEnd);
    float tileSize = tileSizeX < tileSizeY ? tileSizeX : tileSizeY;
    uint32_t nbrExtraTileRowsX = static_cast<uint32_t>(std::floor(
            (m_width - tileSize * m_nbrTilesX) / tileSize));
    uint32_t nbrExtraTilesX = nbrExtraTileRowsX * m_nbrTilesY;

    // minus 2 * nbrExtraTileRowsY for up tunnel components
    uint32_t nbrExtraTileRowsY = static_cast<uint32_t>(std::floor(
            (m_height - tileSize * m_nbrTilesY) / tileSize));
    uint32_t nbrExtraTilesY = nbrExtraTileRowsY * m_nbrTilesX - 2 * nbrExtraTileRowsY;

    // we have a device with a screen of a similar size to our tunnel building area,
    // get some more space by making the movable space less.
    if (nbrExtraTilesX + nbrExtraTilesY < m_nbrComponents) {
        uint32_t moreExtraComponentsRequired = m_nbrComponents - (nbrExtraTilesX + nbrExtraTilesY);
        uint32_t nbrExtraPerimeters =
                moreExtraComponentsRequired / (2 * m_nbrTilesX + 2 * m_nbrTilesY) + 1;
        tileSizeX = m_width / (m_nbrTilesX + 2 * nbrExtraPerimeters);
        tileSizeY = m_height / (m_nbrTilesY + GameBoard::m_nbrTileRowsForStart +
                                GameBoard::m_nbrTileRowsForEnd + 2 * nbrExtraPerimeters);
        tileSize = tileSizeX < tileSizeY ? tileSizeX : tileSizeY;

        nbrExtraTileRowsX = static_cast<uint32_t>(std::floor(
                (m_width - tileSize * m_nbrTilesX) / tileSize));
        nbrExtraTilesX = nbrExtraTileRowsX * m_nbrTilesX;

        // minus 2 * nbrExtraTileRowsY for up tunnel components
        nbrExtraTileRowsY = static_cast<uint32_t>(std::floor(
                (m_height - tileSize * m_nbrTilesY) / tileSize));
        nbrExtraTilesY = nbrExtraTileRowsY * m_nbrTilesX - 2 * nbrExtraTileRowsY;
    }

    m_gameBoard.initialize(
            (m_nbrTilesX + nbrExtraTileRowsX) * tileSize,
            (m_nbrTilesY + nbrExtraTileRowsY + GameBoard::m_nbrTileRowsForEnd +
             GameBoard::m_nbrTileRowsForStart) * tileSize,
            glm::vec3{0.0f, 0.0f, 0.0f},
            m_nbrTilesY + nbrExtraTileRowsY, m_nbrTilesX + nbrExtraTileRowsX);
    if (m_gameBoard.heightInTiles() == 0 || m_gameBoard.widthInTiles() == 0) {
        throw std::runtime_error("MovablePassage game board blocks not initialized properly");
    }

    // The main board area for constructing the tunnel
    for (uint32_t k = nbrExtraTileRowsX / 2; k < m_nbrTilesX - nbrExtraTileRowsX; k++) {
        for (uint32_t l = nbrExtraTileRowsY / 2; l < m_nbrTilesY - nbrExtraTileRowsY; l++) {
            m_gameBoard.block(k, l).setBlockType(GameBoardBlock::BlockType::onBoard);
        }
    }

    uint32_t col = (m_nbrTilesX + nbrExtraTileRowsX) / 2;
    // the fixed tunnel through the places for extra tunnel pieces at the bottom.
    for (uint32_t k = GameBoard::m_nbrTileRowsForStart; k < nbrExtraTileRowsY / 2; k++) {
        auto &comp = m_components[Component::ComponentType::straight];
        auto pos = comp->add(k, col, 0.0f, true);
        auto &block = m_gameBoard.block(k, col);
        block.setComponent(comp, pos);
        block.setBlockType(GameBoardBlock::BlockType::onBoard);
    }

    // the fixed tunnel through the places for extra tunnel pieces at the top.
    auto &comp = m_components[Component::ComponentType::straight];
    for (uint32_t k = m_nbrTilesY - nbrExtraTileRowsY / 2 + GameBoard::m_nbrTileRowsForStart;
         k < m_nbrTilesY + nbrExtraTileRowsY + GameBoard::m_nbrTileRowsForStart;
         k++)
    {
        auto pos = comp->add(k, col, 0.0f, true);
        auto &block = m_gameBoard.block(k, col);
        block.setComponent(comp, pos);
        block.setBlockType(GameBoardBlock::BlockType::onBoard);
    }

    // The start
    for (uint32_t k = 0; k < GameBoard::m_nbrTileRowsForStart; k++) {
        for (uint32_t l = 0; l < m_gameBoard.widthInTiles(); l++) {
            Component::ComponentType type = Component::ComponentType::open;
            float rotation = 0.0f;
            if (k == 0) {
                if (l == 0) {
                    type = Component::ComponentType::closedCorner;
                } else if (l == m_gameBoard.widthInTiles() - 1) {
                    type = Component::ComponentType::closedCorner;
                    rotation = glm::radians(90.0f);
                } else {
                    type = Component::ComponentType::closedBottom;
                }
            } else if (k == GameBoard::m_nbrTileRowsForStart - 1) {
                if (l == 0) {
                    type = Component::ComponentType::closedCorner;
                    rotation = glm::radians(270.0f);
                } else if (l == m_gameBoard.widthInTiles() - 1) {
                    type = Component::ComponentType::closedCorner;
                    rotation = glm::radians(180.0f);
                } else {
                    type = Component::ComponentType::closedBottom;
                    rotation = glm::radians(180.0f);
                }
            } else if (l == 0) {
                type = Component::ComponentType::closedBottom;
                rotation = glm::radians(270.0f);
            } else if (l == m_gameBoard.widthInTiles() - 1) {
                type = Component::ComponentType::closedBottom;
                rotation = glm::radians(90.0f);
            }
            auto &compEnd = m_components[type];
            auto pos = compEnd->add(k, l, rotation, true);
            auto &block = m_gameBoard.block(k, l);
            block.setComponent(compEnd, pos);
            block.setBlockType(GameBoardBlock::BlockType::begin);
        }
    }

    // The end
    for (uint32_t k = m_gameBoard.heightInTiles() - GameBoard::m_nbrTileRowsForEnd;
         k < m_gameBoard.heightInTiles();
         k++)
    {
        for (uint32_t l = 0; l < m_gameBoard.widthInTiles(); l++) {
            auto &block = m_gameBoard.block(0, l);
            block.setBlockType(GameBoardBlock::BlockType::end);
            // don't set a component - we detect the end by the block type
        }
    }

    for (auto &component : m_components) {
        component->setSize(tileSize);
    }

    m_gameBoard.setBlockSize(tileSize);
}

void MovablePassage::initDone() {
    if (m_nbrComponents == 0) {
        throw std::runtime_error("MovablePassage components not initialized properly");
    }

    // place the passage components in the off board sections
    auto it1 = m_components.begin();
    auto end1 = m_components.end();
    auto it2 = (*it1)->placementsBegin();
    auto end2 = (*it1)->placementsEnd();
    uint32_t m = 0;
    bool finished = false;
    for (uint32_t k = 0; k < m_gameBoard.widthInTiles() && !finished; k++){
        for (uint32_t l = 0; l < m_gameBoard.heightInTiles(); l++) {
            auto &block = m_gameBoard.block(k,l);
            if (it2 != end2 && block.blockType() == GameBoardBlock::BlockType::offBoard && block.component() == nullptr) {
                block.setComponent(*it1, m);
                it2->m_row = k;
                it2->m_col = l;

                it2++;
                m++;
            }
            if (it2 == end2) {
                it1++;

                if (it1 == end1) {
                    finished = true;
                    break;
                }

                m = 0;
                it2 = (*it1)->placementsBegin();
                end2 = (*it1)->placementsEnd();
            }
        }
    }

    std::shared_ptr<Component> dirt = m_components[Component::ComponentType::noMovementDirt];
    std::shared_ptr<Component> rock = m_components[Component::ComponentType::noMovementRock];
    for (uint32_t k = 0; k < m_gameBoard.widthInTiles(); k++) {
        for (uint32_t l = 0; l < m_gameBoard.heightInTiles(); l++) {
            auto &b = m_gameBoard.block(k,l);
            if (b.component() == nullptr) {
                if (b.blockType() == GameBoardBlock::BlockType::offBoard) {
                    dirt->add(k, l, 0.0f);
                    b.setComponent(dirt, dirt->nbrPlacements() - 1);
                } else {
                    rock->add(k, l, 0.0f);
                    b.setComponent(rock, rock->nbrPlacements() - 1);
                }
            }
        }
    }

    m_ballRow = 0;
    m_ballCol = m_gameBoard.widthInTiles() / 2;
    m_ball.position = m_gameBoard.position(m_ballRow, m_ballCol);
    m_initDone = true;
}

bool MovablePassage::updateData() {
    if (!m_initDone) {
        return false;
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    float timeDiff = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - m_prevTime).count();
    m_prevTime = currentTime;

    glm::vec3 position = m_ball.position;
    glm::vec3 prevPosition = position;
    glm::vec3 velocity = getUpdatedVelocity(m_ball.acceleration, timeDiff);
    while (timeDiff > 0.0f) {
        glm::vec3 posFromCenter = m_gameBoard.position(m_ballRow, m_ballCol) - position;
        auto &block = m_gameBoard.block(m_ballRow, m_ballCol);
        Component::CellWall wall = block.component()->moveBallInCell(
                block.placementIndex(), posFromCenter, timeDiff, velocity);
        size_t ballRowNext = m_ballRow;
        size_t ballColNext = m_ballCol;
        Component::CellWall wallNextCell;
        switch (wall) {
        case Component::CellWall::noWall:
            m_ball.position = m_gameBoard.position(m_ballRow, m_ballCol) - posFromCenter;
            m_ball.prevPosition = prevPosition;
            return drawingNecessary();
        case Component::CellWall::wallRight:
            if (ballRowNext != m_gameBoard.widthInTiles() - 1) {
                ballRowNext++;
                wallNextCell = Component::CellWall::wallLeft;
            }
            break;
        case Component::CellWall::wallUp:
            if (ballColNext != m_gameBoard.heightInTiles() - 1) {
                ballColNext++;
                wallNextCell = Component::CellWall::wallDown;
            }
            break;
        case Component::CellWall::wallLeft:
            if (ballRowNext != 0) {
                ballRowNext--;
                wallNextCell = Component::CellWall::wallRight;
            }
            break;
        case Component::CellWall::wallDown:
            if (ballColNext != 0) {
                ballColNext--;
                wallNextCell = Component::CellWall::wallUp;
            }
        }

        // the ball is rolling off the game board area - move to the edge and return
        if (m_ballRow == ballRowNext && m_ballCol == ballColNext) {
            m_ball.position = m_gameBoard.position(m_ballRow, m_ballCol) - posFromCenter;
            m_ball.prevPosition = prevPosition;
            m_ball.velocity = {0.0f, 0.0f, 0.0f};
            return drawingNecessary();
        }

        auto &nextBlock = m_gameBoard.block(ballRowNext, ballColNext);
        if (nextBlock.blockType() == GameBoardBlock::BlockType::end) {
            // winning condition.  Set m_finished and return that drawing is necessary.
            m_finished = true;
            m_ball.position = m_gameBoard.position(m_ballRow, m_ballCol) - posFromCenter;
            return true;
        } else if (nextBlock.blockType() == GameBoardBlock::BlockType::offBoard ||
                   nextBlock.component() == nullptr) {
            // hit the edge of the game board or there is no component for the ball to
            // move into, stop the ball at the edge of the cell and return
            m_ball.position = m_gameBoard.position(m_ballRow, m_ballCol) - posFromCenter;
            m_ball.prevPosition = prevPosition;
            m_ball.velocity = {0.0f, 0.0f, 0.0f};
            return drawingNecessary();
        } else if (block.blockType() == GameBoardBlock::BlockType::begin &&
                   nextBlock.blockType() == GameBoardBlock::BlockType::begin) {
            // advance the ball into the next cell, but don't track its path
            m_ball.position = m_gameBoard.position(m_ballRow, m_ballCol) - posFromCenter;
            m_ball.prevPosition = prevPosition;
            m_ballCol = ballColNext;
            m_ballRow = ballRowNext;
            continue;
        }

        if (!nextBlock.component()->hasWallAt(wallNextCell, nextBlock.placementIndex())) {
            // ball allowed to advance into next door cell.  set flag to notify caller that
            // the textures have changed.  Also track the ball moving into the next component.
            m_texturesChanged = true;
            Component::Placement &placement = nextBlock.component()->placement(
                    nextBlock.placementIndex());
            Component::Placement &oldPlacement = block.component()->placement(
                    block.placementIndex());
            if (placement.next().first != nullptr) {
                if (placement.next().first == block.component() &&
                    placement.next().second == block.placementIndex()) {
                    // We are going backwards.  Unblock block that we were at and move on.
                    oldPlacement.next() = std::make_pair(nullptr, 0);
                    oldPlacement.prev() = std::make_pair(nullptr, 0);
                } else {
                    // We encountered a loop.  Unblock the entire loop
                    std::shared_ptr<Component> nextComponent = nextBlock.component();
                    size_t index = nextBlock.placementIndex();
                    std::shared_ptr<Component> loopComponent = block.component();
                    size_t loopIndex = block.placementIndex();
                    while (loopComponent != nextComponent && loopIndex != index) {
                        auto tmp = loopComponent->placement(loopIndex).prev();
                        auto &loopPlacement = loopComponent->placement(loopIndex);
                        loopPlacement.prev() = std::make_pair(nullptr, 0);
                        loopPlacement.next() = std::make_pair(nullptr, 0);
                        loopComponent = tmp.first;
                        loopIndex = tmp.second;
                    }
                }
            } else {
                // the ball is entering a new cell that is not in its path yet.
                placement.prev() = std::make_pair(block.component(), block.placementIndex());
                oldPlacement.next() = std::make_pair(nextBlock.component(),
                                                     nextBlock.placementIndex());
            }
            m_ballCol = ballColNext;
            m_ballRow = ballRowNext;
        }
    }

    updateRotation(timeDiff);

    return drawingNecessary();

}

bool MovablePassage::updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) {
    if (!m_initDone || !objs.empty()) {
        return false;
    }

    auto objEnd = std::make_shared<DrawObject>();
    getQuad(objEnd->vertices, objEnd->indices);
    glm::vec3 endPosition = m_gameBoard.centerPositionEndObject();
    glm::vec3 endScale = m_gameBoard.scaleEndObject();
    endScale.x /= m_modelSize;
    endScale.y /= m_modelSize;
    objEnd->modelMatrices.push_back(glm::translate(glm::mat4(1.0f), endPosition) *
        glm::scale(glm::mat4(1.0f), endScale));
    objEnd->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester, m_componentTextureEnd);
    textures.insert(std::make_pair(objEnd->texture, std::shared_ptr<TextureData>()));
    uint32_t refEnd = objs.size();
    objs.push_back(std::make_pair(objEnd, std::shared_ptr<DrawObjectData>()));

    std::pair<std::vector<Vertex>, std::vector<uint32_t>> v;
    loadModel(m_gameRequester->getAssetStream(m_componentTextures[Component::ComponentType::closedCorner]), v);
    auto objStartCorner = std::make_shared<DrawObject>();
    std::swap(objStartCorner->vertices, v.first);
    std::swap(objStartCorner->indices, v.second);
    objStartCorner->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester,
            m_componentTextures[Component::ComponentType::closedCorner]);
    textures.insert(std::make_pair(objStartCorner->texture, std::shared_ptr<DrawObjectData>()));
    uint32_t refStartCorner = objs.size();
    m_components[Component::ComponentType::closedCorner]->setObjReference(
            Component::MovableType::staticObj, refStartCorner);
    objs.push_back(std::make_pair(objStartCorner, std::shared_ptr<DrawObjectData>()));

    loadModel(m_gameRequester->getAssetStream(m_componentModels[Component::ComponentType::closedBottom]), v);
    auto objStartSide = std::make_shared<DrawObject>();
    std::swap(objStartSide->vertices, v.first);
    std::swap(objStartSide->indices, v.second);
    objStartSide->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester,
            m_componentTextures[Component::ComponentType::closedBottom]);
    textures.insert(std::make_pair(objStartSide->texture, std::shared_ptr<TextureData>()));
    uint32_t refStartSide = objs.size();
    m_components[Component::ComponentType::closedBottom]->setObjReference(
            Component::MovableType::staticObj, refStartSide);
    objs.push_back(std::make_pair(objStartSide, std::shared_ptr<DrawObjectData>()));

    loadModel(m_gameRequester->getAssetStream(m_componentModels[Component::ComponentType::open]), v);
    auto objStartCenter = std::make_shared<DrawObject>();
    std::swap(objStartCenter->vertices, v.first);
    std::swap(objStartCenter->indices, v.second);
    objStartCenter->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester,
            m_componentTextures[Component::ComponentType::open]);
    textures.insert(std::make_pair(objStartCenter->texture, std::shared_ptr<TextureData>()));
    uint32_t refStartCenter = objs.size();
    m_components[Component::ComponentType::open]->setObjReference(
            Component::MovableType::staticObj, refStartCenter);
    objs.push_back(std::make_pair(objStartCenter, std::shared_ptr<DrawObjectData>()));

    float scale = m_gameBoard.blockSize()/m_modelSize;
    bool done = false;
    for (uint32_t i = 0; i < m_gameBoard.heightInTiles() && !done; i++) {
        for (uint32_t j = 0; j < m_gameBoard.widthInTiles(); j++) {
            if (m_gameBoard.blockType(i,j) != GameBoardBlock::BlockType::begin) {
                done = true;
                break;
            }
            auto &component = m_gameBoard.block(i,j).component();
            size_t placementIndex = m_gameBoard.block(i,j).placementIndex();
            auto &placement = component->placement(placementIndex);
            switch (component->type()) {
                case Component::ComponentType::closedCorner:
                    objStartCorner->modelMatrices.push_back(glm::translate(glm::mat4(1.0f), m_gameBoard.position(i,j)) *
                        glm::rotate(glm::mat4(1.0f), placement.rotationAngle(), glm::vec3{0.0f, 0.0f, 1.0f}) *
                        glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale)));
                    break;
                case Component::ComponentType::closedBottom:
                    objStartSide->modelMatrices.push_back(glm::translate(glm::mat4(1.0f), m_gameBoard.position(i,j)) *
                        glm::rotate(glm::mat4(1.0f), placement.rotationAngle(), glm::vec3{0.0f, 0.0f, 1.0f}) *
                        glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale)));
                    break;
                case Component::ComponentType::open:
                    objStartCenter->modelMatrices.push_back(glm::translate(glm::mat4(1.0f), m_gameBoard.position(i,j)) *
                        glm::rotate(glm::mat4(1.0f), placement.rotationAngle(), glm::vec3{0.0f, 0.0f, 1.0f}) *
                        glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale)));
                    break;
            }
        }
    }
}

bool MovablePassage::updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged) {
    glm::vec3 zaxis{0.0f, 0.0f, 1.0f};
    if (objs.empty()) {
        std::pair<std::vector<Vertex>, std::vector<uint32_t>> v;
        for (auto const &component: m_components) {
            if (component->type() == Component::ComponentType::closedBottom ||
                component->type() == Component::ComponentType::closedCorner ||
                component->type() == Component::ComponentType::open)
            {
                // These components are handled as static draw objects.
                continue;
            }
            auto obj = std::make_shared<DrawObject>();
            if (m_componentModels[component->type()].empty()) {
                getQuad(obj->vertices, obj->indices);
            } else {
                loadModel(m_gameRequester->getAssetStream(m_componentModels[component->type()]), v);
                std::swap(v.first, obj->vertices);
                std::swap(v.second, obj->indices);
            }

            obj->texture = std::make_shared<TextureDescriptionPath>(
                    m_gameRequester, m_componentTextures[component->type()]);
            float size = component->componentSize()/m_modelSize;
            for (auto it = component->placementsBegin(); it != component->placementsEnd(); it++) {
                obj->modelMatrices.push_back(
                    glm::translate(glm::mat4(1.0f), m_gameBoard.position(it->row(), it->col())) *
                    glm::rotate(glm::mat4(1.0f), it->rotationAngle(), zaxis) *
                    glm::scale(glm::mat4(1.0f), glm::vec3{size, size, size}));
            }
            component->setObjReference(Component::MovableType::dynamicObj, objs.size());
            objs.push_back(std::make_pair(obj, std::shared_ptr<TextureData>()));
        }
    } else {
        for (size_t i = 0; i < objs.size(); i++) {
            auto &component = m_components[i];
            auto ref = component->objReference();
            if (component->type() == Component::ComponentType::closedBottom ||
                component->type() == Component::ComponentType::closedCorner ||
                component->type() == Component::ComponentType::open ||
                ref.first != Component::MovableType::dynamicObj)
            {
                // These components are handled as static draw objects.
                continue;
            }

            auto &obj = objs[ref.second].first;
            float size = component->componentSize()/m_modelSize;
            for (size_t j = 0; j <  component->nbrPlacements(); j++) {
                auto &placement = component->placement(j);
                obj->modelMatrices[j] =
                    glm::translate(glm::mat4(1.0f), m_gameBoard.position(placement.row(), placement.col())) *
                    glm::rotate(glm::mat4(1.0f), placement.rotationAngle(), zaxis) *
                    glm::scale(glm::mat4(1.0f), glm::vec3{size, size, size});
            }
        }
        if (m_gameBoard.isMoveInProgress()) {
            auto rc = m_gameBoard.moveRC();
            auto &b = m_gameBoard.block(rc.first, rc.second);
            float size = b.component()->componentSize()/m_modelSize;
            auto &placement = b.component()->placement(b.placementIndex());
            glm::vec2 xy = placement.movePositionSoFar();
            auto ref = b.component()->objReference();
            if (ref.first != Component::MovableType::dynamicObj) {
                // shouldn't happen
                return true;
            }

            objs[ref.second].first->modelMatrices[b.placementIndex()] =
                    glm::translate(glm::mat4(1.0f), glm::vec3{xy.x, xy.y, m_zMovingPlacement}) *
                    glm::rotate(glm::mat4(1.0f), placement.rotationAngle(), zaxis) *
                    glm::scale(glm::mat4(1.0f), glm::vec3{size, size, size});
        }

        objs[m_objsReferenceBall].first->modelMatrices[0] =
                glm::translate(glm::mat4(1.0f), m_ball.position) *
                glm::mat4_cast(m_ball.totalRotated) *
                glm::scale(glm::mat4(1.0f), glm::vec3{m_scaleBall, m_scaleBall, m_scaleBall});
    }
}

bool MovablePassage::drag(float startX, float startY, float distanceX, float distanceY) {
    if (!m_initDone) {
        return false;
    }

    glm::vec2 startPosition{startX, startY};
    glm::vec2 distance{distanceX, distanceY};
    return m_gameBoard.drag(startPosition, distance);
}

bool MovablePassage::dragEnded(float x, float y) {
    glm::vec2 endPosition{x, y};
    return m_gameBoard.dragEnded(endPosition);
}

bool MovablePassage::tap(float x, float y) {
    glm::vec2 position{x, y};
    return m_gameBoard.tap(position);
}
