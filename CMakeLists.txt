cmake_minimum_required(VERSION 3.16)
project(OpenCVSubtitlePlayer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(OPENCV REQUIRED opencv4)

add_executable(player
    src/main.cpp
    src/app/PlayerApp.cpp
    src/video/VideoSource.cpp
    src/subs/SubtitleTrack.cpp
    src/subs/SrtParser.cpp
    src/render/SubtitleRenderer.cpp
)


target_include_directories(player PRIVATE
    ${OPENCV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(player PRIVATE ${OPENCV_LIBRARIES})

target_compile_options(player PRIVATE -Wall -Wextra -Wpedantic)
