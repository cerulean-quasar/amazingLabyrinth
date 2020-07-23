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
#include <vector>
#include "level.hpp"
#include "../basic/level.hpp"

namespace movablePassage {
    char constexpr const *Level::m_name;

    void Level::initSetGameBoard(
            uint32_t nbrTilesX,
            uint32_t nbrTilesY,
            uint32_t startColumn,
            uint32_t endColumn) {
        float tileSizeX = m_width / nbrTilesX;
        float tileSizeY = m_height / (nbrTilesY + m_nbrTileRowsForStart +
                                      m_nbrTileRowsForEnd);
        float tileSize = tileSizeX < tileSizeY ? tileSizeX : tileSizeY;
        uint32_t nbrExtraTileRowsX = static_cast<uint32_t>(std::floor(
                (m_width - tileSize * nbrTilesX) / tileSize));
        uint32_t nbrExtraTilesX = nbrExtraTileRowsX * nbrTilesY;

        // minus 2 * nbrExtraTileRowsY for up tunnel components
        uint32_t nbrExtraTileRowsY = static_cast<uint32_t>(std::floor(
                (m_height - tileSize * (nbrTilesY +
                                        m_nbrTileRowsForEnd + m_nbrTileRowsForStart)) / tileSize));
        uint32_t nbrExtraTilesY = nbrExtraTileRowsY * nbrTilesX - 2 * nbrExtraTileRowsY;

        // we have a device with a screen of a similar size to our tunnel building area,
        // get some more space by making the movable space less.
        if (nbrExtraTilesX + nbrExtraTilesY < m_nbrComponents) {
            uint32_t moreExtraComponentsRequired =
                    m_nbrComponents - (nbrExtraTilesX + nbrExtraTilesY);
            uint32_t nbrExtraPerimeters =
                    moreExtraComponentsRequired / (2 * nbrTilesX + 2 * nbrTilesY) + 1;
            tileSizeX = m_width / (nbrTilesX + 2 * nbrExtraPerimeters);
            tileSizeY = m_height / (nbrTilesY + m_nbrTileRowsForStart +
                                    m_nbrTileRowsForEnd + 2 * nbrExtraPerimeters);
            tileSize = tileSizeX < tileSizeY ? tileSizeX : tileSizeY;

            nbrExtraTileRowsX = static_cast<uint32_t>(std::floor(
                    m_width / tileSize - nbrTilesX));

            // minus 2 * nbrExtraTileRowsY for up tunnel components
            nbrExtraTileRowsY = static_cast<uint32_t>(std::floor(
                    m_height / tileSize - (nbrTilesY +
                                           m_nbrTileRowsForStart +
                                           m_nbrTileRowsForEnd)));
        }

        m_gameBoardStartRowColumn.first = m_nbrTileRowsForStart + nbrExtraTileRowsY / 2;
        m_gameBoardStartRowColumn.second = nbrExtraTileRowsX / 2;

        m_gameBoardEndRowColumn.first =
                m_nbrTileRowsForStart + nbrTilesY + nbrExtraTileRowsY / 2 - 1;
        m_gameBoardEndRowColumn.second = nbrTilesX + nbrExtraTileRowsX / 2 - 1;

        m_scaleBall = tileSize / 2.0f / m_originalBallDiameter;
        size_t nbrTilesHeight = nbrTilesY + nbrExtraTileRowsY + m_nbrTileRowsForStart +
                                m_nbrTileRowsForEnd;
        m_gameBoard.initialize(
                tileSize,
                glm::vec3{0.0f, 0.0f, m_mazeFloorZ},
                nbrTilesHeight,
                nbrTilesX + nbrExtraTileRowsX,
                m_nbrTileRowsForStart,
                m_nbrTileRowsForEnd);
        if (m_gameBoard.heightInTiles() == 0 || m_gameBoard.widthInTiles() == 0) {
            throw std::runtime_error("MovablePassage game board blocks not initialized properly");
        }

        // The main board area for constructing the tunnel
        for (uint32_t k = m_gameBoardStartRowColumn.first;
             k <= m_gameBoardEndRowColumn.first; k++) {
            for (uint32_t l = m_gameBoardStartRowColumn.second;
                 l <= m_gameBoardEndRowColumn.second; l++) {
                m_gameBoard.block(k, l).setBlockType(GameBoardBlock::BlockType::onBoard);
            }
        }

        // save the first position where the ball can roll to that can have a user placed placement.
        // used to save the level state.
        m_ballFirstPlaceableComponent = std::make_pair(m_gameBoardStartRowColumn.first,
                m_gameBoardStartRowColumn.second + startColumn);

        // the fixed tunnel through the places for extra tunnel pieces at the bottom.
        for (uint32_t k = m_nbrTileRowsForStart;
             k < m_nbrTileRowsForStart + nbrExtraTileRowsY / 2;
             k++) {
            auto comp = m_components[Component::ComponentType::straight];
            auto pos = comp->add(k, m_gameBoardStartRowColumn.second + startColumn, 0.0f, true);
            auto &block = m_gameBoard.block(k, m_gameBoardStartRowColumn.second + startColumn);
            block.setComponent(comp, pos);
            block.setBlockType(GameBoardBlock::BlockType::onBoard);
        }

        // the fixed tunnel through the places for extra tunnel pieces at the top.
        auto &comp = m_components[Component::ComponentType::straight];
        for (uint32_t k = m_gameBoardEndRowColumn.first + 1;
             k < m_gameBoard.heightInTiles() - m_nbrTileRowsForEnd;
             k++) {
            auto pos = comp->add(k, m_gameBoardStartRowColumn.second + endColumn, 0.0f, true);
            auto &block = m_gameBoard.block(k, m_gameBoardStartRowColumn.second + endColumn);
            block.setComponent(comp, pos);
            block.setBlockType(GameBoardBlock::BlockType::onBoard);
        }

        // The start
        for (uint32_t k = 0; k < m_nbrTileRowsForStart; k++) {
            for (uint32_t l = 0; l < m_gameBoard.widthInTiles(); l++) {
                Component::ComponentType type = Component::ComponentType::open;
                uint32_t nbr90DegreeRotations = 0;
                if (k == 0) {
                    if (l == 0) {
                        type = Component::ComponentType::closedCorner;
                    } else if (l == m_gameBoard.widthInTiles() - 1) {
                        type = Component::ComponentType::closedCorner;
                        nbr90DegreeRotations = 1;
                    } else {
                        type = Component::ComponentType::closedBottom;
                    }
                } else if (k == m_nbrTileRowsForStart - 1) {
                    if (l == 0) {
                        type = Component::ComponentType::closedCorner;
                        nbr90DegreeRotations = 3;
                    } else if (l == m_gameBoard.widthInTiles() - 1) {
                        type = Component::ComponentType::closedCorner;
                        nbr90DegreeRotations = 2;
                    } else if (l == m_gameBoardStartRowColumn.second + startColumn) {
                        type = Component::ComponentType::open;
                        nbr90DegreeRotations = 0;
                    } else {
                        type = Component::ComponentType::closedBottom;
                        nbr90DegreeRotations = 2;
                    }
                } else if (l == 0) {
                    type = Component::ComponentType::closedBottom;
                    nbr90DegreeRotations = 3;
                } else if (l == m_gameBoard.widthInTiles() - 1) {
                    type = Component::ComponentType::closedBottom;
                    nbr90DegreeRotations = 1;
                }
                auto &compEnd = m_components[type];
                auto pos = compEnd->add(k, l, nbr90DegreeRotations, true);
                auto &block = m_gameBoard.block(k, l);
                block.setComponent(compEnd, pos);
                block.setBlockType(GameBoardBlock::BlockType::begin);
            }
        }

        // The end
        for (uint32_t k = m_gameBoard.heightInTiles() - m_nbrTileRowsForEnd;
             k < m_gameBoard.heightInTiles();
             k++) {
            for (uint32_t l = 0; l < m_gameBoard.widthInTiles(); l++) {
                auto &block = m_gameBoard.block(k, l);
                if (k == m_gameBoard.heightInTiles() - m_nbrTileRowsForEnd &&
                    l == m_gameBoardStartRowColumn.second + endColumn) {
                    block.setBlockType(GameBoardBlock::BlockType::end);
                    m_columnEndPosition = l;
                } else {
                    block.setBlockType(GameBoardBlock::endOffBoard);
                }
                // don't set a component - we detect the end by the block type
            }
        }

        for (auto &component : m_components) {
            component->setSize(tileSize);
        }
    }

    void Level::initAddPlayableComponents(std::shared_ptr<LevelSaveData> const &sd) {
        if (m_nbrComponents == 0) {
            throw std::runtime_error("MovablePassage components not initialized properly");
        }

        if (sd) {
            // place the passage components on the segments that they where on before.
            auto restorePlacements = [](
                    GameBoard &gameBoard,
                    std::shared_ptr<Component> const &component,
                    std::vector<PlacementSaveData> const &vec) -> void
            {
                size_t placementNbr = 0;
                for (auto placementIt = component->placementsBegin();
                     placementIt != component->placementsEnd();
                     placementIt++, placementNbr++)
                {
                    for (uint32_t i = 0; i < vec[placementNbr].nbr90DegreeRotations; i++) {
                        placementIt->rotate();
                    }

                    placementIt->setRC(vec[placementNbr].rowCol.x, vec[placementNbr].rowCol.y);
                    auto &b = gameBoard.block(vec[placementNbr].rowCol.x, vec[placementNbr].rowCol.y);
                    b.setComponent(component, placementNbr);
                }
            };

            restorePlacements(m_gameBoard, m_components[Component::ComponentType::straight], sd->straightPositions);
            restorePlacements(m_gameBoard, m_components[Component::ComponentType::tjunction], sd->tjunctionPositions);
            restorePlacements(m_gameBoard, m_components[Component::ComponentType::crossjunction], sd->crossjunctionPositions);
            restorePlacements(m_gameBoard, m_components[Component::ComponentType::turn], sd->turnPositions);

            restorePathLockedInPlace(m_gameBoard, sd->pathLockedInPlace);

            // restore the ball row and column
            m_ballRow = sd->ballRC.x;
            m_ballCol = sd->ballRC.y;
            m_ball.position.x = sd->ballPosition.x;
            m_ball.position.y = sd->ballPosition.y;
            auto position = m_gameBoard.position(m_ballRow, m_ballCol);
            m_ball.position.z = position.z;
        } else {
            // place the passage components in the off board sections
            auto it1 = m_components.begin();
            auto end1 = m_components.end();
            if (it1 == end1) {
                throw std::runtime_error("MovablePassage components not initialized properly");
            }

            auto it2 = (*it1)->placementsBegin();
            auto end2 = (*it1)->placementsEnd();
            uint32_t m = 0;
            bool finished = false;
            for (uint32_t k = 0; k < m_gameBoard.heightInTiles() && !finished; k++) {
                for (uint32_t l = 0; l < m_gameBoard.widthInTiles(); /* increment in loop */) {
                    if (((*it1)->type() != Component::ComponentType::straight &&
                         (*it1)->type() != Component::ComponentType::tjunction &&
                         (*it1)->type() != Component::ComponentType::turn &&
                         (*it1)->type() != Component::ComponentType::crossjunction) ||
                        it2 == end2) {
                        it1++;

                        if (it1 == end1) {
                            finished = true;
                            break;
                        }

                        m = 0;
                        it2 = (*it1)->placementsBegin();
                        end2 = (*it1)->placementsEnd();
                        continue;
                    }
                    auto &block = m_gameBoard.block(k, l);
                    if (block.blockType() != GameBoardBlock::BlockType::offBoard ||
                        block.component() != nullptr) {
                        l++;
                        continue;
                    }
                    if (it2 != end2) {
                        if (!it2->lockedIntoPlace()) {
                            block.setComponent(*it1, m);
                            it2->setRC(k, l);

                            m++;
                            l++;
                        }
                        it2++;
                    }
                }
            }

            m_ballRow = 0;
            m_ballCol = m_gameBoard.widthInTiles() / 2;
            m_ball.position = m_gameBoard.position(m_ballRow, m_ballCol);
        }
    }

    void Level::initDone(std::shared_ptr<LevelSaveData> const &sd) {
        // add the rock segments that were requested of us.
        std::shared_ptr<Component> &rock = m_components[Component::ComponentType::noMovementRock];
        for (auto const &rc: m_addedRocks) {
            uint32_t row = rc.first + m_gameBoardStartRowColumn.first;
            uint32_t col = rc.second + m_gameBoardStartRowColumn.second;

            size_t placementIndex = rock->add(row, col, 0.0f, true);
            auto &block = m_gameBoard.block(row, col);
            block.setComponent(rock, placementIndex);
        }

        // add the dirt and rock placements for parts of the board that are not covered yet.
        // rock for the off board placements, dirt for the on board
        std::shared_ptr<Component> dirt = m_components[Component::ComponentType::noMovementDirt];
        for (uint32_t k = m_nbrTileRowsForStart;
             k < m_gameBoard.heightInTiles() - m_nbrTileRowsForEnd; k++) {
            for (uint32_t l = 0; l < m_gameBoard.widthInTiles(); l++) {
                auto &b = m_gameBoard.block(k, l);
                if (b.blockType() == GameBoardBlock::BlockType::onBoard) {
                    auto index = dirt->add(k, l, 0.0f);
                    if (b.component() == nullptr) {
                        b.setComponent(dirt, index);
                    } else {
                        b.setSecondaryComponent(dirt, index);
                    }
                } else {
                    auto index = rock->add(k, l, 0.0f);
                    if (b.component() == nullptr) {
                        b.setComponent(rock, index);
                    } else {
                        b.setSecondaryComponent(rock, index);
                    }
                }
            }
        }

        // restore the model/texture for each game board component
        if (sd) {
            for (size_t row = 0; row < m_gameBoard.heightInTiles() - m_nbrTileRowsForEnd; row++) {
                for (size_t col = 0; col < m_gameBoard.widthInTiles(); col++) {
                    auto &b = m_gameBoard.block(row, col);
                    b.component()->placement(b.placementIndex()).setObjReference(sd->gameBoardObjReferences[row][col]);
                }
            }
        }
        m_initDone = true;
    }

    bool Level::updateData() {
        if (!m_initDone) {
            return false;
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        float timeDiff = std::chrono::duration<float, std::chrono::seconds::period>(
                currentTime - m_prevTime).count();
        float timeDiffTotal = timeDiff;
        if (timeDiff < m_floatErrorAmount) {
            return false;
        }
        m_prevTime = currentTime;

        glm::vec3 position = m_ball.position;
        glm::vec3 prevPosition = position;
        m_ball.velocity = getUpdatedVelocity(m_ball.acceleration, timeDiff);
        if (glm::length(m_ball.velocity) < m_floatErrorAmount) {
            return false;
        }

        uint32_t nbrComputations = 0;
        bool drawingNecessary_ = false;
        glm::vec3 posFromCenter = position - m_gameBoard.position(m_ballRow, m_ballCol);

        Component::checkForNextWallFunc checkforNextWall{
                [&](Component::CellWall wall1, Component::CellWall wall2)
                        -> std::pair<bool, bool> {
                    return m_gameBoard.checkforNextWall(wall1, wall2, m_ballRow, m_ballCol);
                }
        };
        auto &block = m_gameBoard.block(m_ballRow, m_ballCol);
        auto walls = block.component()->moveBallInCell(
                block.placementIndex(), posFromCenter, timeDiff, m_ball.velocity, ballRadius(),
                checkforNextWall);

        position = m_gameBoard.position(m_ballRow, m_ballCol) + posFromCenter;
        if (walls.first == Component::noWall && walls.second == Component::noWall) {
            m_ball.position = position;
            updateRotation(timeDiffTotal);
            return drawingNecessary();
        }
        if (walls.first == Component::CellWall::wallLeft ||
            walls.second == Component::CellWall::wallLeft) {
            m_ballCol--;
        } else if (walls.first == Component::CellWall::wallRight ||
                   walls.second == Component::CellWall::wallRight) {
            m_ballCol++;
        }
        if (walls.first == Component::CellWall::wallDown ||
            walls.second == Component::CellWall::wallDown) {
            m_ballRow--;
        } else if (walls.first == Component::CellWall::wallUp ||
                   walls.second == Component::CellWall::wallUp) {
            m_ballRow++;
        }

        auto &nextBlock = m_gameBoard.block(m_ballRow, m_ballCol);
        if (nextBlock.blockType() == GameBoardBlock::BlockType::end) {
            // winning condition.  Set m_finished and return that drawing is necessary.
            m_finished = true;
            m_ball.position = position;
            updateRotation(timeDiffTotal);
            return true;
        } else if (block.blockType() == GameBoardBlock::BlockType::begin &&
                   nextBlock.blockType() == GameBoardBlock::BlockType::begin) {
            // advance the ball into the next cell, but don't track its path
            m_ball.position = position;
            updateRotation(timeDiffTotal);
            return drawingNecessary();
        }

        // the ball advanced to the next cell.  Track the ball moving into the next component.
        blockUnblockPlacements(block.component(), block.placementIndex(),
                               nextBlock.component(), nextBlock.placementIndex());

        m_ball.position = position;
        updateRotation(timeDiffTotal);

        // always redraw if we got to this point because we changed the path that the ball was on
        // and thus changed the textures.  We need to redraw to have the effects take place.
        return true;
    }

    void Level::addStaticDrawObjects() {
        // the end
        glm::vec3 endScale = m_gameBoard.scaleEndObject();
        endScale.x /= m_modelSize;
        endScale.y /= m_modelSize;
        auto objIndexEnd = m_levelDrawer.addObject(
                std::make_shared<levelDrawer::ModelDescriptionQuad>(),
                std::make_shared<levelDrawer::TextureDescriptionPath>(m_componentTextureEnd));
        auto objIndexEndOffBoard = m_levelDrawer.addObject(
                std::make_shared<levelDrawer::ModelDescriptionQuad>(),
                std::make_shared<levelDrawer::TextureDescriptionPath>(m_textureEndOffBoard));
        for (uint32_t col = 0; col < m_gameBoard.widthInTiles(); col++) {
            glm::vec3 endPosition = m_gameBoard.position(
                    m_gameBoard.heightInTiles() - m_nbrTileRowsForEnd, col);
            if (col == m_columnEndPosition) {
                m_levelDrawer.addModelMatrixForObject(
                        objIndexEnd,
                        glm::translate(glm::mat4(1.0f), endPosition) *
                        glm::scale(glm::mat4(1.0f), endScale));
            } else {
                m_levelDrawer.addModelMatrixForObject(
                        objIndexEndOffBoard,
                        glm::translate(glm::mat4(1.0f), endPosition) *
                        glm::scale(glm::mat4(1.0f), endScale));
            }
        }

        // add extra end tiles to the left and right to make the entire surface be filled in (no bg
        // color around the edges).  Always do this even if the width of the game board is the same
        // size as the surface at depth: m_mazeFloorZ because we will need it because of perspective.
        // The end tiles are lower than m_mazeFloorZ.
        glm::vec3 endPosition = m_gameBoard.position(
                m_gameBoard.heightInTiles() - m_nbrTileRowsForEnd, 0);
        endPosition.x -= m_gameBoard.blockSize();
        m_levelDrawer.addModelMatrixForObject(
                objIndexEndOffBoard,
                glm::translate(glm::mat4(1.0f), endPosition) * glm::scale(glm::mat4(1.0f), endScale));

        endPosition = m_gameBoard.position(
                m_gameBoard.heightInTiles() - m_nbrTileRowsForEnd, m_gameBoard.widthInTiles() - 1);
        endPosition.x += m_gameBoard.blockSize();
        m_levelDrawer.addModelMatrixForObject(
                objIndexEndOffBoard,
                glm::translate(glm::mat4(1.0f), endPosition) *
                glm::scale(glm::mat4(1.0f), endScale));

        // the start: closed corner type
        auto objRefsStartCorner = addObjs<levelDrawer::ModelDescriptionCube>(
                m_levelDrawer,
                false,
                m_componentModels[Component::ComponentType::closedCorner],
                m_componentTextures[Component::ComponentType::closedCorner]);

        // the start: closed bottom type
        auto objRefsStartSide = addObjs<levelDrawer::ModelDescriptionCube>(
                m_levelDrawer,
                false,
                m_componentModels[Component::ComponentType::closedBottom],
                m_componentTextures[Component::ComponentType::closedBottom]);

        // the start: open type
        auto objRefsStartCenter = addObjs<levelDrawer::ModelDescriptionCube>(
                m_levelDrawer,
                false,
                m_componentModels[Component::ComponentType::open],
                m_componentTextures[Component::ComponentType::open]);

        // the scale for the following are all the same.
        float scale = m_gameBoard.blockSize() / m_modelSize;

        // the start: model matrices.
        bool done = false;
        for (uint32_t i = 0; i < m_gameBoard.heightInTiles() && !done; i++) {
            for (uint32_t j = 0; j < m_gameBoard.widthInTiles(); j++) {
                if (m_gameBoard.blockType(i, j) != GameBoardBlock::BlockType::begin) {
                    done = true;
                    break;
                }
                auto &component = m_gameBoard.block(i, j).component();
                size_t placementIndex = m_gameBoard.block(i, j).placementIndex();
                auto &placement = component->placement(placementIndex);
                glm::mat4 modelMatrix =
                        glm::translate(glm::mat4(1.0f), m_gameBoard.position(i, j)) *
                        glm::rotate(glm::mat4(1.0f), placement.rotationAngle(),
                                    glm::vec3{0.0f, 0.0f, 1.0f}) *
                        glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
                switch (component->type()) {
                    case Component::ComponentType::closedCorner:
                        addModelMatrixToObj(m_levelDrawer, m_random, objRefsStartCorner, component, placementIndex,
                                            modelMatrix);
                        break;
                    case Component::ComponentType::closedBottom:
                        addModelMatrixToObj(m_levelDrawer, m_random, objRefsStartSide, component, placementIndex,
                                            modelMatrix);
                        break;
                    case Component::ComponentType::open:
                        addModelMatrixToObj(m_levelDrawer, m_random, objRefsStartCenter, component, placementIndex,
                                            modelMatrix);
                        break;
                    default:
                        break;
                }
            }
        }

        // fill in the bottom and sides with rocks so that no bg color shows.
        auto objRefsRock = addObjs<levelDrawer::ModelDescriptionCube>(
                m_levelDrawer,
                false,
                m_componentModels[Component::ComponentType::noMovementRock],
                m_componentTextures[Component::ComponentType::noMovementRock]);

        // on the sides
        for (size_t i = 0;
             i < m_gameBoard.heightInTiles() - m_nbrTileRowsForEnd;
             i++) {
            glm::vec3 pos = m_gameBoard.position(i, 0);
            pos.x -= m_gameBoard.blockSize();
            addModelMatrixToObj(
                    m_levelDrawer,m_random, objRefsRock, nullptr, 0,
                    glm::translate(glm::mat4(1.0f), pos) *
                    glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale)));

            pos = m_gameBoard.position(i, m_gameBoard.widthInTiles() - 1);
            pos.x += m_gameBoard.blockSize();
            addModelMatrixToObj(
                    m_levelDrawer, m_random, objRefsRock, nullptr, 0,
                    glm::translate(glm::mat4(1.0f), pos) *
                    glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale)));
        }

        // on the bottom
        // always add these because the gameboard has to be shifted up to cover up the end patch because
        // the end is not at m_mazeFloorZ.
        for (size_t i = 0; i < m_gameBoard.widthInTiles(); i++) {
            glm::vec3 pos = m_gameBoard.position(0, i);
            pos.y -= m_gameBoard.blockSize();
            addModelMatrixToObj(
                    m_levelDrawer, m_random, objRefsRock, nullptr, 0,
                    glm::translate(glm::mat4(1.0f), pos) *
                    glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale)));
        }

        // the left lower corner
        glm::vec3 pos = m_gameBoard.position(0, 0);
        pos.y -= m_gameBoard.blockSize();
        pos.x -= m_gameBoard.blockSize();
        addModelMatrixToObj(
                m_levelDrawer, m_random, objRefsRock, nullptr, 0,
                glm::translate(glm::mat4(1.0f), pos) *
                glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale)));

        // the right lower corner
        pos = m_gameBoard.position(0, m_gameBoard.widthInTiles() - 1);
        pos.y -= m_gameBoard.blockSize();
        pos.x += m_gameBoard.blockSize();
        addModelMatrixToObj(
                m_levelDrawer, m_random, objRefsRock, nullptr, 0,
                glm::translate(glm::mat4(1.0f), pos) *
                glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale)));
    }

    void Level::addComponentModelMatrices(boost::optional<size_t> const &ballReference) {
        glm::vec3 zaxis{0.0f, 0.0f, 1.0f};
        bool moveInProgress = m_gameBoard.isMoveInProgress();
        auto rc = m_gameBoard.moveRC();
        std::vector<size_t> nbrPlacements;
        auto nbrObjs = m_levelDrawer.numberObjects();
        nbrPlacements.resize(nbrObjs, 0);

        auto addModelMatrix = [&](std::shared_ptr<Component> const &component,
                                  size_t placementIndex,
                                  float scale, float extraZ, size_t r, size_t c) -> void {
            auto pos = m_gameBoard.position(r, c);
            pos.z += extraZ;
            glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), pos) *
                                    glm::rotate(glm::mat4(1.0f), component->placement(
                                            placementIndex).rotationAngle(), zaxis) *
                                    glm::scale(glm::mat4(1.0f), glm::vec3{scale, scale, scale});
            auto ref = component->placement(placementIndex).objReference();
            if (ref == boost::none || ref.get().objIndex == boost::none) {
                ref = chooseObj(m_random, component, placementIndex);
            }
            if (ref != boost::none && ref.get().objIndex != boost::none)
            {
                // ref should always be valid at this point, but just in case...
                size_t objIndex = ref.get().objIndex.get();

                if (nbrPlacements[objIndex] >= m_levelDrawer.numberObjectsDataForObject(objIndex)) {
                    m_levelDrawer.addModelMatrixForObject(objIndex, modelMatrix);
                } else {
                    m_levelDrawer.updateModelMatrixForObject(objIndex, nbrPlacements[objIndex], modelMatrix);
                }
                nbrPlacements[objIndex]++;
            }
        };

        float scale = m_gameBoard.blockSize() / m_modelSize;
        float extraZ = 3 * m_gameBoard.blockSize() / 4;
        for (size_t i = m_nbrTileRowsForStart;
             i < m_gameBoard.heightInTiles() - m_nbrTileRowsForEnd;
             i++) {
            for (size_t j = 0; j < m_gameBoard.widthInTiles(); j++) {
                auto &b = m_gameBoard.block(i, j);
                if (moveInProgress && rc.first == i && rc.second == j) {
                    // draw the secondary component if the primary component is being moved.
                    addModelMatrix(b.secondaryComponent(), b.secondaryPlacementIndex(),
                                   scale, 0.0f, i, j);
                } else if (b.blockType() == GameBoardBlock::BlockType::offBoard &&
                           b.secondaryComponent() != nullptr) {
                    addModelMatrix(b.component(), b.placementIndex(), 2 * scale / 3, extraZ, i,
                                   j);
                    addModelMatrix(b.secondaryComponent(), b.secondaryPlacementIndex(), scale,
                                   0.0f, i, j);
                } else {
                    addModelMatrix(b.component(), b.placementIndex(), scale, 0.0f, i, j);
                }
            }
        }

        if (moveInProgress) {
            auto &b = m_gameBoard.block(rc.first, rc.second);
            auto &component = b.component();
            float size = b.component()->componentSize() / m_modelSize;
            auto &placement = b.component()->placement(b.placementIndex());
            glm::vec2 xy = placement.movePositionSoFar();

            glm::mat4 modelMatrix =
                    glm::translate(glm::mat4(1.0f), glm::vec3{xy.x, xy.y, m_zMovingPlacement}) *
                    glm::rotate(glm::mat4(1.0f), placement.rotationAngle(), zaxis) *
                    glm::scale(glm::mat4(1.0f), glm::vec3{size, size, size});
            auto ref = placement.objReference();
            size_t objIndex = 0;
            if (ref == boost::none || ref.get().objIndex == boost::none) {
                ref = addModelMatrixToObj(m_levelDrawer, m_random, component->objReferences(),
                                          component, b.placementIndex(), modelMatrix);
                if (ref == boost::none || ref.get().objIndex == boost::none) {
                    throw (std::runtime_error("Unexpected empty obj index"));
                }
                objIndex = ref.get().objIndex.get();
            } else {
                objIndex = ref.get().objIndex.get();
                if (nbrPlacements[objIndex] >= m_levelDrawer.numberObjectsDataForObject(objIndex)) {
                    m_levelDrawer.addModelMatrixForObject(objIndex, modelMatrix);
                } else {
                    m_levelDrawer.updateModelMatrixForObject(objIndex, nbrPlacements[objIndex],
                            modelMatrix);
                }
            }
            nbrPlacements[objIndex]++;
        }

        nbrObjs = m_levelDrawer.numberObjects();
        for (size_t i = 0; i < nbrObjs; i++) {
            if (ballReference == boost::none || i != ballReference.get()) {
                m_levelDrawer.resizeObjectsData(i, nbrPlacements[i]);
            }
        }
    }

    void Level::addDynamicDrawObjects() {
        glm::vec3 zaxis{0.0f, 0.0f, 1.0f};
        float scaleBall = m_gameBoard.blockSize() / m_modelSize / 2.0f;

        std::vector<std::string> texturesLockedIntoPlace{m_textureLockedComponent};
        for (auto const &component: m_components) {
            if (component->type() == Component::ComponentType::closedBottom ||
                component->type() == Component::ComponentType::closedCorner ||
                component->type() == Component::ComponentType::open ||
                component->nbrPlacements() == 0) {
                // These components are handled as static draw objects or we have none of them
                continue;
            }
            auto refs = addObjs<levelDrawer::ModelDescriptionCube>(
                    m_levelDrawer,false,m_componentModels[component->type()],
                    m_componentTextures[component->type()]);
            component->setObjReferences(refs);

            // when components are locked into place and cannot be moved, use these objs.
            // don't do this for the rocks, they are always colored the same way.
            if (component->type() != Component::ComponentType::noMovementRock) {
                refs = addObjs<levelDrawer::ModelDescriptionCube>(
                        m_levelDrawer,true,
                        m_componentModels[component->type()],
                        texturesLockedIntoPlace);
                component->setObjReferencesLockedComponent(refs);
            }
        }

        addComponentModelMatrices(boost::none);

        // the ball
        m_objsIndexBall = m_levelDrawer.addObject(
                std::make_shared<levelDrawer::ModelDescriptionPath>(m_ballModel),
                std::make_shared<levelDrawer::TextureDescriptionPath>(m_ballTexture));

        m_objDataIndexBall = m_levelDrawer.addModelMatrixForObject(
                m_objsIndexBall,
                glm::translate(glm::mat4(1.0f), m_ball.position) *
                glm::mat4_cast(m_ball.totalRotated) *
                glm::scale(glm::mat4(1.0f), glm::vec3{scaleBall, scaleBall, scaleBall}));
    }

    bool Level::updateDrawObjects() {
        float scaleBall = m_gameBoard.blockSize() / m_modelSize / 2.0f;

        addComponentModelMatrices(m_objsIndexBall);

        // the ball
        m_levelDrawer.updateModelMatrixForObject(
                m_objsIndexBall,
                m_objDataIndexBall,
                glm::translate(glm::mat4(1.0f), m_ball.position) *
                glm::mat4_cast(m_ball.totalRotated) *
                glm::scale(glm::mat4(1.0f), glm::vec3{scaleBall, scaleBall, scaleBall}));
    }

    bool Level::drag(float startX, float startY, float distanceX, float distanceY) {
        if (!m_initDone) {
            return false;
        }

        auto projView = m_levelDrawer.getProjectionView();

        auto startXY = getXYAtZ(startX, startY, m_mazeFloorZ, projView.first, projView.second);
        auto distanceXY = getXYAtZ(startX, startY, m_mazeFloorZ, projView.first, projView.second);

        glm::vec2 startPosition{startXY.first, startXY.second};
        glm::vec2 distance{distanceXY.first, distanceXY.second};
        return m_gameBoard.drag(startPosition, distance);
    }

    bool Level::dragEnded(float x, float y) {
        auto projView = m_levelDrawer.getProjectionView();

        auto XY = getXYAtZ(x, y, m_mazeFloorZ, projView.first, projView.second);

        glm::vec2 endPosition{XY.first, XY.second};
        return m_gameBoard.dragEnded(endPosition);
    }

    bool Level::tap(float x, float y) {
        auto projView = m_levelDrawer.getProjectionView();

        auto XY = getXYAtZ(x, y, m_mazeFloorZ, projView.first, projView.second);

        glm::vec2 position{XY.first, XY.second};
        return m_gameBoard.tap(position);
    }
} // namespace movablePassage