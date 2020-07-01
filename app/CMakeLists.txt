# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

#set (platform64_files)
#if ((NOT ${CMAKE_ANDROID_ARCH_ABI} STREQUAL x86) AND (NOT ${CMAKE_ANDROID_ARCH_ABI} STREQUAL armeabi-v7a))
#    list(APPEND platform64_files
#            ${ANDROID_NDK}/sources/third_party/vulkan/src/common/vulkan_wrapper.cpp
#            src/main/cpp/graphicsVulkan.cpp
#            src/main/cpp/mazeVulkan.cpp
#            src/main/cpp/vk_mem_alloc.cpp
#            src/main/cpp/levelDrawer/renderDetailsTable/renderDetailsDataVulkan.cpp
#            src/main/cpp/levelDrawer/drawObjectTable/drawObjectDataVulkan.cpp)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCQ_ENABLE_VULKAN")
#endif ((NOT ${CMAKE_ANDROID_ARCH_ABI} STREQUAL x86) AND (NOT ${CMAKE_ANDROID_ARCH_ABI} STREQUAL armeabi-v7a))

set (platform64_files)
if ((NOT ${CMAKE_ANDROID_ARCH_ABI} STREQUAL x86) AND (NOT ${CMAKE_ANDROID_ARCH_ABI} STREQUAL armeabi-v7a))
    add_library(
            graphics-vulkan

            STATIC

            ${ANDROID_NDK}/sources/third_party/vulkan/src/common/vulkan_wrapper.cpp
            src/main/cpp/graphicsVulkan.cpp
            src/main/cpp/mazeVulkan.cpp
            src/main/cpp/vk_mem_alloc.cpp
            src/main/cpp/renderDetails/textureWithShadows/renderDetailsDataVulkan.cpp)

    list(APPEND platform64_files
        graphics-vulkan)
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
        src/main/cpp/graphicsGL.cpp
        src/main/cpp/mazeGraphics.cpp
        src/main/cpp/mazeGL.cpp
        src/main/cpp/random.cpp
        src/main/cpp/drawer.cpp
        src/main/cpp/mathGraphics.cpp
        src/main/cpp/common.cpp
        src/main/cpp/renderDetails/basic/renderDetailsCommonGL.cpp
        src/main/cpp/renderDetails/textureWithShadows/renderDetailsDataGL.cpp
        src/main/cpp/levelDrawer/modelTable/modelLoader.cpp
        src/main/cpp/levelDrawer/textureTable/textureTableGL.cpp
        src/main/cpp/levelDrawer/textureTable/textureLoader.cpp
        src/main/cpp/levels/finisher/types.cpp
        src/main/cpp/levels/finisher/serializer.cpp
        src/main/cpp/levels/generatedMazeAlgorithms.cpp
        src/main/cpp/levels/movablePassageAlgorithms.cpp
        src/main/cpp/levels/movablePassageAlgorithmsSerializer.cpp
        src/main/cpp/levels/avoidVortexMaze/level.cpp
        src/main/cpp/levels/avoidVortexMaze/serializer.cpp
        src/main/cpp/levels/avoidVortexOpenArea/level.cpp
        src/main/cpp/levels/avoidVortexOpenArea/serializer.cpp
        src/main/cpp/levels/basic/level.cpp
        src/main/cpp/levels/basic/serializer.cpp
        src/main/cpp/levels/collectMaze/level.cpp
        src/main/cpp/levels/collectMaze/serializer.cpp
        src/main/cpp/levels/fixedMaze/level.cpp
        src/main/cpp/levels/fixedMaze/serializer.cpp
        src/main/cpp/levels/generatedMaze/level.cpp
        src/main/cpp/levels/generatedMaze/serializer.cpp
        src/main/cpp/levels/movablePassage/level.cpp
        src/main/cpp/levels/movablePassage/serializer.cpp
        src/main/cpp/levels/movingSafeAreas/level.cpp
        src/main/cpp/levels/movingSafeAreas/serializer.cpp
        src/main/cpp/levels/openArea/level.cpp
        src/main/cpp/levels/openArea/serializer.cpp
        src/main/cpp/levels/openAreaMaze/level.cpp
        src/main/cpp/levels/rotatablePassage/level.cpp
        src/main/cpp/levels/rotatablePassage/serializer.cpp
        src/main/cpp/levels/starter/level.cpp
        src/main/cpp/levels/starter/serializer.cpp
        src/main/cpp/levelTracker/levelTracker.cpp
)

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
include_directories(${ANDROID_NDK}/sources/third_party/vulkan/src/common)
include_directories(APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
include_directories(/opt/glm-0.9.9.5/glm)
include_directories(/opt/tinyobjloader/include)
include_directories(/opt/stb/include)
include_directories(/opt/jsonforcpp)
include_directories(/opt/boost_1_70_0)
include_directories(/opt/VulkanMemoryAllocator)
set(BOOST_ROOT "/opt/boost_1_70_0")

set(CQ_COMPILE_FLAGS)
if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
    list(APPEND CQ_COMPILE_FLAGS -DDEBUG)
endif(${CMAKE_BUILD_TYPE} STREQUAL Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wno-unused-parameter -W -std=c++14 ${CQ_COMPILE_FLAGS}")

# GLM #defines:
# (1) GLM_FORCE_DEPTH_ZERO_TO_ONE: tell GLM to use 0 to 1 for the depth.  We will translate to OpenGL's
#     -1 to 1 depth in our code.
# (2) GLM_FORCE_RADIANS: tell GLM to use radians for angles.
target_compile_definitions(native-lib PUBLIC -DVK_USE_PLATFORM_ANDROID_KHR -DGLM_FORCE_DEPTH_ZERO_TO_ONE -DGLM_FORCE_RADIANS)
if ((NOT ${CMAKE_ANDROID_ARCH_ABI} STREQUAL x86) AND (NOT ${CMAKE_ANDROID_ARCH_ABI} STREQUAL armeabi-v7a))
target_compile_definitions(graphics-vulkan PUBLIC -DVK_USE_PLATFORM_ANDROID_KHR -DGLM_FORCE_DEPTH_ZERO_TO_ONE -DGLM_FORCE_RADIANS)
endif ((NOT ${CMAKE_ANDROID_ARCH_ABI} STREQUAL x86) AND (NOT ${CMAKE_ANDROID_ARCH_ABI} STREQUAL armeabi-v7a))


# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib}
                       ${platform64_files}
                       EGL
                       GLESv3
                       android)
