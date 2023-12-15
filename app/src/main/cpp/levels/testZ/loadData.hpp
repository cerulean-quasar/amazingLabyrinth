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

}
#endif // AMAZING_LABYRINTH_TESTZ_LOAD_DATA_HPP