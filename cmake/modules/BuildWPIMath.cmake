file(GLOB_RECURSE wpimath_native_src allwpilib/wpimath/src/main/native/cpp/*.cpp)
file(GLOB wpimath_jni_src allwpilib/wpimath/src/main/native/cpp/jni/WPIMathJNI.cpp)
list(REMOVE_ITEM wpimath_native_src ${wpimath_jni_src})

add_library(wpimath ${wpimath_native_src})
set_target_properties(wpimath PROPERTIES DEBUG_POSTFIX "d")

if (MSVC)
    target_compile_options(wpimath PUBLIC /bigobj)
endif()
target_link_libraries(wpimath PRIVATE wpiutil)

target_include_directories(wpimath PUBLIC
                        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/allwpilib/wpimath/src/main/native/eigeninclude>
                        $<INSTALL_INTERFACE:${include_dest}/wpimath>)

target_include_directories(wpimath PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/allwpilib/wpimath/src/main/native/include>
                            $<INSTALL_INTERFACE:${include_dest}/wpimath>)