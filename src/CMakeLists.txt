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

target_include_directories(thud_example PRIVATE ${deps}monsoon/src/include)
target_include_directories(thud_example PRIVATE ${deps}tiny-libc/src/include) # needed from flood-c
target_include_directories(thud_example PRIVATE ${deps}flood-c/src/include)
target_include_directories(thud_example PRIVATE ${deps}clog/src/include)
target_include_directories(thud_example PRIVATE ${deps}basal-c/src/include)
target_include_directories(thud_example PRIVATE ${deps}tyran/src/include)
target_include_directories(thud_example PRIVATE ${deps}imprint/src/include)
target_include_directories(thud_example PRIVATE ${deps}thunder/src/include)
target_include_directories(thud_example PRIVATE include)
target_include_directories(thud_example PRIVATE external/include)

target_compile_definitions(thud_example PRIVATE CONFIGURATION_DEBUG TYRAN_CONFIGURATION_DEBUG)


if (OS_LINUX)
        find_package (PulseAudio REQUIRED)
        include_directories(${PulseAudio_INCLUDE_DIRS})
        target_link_libraries(thud_example ${PulseAudio_LIBRARIES})
        target_link_libraries(thud_example pulse)
endif()

target_link_libraries(thud_example m)
target_link_libraries(thud_example  ${CMAKE_SOURCE_DIR}/external/libopusfile.a ${CMAKE_SOURCE_DIR}/external/libopus.a  ${CMAKE_SOURCE_DIR}/external/libogg.a)
