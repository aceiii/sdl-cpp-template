cmake_minimum_required(VERSION 3.15)

project(sdl-cpp-template VERSION 0.0.1 LANGUAGES CXX)

set(EXE_NAME main)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_definitions(TOML_EXCEPTIONS=0)

set(SOURCE_FILES
    src/main.cpp
)

include(cmake/imgui_club.cmake)

add_subdirectory(external/SDL EXCLUDE_FROM_ALL)
add_subdirectory(external/spdlog EXCLUDE_FROM_ALL)
add_subdirectory(external/argparse EXCLUDE_FROM_ALL)
add_subdirectory(external/magic_enum EXCLUDE_FROM_ALL)
add_subdirectory(external/tomlplusplus EXCLUDE_FROM_ALL)

add_executable(${EXE_NAME} ${SOURCE_FILES})

target_link_libraries(${EXE_NAME} PRIVATE SDL3::SDL3)
target_link_libraries(${EXE_NAME} PRIVATE spdlog::spdlog)
target_link_libraries(${EXE_NAME} PRIVATE argparse::argparse)
target_link_libraries(${EXE_NAME} PRIVATE magic_enum::magic_enum)
target_link_libraries(${EXE_NAME} PRIVATE tomlplusplus::tomlplusplus)

