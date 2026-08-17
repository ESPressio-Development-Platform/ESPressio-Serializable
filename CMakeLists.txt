cmake_minimum_required(VERSION 3.16)

project(ESPressioSerializable LANGUAGES CXX)

add_library(ESPressioSerializable INTERFACE)

target_include_directories(
    ESPressioSerializable
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(
    ESPressioSerializable
    INTERFACE
    cxx_std_17
)
