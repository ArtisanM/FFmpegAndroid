cmake_minimum_required(VERSION 3.10.2)

project(nextplayer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wno-deprecated")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG ")

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(EXTRA_DIR "${ROOT_DIR}/../../extra")
set(COMMON_DIR "${ROOT_DIR}/../common")
set(DEMUX_DIR "${ROOT_DIR}/../demux")
set(PLAYER_DIR "${ROOT_DIR}/../")
set(EXTRA_FFMPEG "${EXTRA_DIR}/ffmpeg")

if (CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(TARGET_PLATFORM android)
    set(CMAKE_SYSTEM_VERSION 21)
    set(CMAKE_ANDROID_NDK $ENV{ANDROID_NDK})
    set(EXTRA_FFMPEG_DIR "${EXTRA_DIR}/libs/${CMAKE_ANDROID_ARCH_ABI}")
elseif (CMAKE_SYSTEM_NAME STREQUAL "OHOS")
    add_definitions(-D__HARMONY__)
    set(TARGET_PLATFORM harmony)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-command-line-argument -Wno-c99-designator")
    set(EXTRA_FFMPEG_DIR "${EXTRA_DIR}/libs/${OHOS_ARCH}")
else ()
    message(FATAL_ERROR "This CMake script does not support ${CMAKE_SYSTEM_NAME}!")
endif ()

include_directories(
        "${EXTRA_FFMPEG}/include"
        "${COMMON_DIR}/include"
        "${DEMUX_DIR}"
        "${PLAYER_DIR}"
        "player"
        ${CMAKE_CURRENT_SOURCE_DIR}
)

link_directories("${EXTRA_FFMPEG_DIR}")

set(SRC_LIST
        common/BaseThread.cpp
        common/MediaClock.cpp
        common/NextConfig.cpp
        common/MessageQueue.cpp
        common/NextPacket.cpp
        common/NextPacketQueue.cpp
        common/NextFrameQueue.cpp
        common/NextSpeedMeter.cpp
        kernel/PlayerKernel.cpp
        kernel/handler/MediaParseHandler.cpp
        kernel/handler/AudioDecodeHandler.cpp
        kernel/handler/VideoDecodeHandler.cpp
        kernel/handler/AudioRenderHandler.cpp
        kernel/handler/VideoRenderHandler.cpp
        NextPlayer.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(SRC_LIST ${SRC_LIST} android/next_player_jni.cpp)
else ()
    message(FATAL_ERROR "Don't support ${CMAKE_SYSTEM_NAME}!")
endif ()

add_library(nextplayer SHARED ${SRC_LIST})

target_link_options(nextplayer PRIVATE "-Wl,-z,max-page-size=16384")

if (CMAKE_SYSTEM_NAME STREQUAL "Android")
    find_library(log-lib log)
    target_link_libraries(
            nextplayer
            ${log-lib}
            common
            render
            demux
            decode
            ffmpeg
            mediandk
            android
    )
else ()
    message(FATAL_ERROR "Don't support ${CMAKE_SYSTEM_NAME}!")
endif ()

