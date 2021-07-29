cmake_minimum_required(VERSION 3.10)

project(VRender)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/")

set(SOURCE_FILES
    "src/graphics/descriptor.h"
    "src/graphics/descriptor.cpp"
    "src/graphics/ubo.h"
    "src/graphics/ubo.cpp"
    "src/managers/asset_manager.h"
    "src/managers/asset_manager.cpp"
    "src/graphics/buffer.h"
    "src/graphics/buffer.cpp"
    "src/graphics/shader.h"
    "src/graphics/shader.cpp"
    "src/debug/logger.h"
    "src/rendering/renderable.h"
    "src/rendering/mesh.h"
    "src/rendering/material.h"
    "src/managers/scene_manager.h"
    "src/managers/scene_manager.cpp"
    "src/managers/render_manager.h"
    "src/managers/render_manager.cpp"
    "src/engine/engine.h"
    "src/vrender.h"
    "src/api.h"
    "src/vulkan/vulkan_app.h"
    "src/vulkan/vulkan_app.cpp"
    "src/main.cpp"
    )

IF(MSVC)
    foreach(source IN LISTS SOURCE_FILES)
        get_filename_component(source_path "${source}" PATH)
        string(REPLACE "/" "\\" source_path_msvc "${source_path}")
        source_group("${source_path_msvc}" FILES "${source}")
    endforeach()
ENDIF(MSVC)

link_directories(extern/vulkan/Lib/)
link_directories(extern/GLFW/lib-vc2019/)
link_directories(extern/assimp/lib)

add_executable(VRender ${SOURCE_FILES})


add_custom_command(TARGET  VRender POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   "${CMAKE_SOURCE_DIR}/res"
                   $<TARGET_FILE_DIR:VRender>)   
                        
add_custom_command(TARGET  VRender POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   "${CMAKE_SOURCE_DIR}/res"
                   ${PROJECT_BINARY_DIR})
        

#Copy dll
add_custom_command(TARGET VRender POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                   "${CMAKE_SOURCE_DIR}/extern/assimp/assimp-vc142-mt.dll"
                   $<TARGET_FILE_DIR:VRender>)

add_custom_command(TARGET VRender POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                   "${CMAKE_SOURCE_DIR}/extern/assimp/assimp-vc142-mt.dll"
                   ${PROJECT_BINARY_DIR})


set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VRender)

target_include_directories(VRender PRIVATE src/)
target_include_directories(VRender PRIVATE extern/vulkan/Include)
target_include_directories(VRender PRIVATE extern/GLFW/include)
target_include_directories(VRender PRIVATE extern/glm)
target_include_directories(VRender PRIVATE extern/assimp/include)

target_link_libraries(VRender glfw3)
target_link_libraries(VRender vulkan-1)
target_link_libraries(VRender assimp-vc142-mt)

target_compile_definitions(VRender PRIVATE LIB)