file(GLOB_RECURSE wpiutil_native_src allwpilib/wpiutil/src/main/native/cpp/*.cpp)
file(GLOB wpiutil_jni_src allwpilib/wpiutil/src/main/native/cpp/jni/WPIUtilJNI.cpp)
list(REMOVE_ITEM wpiutil_native_src ${wpiutil_jni_src})
file(GLOB_RECURSE wpiutil_unix_src allwpilib/wpiutil/src/main/native/unix/*.cpp)
file(GLOB_RECURSE wpiutil_windows_src allwpilib/wpiutil/src/main/native/windows/*.cpp)

set(uv_unix_src
    allwpilib/wpiutil/src/main/native/libuv/src/unix/async.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/core.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/dl.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/fs.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/getaddrinfo.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/getnameinfo.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/loop-watcher.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/loop.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/pipe.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/poll.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/process.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/signal.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/stream.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/tcp.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/thread.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/tty.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/udp.cpp
)

set(uv_darwin_src
    allwpilib/wpiutil/src/main/native/libuv/src/unix/bsd-ifaddrs.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/darwin.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/darwin-proctitle.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/fsevents.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/kqueue.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/proctitle.cpp
)

set(uv_linux_src
    allwpilib/wpiutil/src/main/native/libuv/src/unix/linux-core.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/linux-inotify.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/linux-syscalls.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/procfs-exepath.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/proctitle.cpp
    allwpilib/wpiutil/src/main/native/libuv/src/unix/sysinfo-loadavg.cpp
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if (NOT MSVC AND NOT APPLE)
    find_library(ATOMIC NAMES atomic libatomic.so.1)
    if (ATOMIC)
        message(STATUS "Found libatomic: ${ATOMIC}")
    endif()
endif()

GENERATE_RESOURCES(allwpilib/wpiutil/src/main/native/resources generated/main/cpp WPI wpi wpiutil_resources_src)

add_library(wpiutil ${wpiutil_native_src} ${wpiutil_resources_src})
set_target_properties(wpiutil PROPERTIES DEBUG_POSTFIX "d")

if (MSVC)
    target_compile_options(wpiutil PUBLIC /permissive- /Zc:throwingNew /MP /bigobj)
    target_compile_definitions(wpiutil PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif()
target_link_libraries(wpiutil Threads::Threads ${CMAKE_DL_LIBS} ${ATOMIC})

target_sources(wpiutil PRIVATE ${uv_native_src})
target_include_directories(wpiutil PRIVATE allwpilib/wpiutil/src/main/native/libuv/src)
target_include_directories(wpiutil PUBLIC
                        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/allwpilib/wpiutil/src/main/native/libuv/include>
                        $<INSTALL_INTERFACE:${include_dest}/wpiutil>)
if(NOT MSVC)
    target_sources(wpiutil PRIVATE ${uv_unix_src})
    if (APPLE)
        target_sources(wpiutil PRIVATE ${uv_darwin_src})
    else()
        target_sources(wpiutil PRIVATE ${uv_linux_src})
    endif()
    target_compile_definitions(wpiutil PRIVATE -D_GNU_SOURCE)
else()
    target_sources(wpiutil PRIVATE ${uv_windows_src})
endif()

if (MSVC)
    target_sources(wpiutil PRIVATE ${wpiutil_windows_src})
else ()
    target_sources(wpiutil PRIVATE ${wpiutil_unix_src})
endif()

target_include_directories(wpiutil PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/allwpilib/wpiutil/src/main/native/include>
                            $<INSTALL_INTERFACE:${include_dest}/wpiutil>)

install(TARGETS wpiutil EXPORT wpiutil DESTINATION "${main_lib_dest}")
install(DIRECTORY allwpilib/wpiutil/src/main/native/include/ DESTINATION "${include_dest}/wpiutil")


