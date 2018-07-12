#ifndef AMAZING_LABYRINTH_LEVEL_FINISH_HPP
#define AMAZING_LABYRINTH_LEVEL_FINISH_HPP

#include <cstdint>
#include <vector>
#include <list>
#include "graphics.hpp"
#include "random.hpp"

class LevelFinish {
public:
    virtual uint32_t getTotalNumberObjects() = 0;
    virtual std::shared_ptr<DrawObject> getNextDrawObject() = 0;
    virtual bool isDone() = 0;
    virtual ~LevelFinish() {}
};

class ManyQuadCoverUpLevelFinish : public LevelFinish {
private:
    uint32_t const totalNumberObjectsForSide = 5;
    uint32_t const totalNumberObjects = totalNumberObjectsForSide * totalNumberObjectsForSide;
    std::list<glm::vec3> translateVectors;

    // every timeThreshold, a new image appears, covering up the maze.
    float const timeThreshold = 0.05f;

    Random random;
    uint32_t totalNumberReturned;

    std::chrono::high_resolution_clock::time_point prevTime;
    std::vector<std::string> imagePaths;
public:
    virtual uint32_t getTotalNumberObjects();
    virtual std::shared_ptr<DrawObject> getNextDrawObject();
    virtual bool isDone() { return totalNumberReturned == totalNumberObjects; }
    ManyQuadCoverUpLevelFinish();
};
#endif