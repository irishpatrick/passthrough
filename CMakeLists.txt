cmake_minimum_required(VERSION 3.0)

project(passthrough C)

set(sources
    src/main.c
)

set(incl "")

if (NOT ${CMAKE_TOOLCHAIN_FILE} STREQUAL "")
    set(cflags /Debug /W3)
    find_package(portaudio CONFIG REQUIRED)

    set(incl
        ${incl}
    )

    set(libs
        ${libs}
        portaudio
    )
    
else()
    set(libs m)
    find_package(PkgConfig)

    pkg_check_modules(PORTAUDIO REQUIRED portaudio)

    set(incl
        ${incl}
        ${PORTAUDIO_INCLUDE_DIRS}
    )

    set(libs
        ${libs}
        ${PORTAUDIO_LIBRARIES}
    )

endif()

add_executable(${PROJECT_NAME} ${sources})
target_include_directories(${PROJECT_NAME} PRIVATE ${incl})
target_link_libraries(${PROJECT_NAME} PRIVATE ${libs})
