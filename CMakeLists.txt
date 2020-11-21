cmake_minimum_required(VERSION 3.10)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

include(WPILib)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds are not allowed.")
endif()

project(imgui-frc-char)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_wpilib_targets(2021.1.1-beta-2 STATIC)
add_subdirectory(imgui)

add_executable(imgui-frc-char src/main/native/Main.cpp)
if (APPLE)
  set_target_properties(imgui-frc-char PROPERTIES LINK_FLAGS "-framework Metal -framework QuartzCore")
endif()
target_link_libraries(imgui-frc-char PUBLIC wpigui imgui wpimath wpiutil)

