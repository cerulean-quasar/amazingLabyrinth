#ifndef AMAZING_LABYRINTH_TESTZ_LEVEL_HPP
#define AMAZING_LABYRINTH_TESTZ_LEVEL_HPP
#include <chrono>
#include <boost/math/constants/constants.hpp>
#include "../basic/level.hpp"
#include "../../levelDrawer/modelTable/modelLoader.hpp"
#include "loadData.hpp"

namespace testZ {
    class Level : public basic::Level {
    public:
        static char constexpr const *m_name = "testz";

    struct Request : public basic::Level::Request {
            Request(levelDrawer::Adaptor levelDrawer, bool shadowsEnabled)
                    : basic::Level::Request(std::move(levelDrawer), shadowsEnabled)
            {}
        };
        Level(
                levelDrawer::Adaptor inLevelDrawer,
                std::shared_ptr<basic::LevelConfigData> const &lcd,
                std::shared_ptr<LevelSaveData> const &levelRestoreData,
                float maxZ,
                Request &request)
                : basic::Level(std::move(inLevelDrawer), lcd, maxZ, true, request),
                m_refreshedAfterStarter{false},
                m_ref1{},
                m_ref1data{},
                m_m1{1.0f},
                m_ref2{},
                m_ref2data{},
                m_m2{1.0f}
        {
            if (levelRestoreData == nullptr) {
                m_levelDrawer.setClearColor(glm::vec4{0.0f, 0.0f, 0.0f, 1.0f});
            } else {
                m_levelDrawer.setClearColor(glm::vec4{0.7f, 0.7f, 1.0f, 1.0f});
            }

            auto const &quad1Data = findModelsAndTextures("Quad1");
            m_ref1 = m_levelDrawer.addObject(quad1Data.models[0], getFirstTexture(quad1Data));

            m_m1 = glm::scale(glm::mat4(1.0f), glm::vec3{m_width/2, m_height/2, 1.0f});
            m_m1 = glm::rotate(glm::mat4(1.0f), boost::math::constants::pi<float>()/4.0f, glm::vec3{0.0f, 1.0f, 0.0f}) * m_m1;
            m_m1 = glm::translate(glm::mat4(1.0f), glm::vec3{0.0f, 0.0f, maxZ+0.2f}) * m_m1;
            m_ref1data = m_levelDrawer.addModelMatrixForObject(m_ref1, m_m1);

            auto const &quad2Data = findModelsAndTextures("Quad2");
            m_ref2 = m_levelDrawer.addObject(quad2Data.models[0], getFirstTexture(quad2Data));

            m_m2 = glm::scale(glm::mat4(1.0f), glm::vec3{m_width, m_height, 1.0f});
            m_m2 = glm::rotate(glm::mat4(1.0f), -boost::math::constants::pi<float>()/4.0f, glm::vec3{0.0f, 1.0f, 0.0f}) * m_m2;
            m_m2 = glm::translate(glm::mat4(1.0f), glm::vec3{0.0f, 0.0f, maxZ+0.2}) * m_m2;
            m_ref2data = m_levelDrawer.addModelMatrixForObject(m_ref2, m_m2);


            /*
            auto const &quad3Data = findModelsAndTextures("Quad3");
            levelDrawer::DrawObjReference refq3 = m_levelDrawer.addObject(quad2Data.models[0], getFirstTexture(quad3Data));
             */

            glm::mat4 mq3 = glm::scale(glm::mat4(1.0f), glm::vec3{m_width/4, m_height/4, 0.2f});
            //mq3 = glm::rotate(glm::mat4(1.0f), -boost::math::constants::pi<float>()/4.0f, glm::vec3{0.0f, 1.0f, 0.0f}) * mq3;
            mq3 = glm::translate(glm::mat4(1.0f), glm::vec3{0.0f, 0.0f, maxZ+0.5f}) * mq3;
            //levelDrawer::DrawObjReference refq3data = m_levelDrawer.addModelMatrixForObject(refq3, mq3);

            auto const &ballData = findModelsAndTextures(ModelNameBall);
            levelDrawer::DrawObjReference ref3 = m_levelDrawer.addObject(ballData.models[0],
                                                                         getFirstTexture(ballData));
            glm::mat4 m3 = glm::scale(glm::mat4(1.0f), glm::vec3{m_height/20, m_height/20, m_height/20});
            m3 = glm::translate(glm::mat4(1.0f), glm::vec3{0.0f, 0.0f, maxZ + 0.8f}) * m3;
            m_levelDrawer.addModelMatrixForObject(ref3, m3);
            m_levelDrawer.addModelMatrixForObject(ref3, mq3);
        }

        bool updateData() override {
            if (m_refreshedAfterStarter >= 10) {
                return false;
            } else {
                m_refreshedAfterStarter++;
                return true;
            }
        }

        bool updateDrawObjects() override {
            m_levelDrawer.updateModelMatrixForObject(m_ref1, m_ref1data, m_m1);
            m_levelDrawer.updateModelMatrixForObject(m_ref2, m_ref2data, m_m2);
            return true;
        }

        void start() override {
            prevTime = std::chrono::high_resolution_clock::now();
        }

        void getLevelFinisherCenter(float &x, float &y, float &z) override {
            x = 0.0;
            y = 0.0;
            z = 0.0;
        }

        std::vector<uint8_t> saveData(levelTracker::GameSaveData const &gsd,
                                      char const *saveLevelDataKey) override;

        char const *name() override {
            return m_name;
        }

    private:
        std::chrono::high_resolution_clock::time_point prevTime;

        uint32_t m_refreshedAfterStarter;
        levelDrawer::DrawObjReference m_ref1;
        levelDrawer::DrawObjDataReference m_ref1data;
        glm::mat4 m_m1;

        levelDrawer::DrawObjReference m_ref2;
        levelDrawer::DrawObjDataReference m_ref2data;
        glm::mat4 m_m2;
    };
}
#endif // AMAZING_LABYRINTH_TESTZ_LEVEL_HPP