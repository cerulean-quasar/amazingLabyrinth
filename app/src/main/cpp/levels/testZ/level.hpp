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

        Level(
                levelDrawer::Adaptor inLevelDrawer,
                std::shared_ptr<LevelConfigData> const &lcd,
                std::shared_ptr<LevelSaveData> const &levelRestoreData,
                float maxZ)
                : basic::Level(std::move(inLevelDrawer), lcd, maxZ, true),
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

            m_ref1 = m_levelDrawer.addObject(std::make_shared<levelDrawer::ModelDescriptionQuad>(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.8f, 0.2f, 0.2f}), nullptr/*,
                                     std::make_shared<levelDrawer::TextureDescriptionPath>(m_ballTexture)*/);

            m_m1 = glm::rotate(glm::scale(m_m1, glm::vec3{2.0f, 2.0f, 1.0f}), boost::math::constants::pi<float>()/4.0f, glm::vec3{0.0f, 1.0f, 0.0f});
            m_m1 = glm::translate(m_m1, glm::vec3{0.0f, 0.0f, maxZ+0.5});
            m_ref1data = m_levelDrawer.addModelMatrixForObject(m_ref1, m_m1);

            m_ref2 = m_levelDrawer.addObject(std::make_shared<levelDrawer::ModelDescriptionQuad>(),
                                                                        std::make_shared<levelDrawer::TextureDescriptionPath>(lcd->holeTexture));

            m_m2 = glm::rotate(glm::scale(m_m2, glm::vec3{2.0f, 2.0f, 1.0f}), -boost::math::constants::pi<float>()/4.0f, glm::vec3{0.0f, 1.0f, 0.0f});
            m_m2 = glm::translate(m_m2, glm::vec3{0.0f, 0.0f, maxZ+0.5});
            m_ref2data = m_levelDrawer.addModelMatrixForObject(m_ref2, m_m2);
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
            m_levelDrawer.updateModelMatrixForObject(m_ref1, m_ref1data,m_m1);
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