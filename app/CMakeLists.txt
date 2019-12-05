# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set (platform64_files)
if ((NOT ${CMAKE_ANDROID_ARCH_ABI} STREQUAL x86) AND (NOT ${CMAKE_ANDROID_ARCH_ABI} STREQUAL armeabi-v7a))
    list(APPEND platform64_files src/main/cpp/graphicsVulkan.cpp src/main/cpp/mazeVulkan.cpp src/main/cpp/vulkanWrapper.cpp)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCQ_ENABLE_VULKAN")
endif ((NOT ${CMAKE_ANDROID_ARCH_ABI} STREQUAL x86) AND (NOT ${CMAKE_ANDROID_ARCH_ABI} STREQUAL armeabi-v7a))

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        src/main/cpp/native-lib.cpp
        src/main/cpp/gameRequester.cpp
        src/main/cpp/android.cpp
        src/main/cpp/graphics.cpp
        src/main/cpp/graphicsGL.cpp
        src/main/cpp/mazeGraphics.cpp
        src/main/cpp/mazeGL.cpp
        ${platform64_files}
        src/main/cpp/random.cpp
        src/main/cpp/drawer.cpp
        src/main/cpp/serializeSaveData.cpp
        src/main/cpp/level/levelSerializer.cpp
        src/main/cpp/level/levelFinish.cpp
        src/main/cpp/level/levels/maze.cpp
        src/main/cpp/level/levels/mazeSerializer.cpp
        src/main/cpp/level/levels/openAreaLevel.cpp
        src/main/cpp/level/levels/openAreaLevelSerializer.cpp
        src/main/cpp/level/levels/avoidVortexLevel.cpp
        src/main/cpp/level/levels/avoidVortexLevelSerializer.cpp
        src/main/cpp/level/levels/movingQuadsLevel.cpp
        src/main/cpp/level/levels/movingQuadsLevelSerializer.cpp
        src/main/cpp/level/levels/mazeOpenArea.cpp
        src/main/cpp/level/levels/mazeAvoid.cpp
        src/main/cpp/level/levels/mazeAvoidSerializer.cpp
        src/main/cpp/level/levels/mazeCollect.cpp
        src/main/cpp/level/levels/mazeCollectSerializer.cpp
        src/main/cpp/level/levelTracker.cpp
        src/main/cpp/level/levelStarter.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

include_directories(${ANDROID_NDK}/sysroot/usr/include/android)
include_directories(${ANDROID_NDK}/sources/third_party/vulkan/src/libs)
include_directories(APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
include_directories(/opt/glm-0.9.9.5/glm)
include_directories(/opt/tinyobjloader/include)
include_directories(/opt/stb/include)
include_directories(/opt/jsonforcpp)
include_directories(/opt/boost_1_70_0)
set(BOOST_ROOT "/opt/boost_1_70_0")

set(CQ_COMPILE_FLAGS)
if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
    list(APPEND CQ_COMPILE_FLAGS -DDEBUG)
endif(${CMAKE_BUILD_TYPE} STREQUAL Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -std=c++14 ${CQ_COMPILE_FLAGS}")

target_compile_definitions(native-lib PUBLIC -DVK_USE_PLATFORM_ANDROID_KHR)


# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib}
                       EGL
                       GLESv3
                       android)
