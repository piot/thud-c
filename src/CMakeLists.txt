cmake_minimum_required(VERSION 3.17)
project(thud_example C)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(CMAKE_C_STANDARD 99)

set(deps ../deps/)
if (APPLE)
        add_compile_definitions(TORNADO_OS_MACOS)
        set(OS_MACOS 1)
elseif (UNIX)
        add_compile_definitions(TORNADO_OS_LINUX)
        set(OS_LINUX 1)
endif()

include_directories(${deps}monsoon/src/include)
include_directories(${deps}tiny-libc/src/include) # needed from flood-c
include_directories(${deps}flood-c/src/include)
include_directories(${deps}clog/src/include)
include_directories(${deps}basal-c/src/include)
include_directories(${deps}tyran/src/include)
include_directories(${deps}imprint/src/include)
include_directories(${deps}thunder/src/include)
include_directories(include)
include_directories(external/include)

file(GLOB_RECURSE deps_src FOLLOW_SYMLINKS
        "${deps}*/src/lib/*.c"
        )

file(GLOB_RECURSE lib_src FOLLOW_SYMLINKS
        "lib/*.c"
        )

if (OS_LINUX)
        file(GLOB_RECURSE deps_platform_src FOLLOW_SYMLINKS
            "${deps}thunder/src/platform/pulseaudio/*.c"
        )
endif()

add_executable(thud_example
    ${deps_src}
    ${deps_platform_src}
    ${lib_src}
    examples/main.c
)

if (OS_LINUX)
        find_package (PulseAudio REQUIRED)
        include_directories(${PulseAudio_INCLUDE_DIRS})
        target_link_libraries(thud_example ${PulseAudio_LIBRARIES})
        target_link_libraries(thud_example pulse)
endif()

target_link_libraries(thud_example m)
target_link_libraries(thud_example  ${CMAKE_SOURCE_DIR}/external/libopusfile.a ${CMAKE_SOURCE_DIR}/external/libopus.a  ${CMAKE_SOURCE_DIR}/external/libogg.a)
