cmake_minimum_required(VERSION 3.18)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds are not allowed.")
endif()

project(imgui-frc-char)

add_subdirectory(allwpilib)

# Place libraries in lib and binaries in bin.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Make sure we build for both macOS arm64 and x86_64
set(CMAKE_OSX_ARCHITECTURES arm64;x86_64)

# Set C++ standard.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Create our main executable.
file(GLOB_RECURSE imgui-frc-char-sources src/main/native/*.cpp)
file(GLOB_RECURSE imgui-frc-char-headers src/main/native/*.h)

if (APPLE)
  add_executable(imgui-frc-char MACOSX_BUNDLE ${imgui-frc-char-sources} ${imgui-frc-char-headers})
  target_compile_definitions(imgui-frc-char PUBLIC MACOSX_BUNDLE)
  set_target_properties(imgui-frc-char PROPERTIES LINK_FLAGS "-framework Metal -framework QuartzCore")
else()
  add_executable(imgui-frc-char MACOSX_BUNDLE ${imgui-frc-char-sources} ${imgui-frc-char-headers})
endif()

target_include_directories(imgui-frc-char PUBLIC src/main/native)

# Link to imgui and WPILib.
target_link_libraries(imgui-frc-char PUBLIC wpigui imgui wpimath ntcore wpiutil)
