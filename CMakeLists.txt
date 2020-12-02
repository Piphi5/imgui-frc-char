cmake_minimum_required(VERSION 3.11)
project(imgui-frc-char)

# Do not allow in-source builds.
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds are not allowed.")
endif()

# Add WPILib (we are using wpiutil, wpimath, ntcore, wpigui, and imgui).
add_subdirectory(allwpilib)

# Set locations for generated libraries and binaries.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Require C++17 to build the app.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Set our headers and sources.
file(GLOB_RECURSE imgui-frc-char-sources src/main/native/cpp/*.cpp)
file(GLOB_RECURSE imgui-frc-char-headers src/main/native/include/*.h)

# Add our robot project files.
file(GLOB_RECURSE robot-project ${CMAKE_SOURCE_DIR}/robot-project/*)

# Add the main executable.
add_executable(imgui-frc-char ${imgui-frc-char-sources} ${imgui-frc-char-headers})

# Set platform-specific options.
if (APPLE)
  # Link to Metal and QuartzCore frameworks for the GUI.
  set_target_properties(imgui-frc-char PROPERTIES LINK_FLAGS "-framework Metal -framework QuartzCore")
else()
  # Link to the filesystem library on Linux.
  target_link_libraries(imgui-frc-char PUBLIC stdc++fs)
endif()

# Enable all warnings.
target_compile_options(imgui-frc-char PRIVATE -Wall -pedantic -Wextra -Werror -Wno-unused-parameter -Wno-error=deprecated-declarations)

# Add include directories.
target_include_directories(imgui-frc-char PUBLIC src/main/native/include)

# Link to imgui and WPILib.
target_link_libraries(imgui-frc-char PUBLIC libglass wpigui imgui wpimath ntcore wpiutil)
