#include <list>
#define GLM_FORCE_RADIANS
#include <glm/gtx/transform.hpp>
#include "levelFinish.hpp"

ManyQuadCoverUpLevelFinish::ManyQuadCoverUpLevelFinish() : totalNumberReturned(0) {
    prevTime = std::chrono::high_resolution_clock::now();
    imagePaths =
            {"textures/flower1.png",
             "textures/flower2.png",
             "textures/flower3.png",
             "textures/flower4.png"};
    float range = 1.5f;
    for (uint32_t i = 0; i < totalNumberObjectsForSide; i++) {
        for (uint32_t j = 0; j < totalNumberObjectsForSide; j++) {
            if (translateVectors.size() > 0) {
                uint32_t index = random.getUInt(0, translateVectors.size());
                std::list<glm::vec3>::iterator it = translateVectors.begin();
                for (int k = 0; k < index; k++) {
                    it++;
                }
                translateVectors.insert(it, glm::vec3(2.0f / (totalNumberObjectsForSide - 1) * i - 1.0f,
                                          range / (totalNumberObjectsForSide - 1) * j - range/2,
                                          0.0f));
            } else {
                translateVectors.push_back(glm::vec3(2.0f / (totalNumberObjectsForSide - 1) * i - 1.0f,
                                         range / (totalNumberObjectsForSide - 1) * j - range/2,
                                         0.0f));
            }
        }
    }
}

uint32_t ManyQuadCoverUpLevelFinish::getTotalNumberObjects() {
    return totalNumberObjects;
}

std::shared_ptr<DrawObject> ManyQuadCoverUpLevelFinish::getNextDrawObject() {
    if (totalNumberReturned >= totalNumberObjects) {
        return std::shared_ptr<DrawObject>();
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count();

    if (time < timeThreshold) {
        return std::shared_ptr<DrawObject>();
    }
    prevTime = currentTime;

    float sideLength = random.getFloat(0.2f, 0.4f);
    glm::mat4 scale = glm::scale(glm::vec3(sideLength, sideLength, 1.0f));
    glm::vec3 translateVector = translateVectors.back();
    translateVectors.pop_back();
    translateVector.z = 0.1f+totalNumberReturned*0.01f;
    glm::mat4 trans = glm::translate(translateVector);

    std::string const &image = imagePaths[random.getUInt(0, imagePaths.size()-1)];


    std::shared_ptr<DrawObject> obj(new DrawObject());
    obj->modelMatrices.push_back(trans*scale);
    obj->imagePath = image;
    getQuad(obj->vertices, obj->indices);

    totalNumberReturned++;
    return obj;
}
