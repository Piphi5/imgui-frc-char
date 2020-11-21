file(GLOB wpigui_src allwpilib/wpigui/src/main/native/cpp/*.cpp)
file(GLOB wpigui_windows_src allwpilib/wpigui/src/main/native/directx11/*.cpp)
file(GLOB wpigui_mac_src allwpilib/wpigui/src/main/native/metal/*.mm)
file(GLOB wpigui_unix_src allwpilib/wpigui/src/main/native/opengl3/*.cpp)

add_library(wpigui STATIC ${wpigui_src})
set_target_properties(wpigui PROPERTIES DEBUG_POSTFIX "d")
set_property(TARGET wpigui PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(wpigui PUBLIC imgui)

target_include_directories(wpigui PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/allwpilib/wpigui/src/main/native/include>
                            $<INSTALL_INTERFACE:${include_dest}/wpigui>)

if (MSVC)
    target_sources(wpigui PRIVATE ${wpigui_windows_src})
else()
    if (APPLE)
        target_compile_options(wpigui PRIVATE -fobjc-arc)
        set_target_properties(wpigui PROPERTIES LINK_FLAGS "-framework Metal -framework QuartzCore")
        target_sources(wpigui PRIVATE ${wpigui_mac_src})
    else()
        target_sources(wpigui PRIVATE ${wpigui_unix_src})
    endif()
endif()
