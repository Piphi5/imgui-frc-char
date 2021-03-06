# Download and unpack allwpilib at configure time
configure_file(CMakeLists.txt.in allwpilib-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND}
    -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/allwpilib-download )

  if(result)
  message(FATAL_ERROR "CMake step for allwpilib failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/allwpilib-download )
if(result)
  message(FATAL_ERROR "Build step for allwpilib failed: ${result}")
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(WITH_JAVA OFF CACHE BOOL "" FORCE)
set(WITH_CSCORE OFF CACHE BOOL "" FORCE)
set(WITH_WPILIB OFF CACHE BOOL "" FORCE)
set(WITH_SIMULATION_MODULES OFF CACHE BOOL "" FORCE)

# Add allwpilib directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/allwpilib-src
                 ${CMAKE_CURRENT_BINARY_DIR}/allwpilib-build
                 EXCLUDE_FROM_ALL)
