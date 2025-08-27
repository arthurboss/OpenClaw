cmake_minimum_required(VERSION 4.1.0)

target_sources(openclaw
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Audio.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Audio.cpp
)

if (Emscripten)
    target_sources(openclaw
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/WebAudioAPI.h
        ${CMAKE_CURRENT_SOURCE_DIR}/WebAudioAPI.cpp
    )
endif (Emscripten)

if (WIN32)
    target_sources(openclaw PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/midiproc_c.c)
endif (WIN32)