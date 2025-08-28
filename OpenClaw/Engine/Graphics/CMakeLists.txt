cmake_minimum_required(VERSION 4.1.0)

target_sources(openclaw
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/IRenderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GraphicsManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GraphicsManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GraphicsAdapter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GraphicsAdapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Data/MenuBackgroundData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Data/MenuItemData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Data/MenuTextData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/WebGL/WebGLRenderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/WebGL/WebGLRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/WebGPU/WebGPURenderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/WebGPU/WebGPURenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/WebGPU/WebGPUShaders.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GraphicsTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IntegrationTest.cpp
)
