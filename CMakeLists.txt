cmake_minimum_required(VERSION 3.18)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

include(WPILib)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds are not allowed.")
endif()

project(imgui-frc-char)

# Place libraries in lib and binaries in bin.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set C++ standard.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Create WPILib targets and imgui.
add_wpilib_targets(2021.1.1-beta-2 STATIC)
add_subdirectory(imgui)

# Create our main executable.
file(GLOB_RECURSE imgui-frc-char-sources src/main/native/*.cpp)
file(GLOB_RECURSE imgui-frc-char-headers src/main/native/*.h)
add_executable(imgui-frc-char ${imgui-frc-char-sources} ${imgui-frc-char-headers})
target_include_directories(imgui-frc-char PUBLIC src/main/native)

# Link to macOS Frameworks.
if (APPLE)
  set_target_properties(imgui-frc-char PROPERTIES LINK_FLAGS "-framework Metal -framework QuartzCore")
endif()

# Link to imgui and WPILib.
target_link_libraries(imgui-frc-char PUBLIC wpigui imgui wpimath ntcore wpiutil)
