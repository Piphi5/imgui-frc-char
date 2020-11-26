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

# Set C++ standard.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Create our main executable.
file(GLOB_RECURSE imgui-frc-char-sources src/main/native/cpp/*.cpp)
file(GLOB_RECURSE imgui-frc-char-headers src/main/native/include/*.h)

add_executable(imgui-frc-char ${imgui-frc-char-sources} ${imgui-frc-char-headers})
if (APPLE)
  set_target_properties(imgui-frc-char PROPERTIES LINK_FLAGS "-framework Metal -framework QuartzCore")
else()
  set_target_properties(imgui-frc-char PROPERTIES LINK_FLAGS "-lstdc++fs")
endif()

target_compile_options(imgui-frc-char PRIVATE -Wall -pedantic -Wextra -Werror -Wno-unused-parameter -Wno-error=deprecated-declarations)
target_include_directories(imgui-frc-char PUBLIC src/main/native/include)

# Link to imgui and WPILib.
target_link_libraries(imgui-frc-char PUBLIC wpigui imgui wpimath ntcore wpiutil)
