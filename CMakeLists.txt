cmake_minimum_required(VERSION 3.10)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds are not allowed.")
endif()

project(imgui-frc-char)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if (APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
endif()

include(GenResources)
include(BuildWPIUtil)
include(BuildWPIMath)

add_executable(imgui-frc-char src/main/native/Main.cpp)
target_link_libraries(imgui-frc-char PUBLIC wpiutil wpimath)

