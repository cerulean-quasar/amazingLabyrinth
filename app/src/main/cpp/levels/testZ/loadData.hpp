#ifndef AMAZING_LABYRINTH_TESTZ_LOAD_DATA_HPP
#define AMAZING_LABYRINTH_TESTZ_LOAD_DATA_HPP
#include "../basic/loadData.hpp"
namespace testZ {
    int constexpr
    levelSaveDataVersion = 1;

    struct LevelSaveData : public basic::LevelSaveData {
        LevelSaveData(LevelSaveData &&other) noexcept = default;

        LevelSaveData(LevelSaveData const &other) noexcept = default;

        LevelSaveData &operator=(LevelSaveData const &other) noexcept = default;

        LevelSaveData()
                : basic::LevelSaveData{levelSaveDataVersion}
        {}
    };

    struct LevelConfigData : public basic::LevelConfigData {
        std::string holeTexture;

        LevelConfigData()
                : basic::LevelConfigData{},
                  holeTexture{}
        {}

        LevelConfigData(LevelConfigData const &other) noexcept = default;

        LevelConfigData(LevelConfigData &&other) noexcept = default;

        LevelConfigData &operator=(LevelConfigData const &other) noexcept = default;
    };

}
#endif // AMAZING_LABYRINTH_TESTZ_LOAD_DATA_HPP